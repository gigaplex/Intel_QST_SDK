/****************************************************************************/
/*                                                                          */
/*  Module:         libSMART.c                                              */
/*                                                                          */
/*  Description:    This module forms the core of the libSMART  DLL.  This  */
/*                  DLL  provides  the functions necessary to identify all  */
/*                  hard disk drives (HDDs) that support  Self-Monitoring,  */
/*                  Analysis  and  Reporting  Technology  (S.M.A.R.T.) and  */
/*                  expose the  failure  prediction,  attribute/threshold,  */
/*                  temperature  and  other data that is provided via this  */
/*                  technology.                                             */
/*                                                                          */
/*  Notes:      1.  This implementation  uses  driver  IOCTLs  to  extract  */
/*                  S.M.A.R.T.   data   directly  from  the  hard  drives.  */
/*                  Currently, support is provided for:                     */
/*                                                                          */
/*                    . The standard S.M.A.R.T. IOCTL interface  supported  */
/*                      by Microsoft's driver stack. This provides support  */
/*                      for  IDE  HDDs  and, provided RAID/AHCI support is  */
/*                      NOT enabled, SATA HDDs.                             */
/*                                                                          */
/*                    . The Common  Storage  Management  Interface  (CSMI)  */
/*                      IOCTL interface of the Intel Storage driver.  This  */
/*                      provides  support  for SATA HDDs that are included  */
/*                      within RAID arrays.                                 */
/*                                                                          */
/*              2.  Temperature Support is inconsistently implemented  and  */
/*                  is just as inconsistently available across the various  */
/*                  manufacturers'  S.M.A.R.T. implementations. In theory,  */
/*                  drives with temperature  sensors  should  be  exposing  */
/*                  current,  worst-case  (highest)  and maximum allowable  */
/*                  (limit) temperature values, but  this  is  rarely  the  */
/*                  case.  The author has made a stab at identifying which  */
/*                  are supported and how to extract them. This  has  only  */
/*                  been  verified  on the (non-exhaustive!) set of drives  */
/*                  available to the author.                                */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                                                                          */
/*     Copyright (c) 2005-2009, Intel Corporation. All Rights Reserved.     */
/*                                                                          */
/*  Redistribution and use in source and binary  forms,  with  or  without  */
/*  modification, are permitted provided that the following conditions are  */
/*  met:                                                                    */
/*                                                                          */
/*    - Redistributions of source code must  retain  the  above  copyright  */
/*      notice, this list of conditions and the following disclaimer.       */
/*                                                                          */
/*    - Redistributions  in binary form must reproduce the above copyright  */
/*      notice, this list of conditions and the  following  disclaimer  in  */
/*      the   documentation  and/or  other  materials  provided  with  the  */
/*      distribution.                                                       */
/*                                                                          */
/*    - Neither the name  of  Intel  Corporation  nor  the  names  of  its  */
/*      contributors  may  be  used to endorse or promote products derived  */
/*      from this software without specific prior written permission.       */
/*                                                                          */
/*  DISCLAIMER: THIS SOFTWARE IS PROVIDED BY  THE  COPYRIGHT  HOLDERS  AND  */
/*  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  */
/*  BUT  NOT  LIMITED  TO,  THE  IMPLIED WARRANTIES OF MERCHANTABILITY AND  */
/*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN  NO  EVENT  SHALL  */
/*  INTEL  CORPORATION  OR  THE  CONTRIBUTORS  BE  LIABLE  FOR ANY DIRECT,  */
/*  INDIRECT, INCIDENTAL, SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL  DAMAGES  */
/*  (INCLUDING,  BUT  NOT  LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR  */
/*  SERVICES; LOSS OF USE, DATA, OR  PROFITS;  OR  BUSINESS  INTERRUPTION)  */
/*  HOWEVER  CAUSED  AND  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  */
/*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING  */
/*  IN  ANY  WAY  OUT  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  */
/*  POSSIBILITY OF SUCH DAMAGE.                                             */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <wchar.h>

#include "SMART.h"

#include "libSMART.h"
#include "IDESMART.h"
#include "CSMISMART.h"

/****************************************************************************/
/* DRIVE_INFO - Provides information about a particular drive               */
/****************************************************************************/

typedef struct _DRIVE_INFO
{
    BIT_FIELD_IN_BYTE           bExposesTemp:       1;
    BIT_FIELD_IN_BYTE           bExposesTempWorst:  1;
    BIT_FIELD_IN_BYTE           uInterfaceType:     2;  // 0-3
    BIT_FIELD_IN_BYTE           uInterfaceIndex:    4;  // 0-15

} DRIVE_INFO;

/****************************************************************************/
/* IOCTL Interface Types                                                    */
/****************************************************************************/

#define IDE_INTERFACE           0
#define CSMI_INTERFACE          1

/****************************************************************************/
/* Module Global Variables                                                  */
/****************************************************************************/

static int                      iDrives     = 0;
static int                      iIDEDrives  = 0;
static int                      iCSMIDrives = 0;

static DRIVE_INFO               stDriveInfo[SMART_MAX_DRIVES];





/****************************************************************************/
/****************************************************************************/
/*                           DLL Support Routines                           */
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Initialize() - Initializes support for the use of various public         */
/* routines. Determines the O/S Version, the available IDE hard drives and  */
/* whether they support S.M.A.R.T. failure predictions, temperature         */
/* reporting, etc.                                                          */
/****************************************************************************/

static BOOL Initialize( void )
{
    int             iDrive, iAttrib;
    SMART_ATTRIB    *stAttrib, *pstAttrib;
    DWORD           dwError = NO_ERROR;

    // Get a temporary buffer for handling S.M.A.R.T. Attribute data

    stAttrib = (SMART_ATTRIB *)malloc( SMART_MAX_ATTRIBS * sizeof(SMART_ATTRIB) );

    if( stAttrib )
    {
        // Initialize support for IDE interface

        if( IDESMARTInitialize( &iIDEDrives ) )
        {
            if( iIDEDrives )
            {
                // Collect info about IDE drives

                for( iDrive = 0; iDrive < iIDEDrives; iDrive++ )
                {
                    stDriveInfo[iDrive].uInterfaceType      = IDE_INTERFACE;
                    stDriveInfo[iDrive].uInterfaceIndex     = (BYTE)iDrive;
                    stDriveInfo[iDrive].bExposesTemp        = FALSE;
                    stDriveInfo[iDrive].bExposesTempWorst   = FALSE;

                    // Get current attributes

                    if( !IDESMARTGetAttributeData( iDrive, stAttrib ) )
                    {
                        dwError = GetLastError();
                        break;
                    }

                    // Attempt to locate a temperature attribute

                    for( iAttrib = 0; (iAttrib < SMART_MAX_ATTRIBS) && stAttrib[iAttrib].byAttribId; iAttrib++ )
                    {
                        pstAttrib = &stAttrib[iAttrib];

                        if( pstAttrib->byAttribId == SMART_TEMP_ATTRIB_1 )
                        {
                            stDriveInfo[iDrive].bExposesTemp = TRUE;

                            if(    (pstAttrib->byWorst >= pstAttrib->byRaw[0])
                                || (pstAttrib->byWorst >= pstAttrib->byValue ) )
                                stDriveInfo[iDrive].bExposesTempWorst = TRUE;

                            break;
                        }

                        if( pstAttrib->byAttribId == SMART_TEMP_ATTRIB_2 )
                        {
                            stDriveInfo[iDrive].bExposesTemp = TRUE;

                            if( pstAttrib->byWorst >= pstAttrib->byValue )
                                stDriveInfo[iDrive].bExposesTempWorst = TRUE;

                            break;
                        }
                    }
                }

                if( dwError )
                {
                    IDESMARTCleanup();          // error, won't need interface
                    iIDEDrives = 0;
                }
                else
                    iDrives += iIDEDrives;      // successful, commit drives
            }
            else
                IDESMARTCleanup();              // no drives, won't need interface
        }
        else    // initialization failed
        {
            dwError = GetLastError();
            iIDEDrives = 0;
        }

        if( !dwError )
        {
            // Initialize support for CSMI interface

            if( CSMISMARTInitialize( &iCSMIDrives ) )
            {
                if( iCSMIDrives )
                {
                    // Collect info about CSMI drives

                    for( iDrive = 0; iDrive < iCSMIDrives; iDrive++ )
                    {
                        int iIndex = iDrives + iDrive;

                        stDriveInfo[iIndex].uInterfaceType      = CSMI_INTERFACE;
                        stDriveInfo[iIndex].uInterfaceIndex     = (BYTE)iDrive;
                        stDriveInfo[iIndex].bExposesTemp        = FALSE;
                        stDriveInfo[iIndex].bExposesTempWorst   = FALSE;

                        // Get current attributes

                        if( !CSMISMARTGetAttributeData( iDrive, stAttrib ) )
                        {
                            dwError = GetLastError();
                            break;
                        }

                        // Attempt to locate a temperature attribute

                        for( iAttrib = 0; (iAttrib < SMART_MAX_ATTRIBS) && stAttrib[iAttrib].byAttribId; iAttrib++ )
                        {
                            pstAttrib = &stAttrib[iAttrib];

                            if( pstAttrib->byAttribId == SMART_TEMP_ATTRIB_1 )
                            {
                                stDriveInfo[iIndex].bExposesTemp = TRUE;

                                if(    (pstAttrib->byWorst >= pstAttrib->byRaw[0])
                                    || (pstAttrib->byWorst >= pstAttrib->byValue ) )
                                    stDriveInfo[iIndex].bExposesTempWorst = TRUE;

                                break;
                            }

                            if( pstAttrib->byAttribId == SMART_TEMP_ATTRIB_2 )
                            {
                                stDriveInfo[iIndex].bExposesTemp = TRUE;

                                if( pstAttrib->byWorst >= pstAttrib->byValue )
                                    stDriveInfo[iIndex].bExposesTempWorst = TRUE;

                                break;
                            }
                        }
                    }

                    if( dwError )
                    {
                        CSMISMARTCleanup();         // error, won't need interface
                        iCSMIDrives = 0;
                    }
                    else
                        iDrives += iCSMIDrives;     // successful, commit drives
                }
                else
                    CSMISMARTCleanup();             // no drives, won't need interface
            }
            else    // initialization failed
            {
                dwError = GetLastError();
                iCSMIDrives = 0;
            }
        }

        if( dwError )
        {
            if( iIDEDrives )
            {
                IDESMARTCleanup();
                iIDEDrives = 0;
            }

            if( iCSMIDrives )
            {
                CSMISMARTCleanup();
                iCSMIDrives = 0;
            }

            iDrives = 0;
        }

        free( stAttrib );
    }

    if( dwError )
    {
        SetLastError( dwError );
        return( FALSE );
    }
    else
        return( TRUE );
}

/****************************************************************************/
/* Cleanup() - Cleans up for DLL unload                                     */
/****************************************************************************/

static void Cleanup( void )
{
    if( iDrives )
    {
        if( iIDEDrives )
        {
            IDESMARTCleanup();
            iIDEDrives = 0;
        }

        if( iCSMIDrives )
        {
            CSMISMARTCleanup();
            iCSMIDrives = 0;
        }

        iDrives = 0;
    }
}

#ifndef TESTING_DLL_FUNC
/****************************************************************************/
/* DllMain() - Main entry point for DLL. Handles process and thread attach  */
/* and detach requests.                                                     */
/****************************************************************************/

BOOL APIENTRY DllMain( HANDLE hModule, DWORD dwReason, LPVOID pvReserved )
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        iDrives = iIDEDrives = iCSMIDrives = 0;
        break;

    case DLL_PROCESS_DETACH:

        Cleanup();
        break;
    }

    return( TRUE );

    UNREFERENCED_PARAMETER( hModule );
    UNREFERENCED_PARAMETER( pvReserved );
}

#endif





/****************************************************************************/
/****************************************************************************/
/*                             Support Routines                             */
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* WordCopyBigEndian() - Copies array of words from one buffer to another,  */
/* swapping low and high bytes of each word to convert from Big Endian to   */
/* (Intel's) Little Endian ordering                                         */
/****************************************************************************/

static void WordCopyBigEndian( WORD wDest[], const WORD wSrc[], unsigned uWords )
{
    unsigned uIndex;

    for( uIndex = 0; uIndex < uWords; uIndex++ )
        wDest[uIndex] = (WORD)((wSrc[uIndex] << 8) | (wSrc[uIndex] >> 8));
}

/****************************************************************************/
/* StrTrim() - Removes leading & trailing whitespace from specified string  */
/****************************************************************************/

static void StrTrim( char *pszString )
{
    if( pszString && *pszString )
    {
        // Remove trailing whitespace characters

        int iIndex = strlen(pszString) - 1;

        while( iIndex && isspace(pszString[iIndex]) )
            pszString[iIndex--] = '\0';

        // Remove leading whitespace characters

        iIndex = 0;

        while( isspace(pszString[iIndex]) )
            iIndex++;

        if( iIndex )
            strcpy( pszString, &pszString[iIndex] );
    }
}





/****************************************************************************/
/****************************************************************************/
/*                             Public Routines                              */
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* SMARTGetDriveCapabilities() - Gets a drive's S.M.A.R.T. Capabilities     */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveCapabilities( int iDrive, BOOL *pbCurrentTemp, BOOL *pbWorstTemp )
{
    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || !pbCurrentTemp || !pbWorstTemp )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    *pbCurrentTemp = stDriveInfo[iDrive].bExposesTemp;
    *pbWorstTemp   = stDriveInfo[iDrive].bExposesTempWorst;

    return( TRUE );
}

/****************************************************************************/
/* SMARTGetDrivePrediction() - Gets a drive's Failure Prediction            */
/****************************************************************************/

BOOL APIENTRY SMARTGetDrivePrediction( int iDrive, BOOL *pbFailurePedicted )
{
    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || !pbFailurePedicted )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    switch( stDriveInfo[iDrive].uInterfaceType )
    {
    case IDE_INTERFACE:

        return( IDESMARTGetPrediction( stDriveInfo[iDrive].uInterfaceIndex, pbFailurePedicted ) );

    case CSMI_INTERFACE:

        return( CSMISMARTGetPrediction( stDriveInfo[iDrive].uInterfaceIndex, pbFailurePedicted ) );

    default:

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }
}

/****************************************************************************/
/* SMARTGetDriveTemperatures() - Gets a drive's current and/or worst-case   */
/* temperature. Specifying NULL for either destination pointer indicates    */
/* not interested in that particular temperature value. If the return of    */
/* the value(s) requested is not supported by the specified drive, the      */
/* function will return FALSE and set the last-error variable to            */
/* ERROR_NOT_SUPPORTED. Any other errors will also result in FALSE being    */
/* returned and the last-error variable set to an appropriate error code.   */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveTemperatures( int iDrive, int *piCurrentTemp, int *piWorstTemp )
{
    SMART_ATTRIB stAttrib[SMART_MAX_ATTRIBS];
    int          iAttrib;

    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || (!piCurrentTemp && !piWorstTemp) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    if(    (piCurrentTemp && !stDriveInfo[iDrive].bExposesTemp)
        || (piWorstTemp   && !stDriveInfo[iDrive].bExposesTempWorst) )
    {
        SetLastError( ERROR_NOT_SUPPORTED );
        return( FALSE );
    }

    // Obtain the current attribute data

    switch( stDriveInfo[iDrive].uInterfaceType )
    {
    case IDE_INTERFACE:

        if( !IDESMARTGetAttributeData( stDriveInfo[iDrive].uInterfaceIndex, stAttrib ) )
            return( FALSE );

        break;

    case CSMI_INTERFACE:

        if( !CSMISMARTGetAttributeData( stDriveInfo[iDrive].uInterfaceIndex, stAttrib ) )
            return( FALSE );

        break;

    default:

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    // Search through the attributes for one of the two temperature attribute types

    for( iAttrib = 0; (iAttrib < SMART_MAX_ATTRIBS) && stAttrib[iAttrib].byAttribId; iAttrib++ )
    {
        if( stAttrib[iAttrib].byAttribId == SMART_TEMP_ATTRIB_1 )
        {
            if( piCurrentTemp )
                *piCurrentTemp = ( stAttrib[iAttrib].byRaw[0] )? stAttrib[iAttrib].byRaw[0] : stAttrib[iAttrib].byValue;

            if( piWorstTemp )
                *piWorstTemp = stAttrib[iAttrib].byWorst;

            return( TRUE );
        }

        if( stAttrib[iAttrib].byAttribId == SMART_TEMP_ATTRIB_2 )
        {
            if( piCurrentTemp )
                *piCurrentTemp = stAttrib[iAttrib].byValue;

            if( piWorstTemp )
                *piWorstTemp = stAttrib[iAttrib].byWorst;

            return( TRUE );
        }
    }

    // Didn't find a temperature attribute...

    SetLastError( ERROR_NOT_FOUND );
    return( FALSE );
}

/****************************************************************************/
/* SMARTGetDriveInformation() - Gets information about the hard drive. This */
/* can include Model Number string, Serial Number string and/or size of the */
/* drive (in gigabytes). Specifying NULL for an item's destination pointer  */
/* indicates not interested in that particular item.                        */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveInformation( int iDrive, char *pszModel, char *pszSerial, int *piSize )
{
    WORD wId[256];

    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || (!pszModel && !pszSerial && !piSize) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    switch( stDriveInfo[iDrive].uInterfaceType )
    {
    case IDE_INTERFACE:

        if( !IDESMARTGetIdentifyData( stDriveInfo[iDrive].uInterfaceIndex, wId ) )
            return( FALSE );

        break;

    case CSMI_INTERFACE:

        if( !CSMISMARTGetIdentifyData( stDriveInfo[iDrive].uInterfaceIndex, wId ) )
            return( FALSE );

        break;

    default:

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    // Process Serial Number String

    if( pszSerial )
    {
        WordCopyBigEndian( (WORD *)pszSerial, &wId[ID_SERIAL_NO_OFFSET], ID_SERIAL_NO_WORDS );
        pszSerial[SMART_MAX_SERIAL] = '\0';
        StrTrim( pszSerial );
    }

    // Process Model Number String

    if( pszModel )
    {
        WordCopyBigEndian( (WORD *)pszModel, &wId[ID_MODEL_NO_OFFSET], ID_MODEL_NO_WORDS );
        pszModel[SMART_MAX_MODEL] = '\0';
        StrTrim( pszModel );
    }

    // Calculate Drive size

    if( piSize )
    {
        __int64 qiSize, qiSect;

        // Get sector count

        if( wId[ID_CMD_SET_SUPP_OFFSET] & ID_CMD_SET_SUPP_48BITS )
            qiSize = ((__int64)wId[ID_SECT48_WORD2_OFFSET] << 32) // 48-bit addressing
                   + ((__int64)wId[ID_SECT48_WORD1_OFFSET] << 16)
                   +  (__int64)wId[ID_SECT48_WORD0_OFFSET];
        else
            qiSize = ((__int64)wId[ID_SECT28_WORD1_OFFSET] << 16) // 28-bit addressing
                   +  (__int64)wId[ID_SECT28_WORD0_OFFSET];

        // Get sector size

        qiSect = 512;

        if( (wId[ID_SECTINFO_OFFSET] & ID_SECTINFO_VALID_MASK) == ID_SECTINFO_VALID )
        {
            // Sector Info word is valid

            if( wId[ID_SECTINFO_OFFSET] & ID_SECTINFO_SIZE_SPEC )
            {
                // Sector Size dword valid; extract and convert from words to bytes

                qiSect = ((__int64)wId[ID_SECTSIZE_WORD1_OFFSET] << 17)
                       + ((__int64)wId[ID_SECTSIZE_WORD0_OFFSET] <<  1);
            }
        }

        // Calculate drive size in bytes

        qiSize *= qiSect;

        // Convert to gigabytes as we save

        *piSize = (int)(qiSize >> 30);
    }

    return( TRUE );
}

/****************************************************************************/
/* SMARTGetDriveIndex() - Gets the physical index for the specified drive   */
/****************************************************************************/

BOOL APIENTRY SMARTGetDriveIndex( int iDrive, int *piIndex )
{
    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || !piIndex )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    switch( stDriveInfo[iDrive].uInterfaceType )
    {
    case IDE_INTERFACE:

        return( IDESMARTGetPhysicalIndex( stDriveInfo[iDrive].uInterfaceIndex, piIndex ) );

    case CSMI_INTERFACE:

        return( CSMISMARTGetPhysicalIndex( stDriveInfo[iDrive].uInterfaceIndex, piIndex ) );

    default:

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }
}

/****************************************************************************/
/* SMARTDisplayDriveAttributes - Prepares display output for a drive's      */
/* S.M.A.R.T. Attributes and calls the invoker's (call-back) output routine */
/* to have this output delivered to the display/log. Within this output     */
/* routine, the first invocation can be ignored if the user is not          */
/* interested in the title line generated by this facility (i.e. if they    */
/* have generated their own).                                               */
/*                                                                          */
/* NOTE: This routine is provided for debug purposes only. As stated in     */
/* the S.M.A.R.T. Specification: "APPLICATIONS SHOULD NOT INTERPRET, OR     */
/* PRESENT TO USERS, THE ATTRIBUTES OR THRESHOLD DATA VALUES".              */
/****************************************************************************/

static const struct
{
    unsigned char   byAttr;
    char            *pszAttr;
}
stAttrInfo[] =                                      // Used by (where mentioned):
{
    {   1, "Raw Read Error Rate"                },  // common
    {   2, "Throughput Performance"             },  // common
    {   3, "Spin Up Time"                       },  // common
    {   4, "Start/Stop Count"                   },  // common
    {   5, "Reallocated Sectors Count"          },  // common
    {   6, "Read Channel Margin"                },  // common
    {   7, "Seek Error Rate"                    },  // common
    {   8, "Seek Time Performance"              },  // common
    {   9, "Power-On Hours"                     },  // common
    {  10, "Spin Up Retry Count"                },  // IBM, Maxtor, Seagate
    {  11, "Recalibration Retries"              },  // Quantum, Maxtor
    {  12, "Device Power Cycle Count"           },  // IBM, Fujitsu, Quantum, Maxtor, Seagate
    {  13, "Soft Read Error Rate"               },  // Quantum
                                                    // Note: 96-101 reportedly used by Maxtor but no explanations...
    { 190, "Airflow Temperature"                },  // WDC
    { 190, "Temperature Difference From 100"    },  // Seagate
    { 191, "G-Sense Error Rate"                 },  // IBM
    { 192, "Power-Off/Emergency Retract Count"  },  // IBM, Maxtor, Fujitsu
    { 193, "Load/Unload Cycle Count"            },  // IBM, Maxtor
    { 194, "HDA Temperature (Celsius)"          },  // Maxtor, Seagate
    { 195, "Hardware ECC Recovered"             },  // Maxtor, Seagate, Quantum
    { 196, "Reallocated Event Count"            },  // IBM, Maxtor
    { 197, "Current Pending Sector Count"       },  // IBM, Maxtor, Seagate
    { 198, "OffLine Scan Uncorrectable Count"   },  // IBM, Maxtor, Seagate
    { 199, "UltraDMA CRC Error Rate"            },  // IBM, Quantum, Maxtor, Seagate
    { 200, "Write Error Rate"                   },  // Fujitsu, Maxtor, Seagate
    { 201, "Soft Read Error Rate"               },  // Maxtor
    { 202, "Data Address Mark Errors"           },  // Maxtor, Seagate
    { 203, "Run Out Cancel (ECC Errors)"        },  // Maxtor
    { 204, "Soft ECC Correction"                },  // Maxtor
    { 205, "Thermal Asperity Rate"              },  // Maxtor
    { 207, "Spin High Current"                  },  // Maxtor
    { 208, "Spin Buzz"                          },  // Maxtor
    { 209, "Offline Seek Performance"           },  // Maxtor
    { 220, "Disk Shift"                         },  // IBM
    { 221, "G-Sense Error Rate"                 },  // IBM, Hitachi
    { 222, "Loaded Hours"                       },  // IBM
    { 223, "Load/Unload Retry Count"            },  // IBM
    { 224, "Load Friction"                      },  // IBM
    { 225, "Load/Unload Cycle Count"            },  // IBM
    { 226, "Load-In Time"                       },  // IBM
    { 227, "Torque Amplification Count"         },  // IBM
    { 228, "Power-Off Retract Count"            },  // IBM
    { 230, "GMR Head Amplitude"                 },
    { 231, "HDD Temperature (Celsius)"          },
    { 240, "Head Flying Hours"                  },  // Hitachi
    { 250, "Read Error Retry Rate"              },
    { 255, "Unknown Attribute"                  }   // End of Table; this entry's string
                                                    // will be displayed for any unmatched
                                                    // attributes
};

BOOL APIENTRY SMARTDisplayDriveAttributes( int iDrive, void (APIENTRY *PutStrCallBack)( char *szStr ) )
{
    SMART_ATTRIB    stAttrib[SMART_MAX_ATTRIBS];
    SMART_THRESH    stThresh[SMART_MAX_ATTRIBS];
    char            szModel[SMART_MAX_MODEL+1];
    char            szLine[81];
    int             iAttrib, iString, iIndex;

    if( !iDrives && !Initialize() )
        return( FALSE );

    if( (iDrive < 0) || (iDrive >= iDrives) || !PutStrCallBack )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    switch( stDriveInfo[iDrive].uInterfaceType )
    {
    case IDE_INTERFACE:

        if(    !IDESMARTGetThresholdData( stDriveInfo[iDrive].uInterfaceIndex, stThresh )
            || !IDESMARTGetAttributeData( stDriveInfo[iDrive].uInterfaceIndex, stAttrib )
            || !IDESMARTGetPhysicalIndex( stDriveInfo[iDrive].uInterfaceIndex, &iIndex  ) )
            return( FALSE );

        break;

    case CSMI_INTERFACE:

        if(    !CSMISMARTGetThresholdData( stDriveInfo[iDrive].uInterfaceIndex, stThresh )
            || !CSMISMARTGetAttributeData( stDriveInfo[iDrive].uInterfaceIndex, stAttrib )
            || !CSMISMARTGetPhysicalIndex( stDriveInfo[iDrive].uInterfaceIndex, &iIndex  ) )
            return( FALSE );

        break;

    default:

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    // Print the header and loop through the structures, printing
    // the structures when the attribute ID is known.

    if( !SMARTGetDriveInformation( iDrive, szModel, NULL, NULL ) )
        return( FALSE );

    sprintf( szLine, "\n    Attribute Information for Drive %d (%s):\n\n", iIndex, szModel );
    PutStrCallBack( szLine );

    PutStrCallBack( "    Attribute  Description                               Threshold    Value  \n" );
    PutStrCallBack( "    ---------  ----------------------------------------  ---------  ---------\n" );

    for( iAttrib = 0; (iAttrib < SMART_MAX_ATTRIBS) && stAttrib[iAttrib].byAttribId; iAttrib++ )
    {
        BYTE byValue = stThresh[iAttrib].byValue;
        BYTE byAttr  = stAttrib[iAttrib].byAttribId;

        // find the name string to go with the attribute

        for( iString = 0; stAttrInfo[iString].byAttr != 255; iString++ )
        {
            if( byAttr == stAttrInfo[iString].byAttr )
                break;
        }

        // For the first of the temperature attribute types, if the first byte of
        // the raw data is non-zero, it is the current temperature value for the drive.

        if( (byAttr == SMART_TEMP_ATTRIB_1) && stAttrib[iAttrib].byRaw[0] )
            byValue = stAttrib[iAttrib].byRaw[0];

        // Output the attribute information

        sprintf( szLine, "     %3d (%02X)  %-43s%3d (%02X)   %3d (%02X)\n", byAttr, byAttr,
                 stAttrInfo[iString].pszAttr, stAttrib[iAttrib].byValue, stAttrib[iAttrib].byValue,
                 byValue, byValue );

        PutStrCallBack( szLine );
    }

    PutStrCallBack( "\n" );
    return( TRUE );
}





#ifdef TESTING_DLL_FUNC
/****************************************************************************/
/****************************************************************************/
/****************************** Test Functions ******************************/
/****************************************************************************/
/****************************************************************************/

static BOOL bOutputSTDERR = FALSE;

/****************************************************************************/
/* ConPuts() - Performs a fputs() to stdout and, if console output has been */
/* redirected, also to stderr.                                              */
/****************************************************************************/

int ConPuts( const char *pszString )
{
   if( bOutputSTDERR )
      fputs( pszString, stderr );

   return( fputs( pszString, stdout ) );
}

/****************************************************************************/
/* ConPrintf() - Performs a printf() to stdout and, if console output has   */
/* been redirected, also to stderr.                                         */
/****************************************************************************/

int ConPrintf( const char *pszFormat, ... )
{
   static char  szString[512];
   va_list      vaList;

   va_start( vaList, pszFormat );
   vsprintf( szString, pszFormat, vaList );
   va_end( vaList );

   return( ConPuts( szString ) );
}

/****************************************************************************/
/* DocError() - Documents an error                                          */
/****************************************************************************/

void DocError( char *pszMsg )
{
    char    *pszMsgBuf;
    size_t  tLen;
    DWORD   dwError = GetLastError();

    ConPrintf( "*** %s() failed, ccode = 0x%08X ", pszMsg, dwError );

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszMsgBuf, 0, NULL );

    tLen = strlen(pszMsgBuf);

    if( tLen > 2 )
    {
        if( pszMsgBuf[tLen - 2] == '\r' )
            pszMsgBuf[tLen - 2] = '\0';

        else if( pszMsgBuf[tLen - 1] == '\n' )
            pszMsgBuf[tLen - 1] = '\0';

        ConPrintf( "(%s)\n", pszMsgBuf );
    }
    else
        ConPuts( "(Unknown Error)\n" );

    LocalFree( pszMsgBuf );
}

/****************************************************************************/
/* PutStrCallBack() - Callback function used to output strings generated by */
/* DLL function SMARTDisplayDriveAttributes().                              */
/****************************************************************************/

static void APIENTRY PutStrCallBack( char *pszString )
{
    ConPuts( pszString );
}

/****************************************************************************/
/* main() - Mainline for testing module functionality                       */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
{
    int     iDrive, iIndex, iTemp, iWorst, iSize;
    BOOL    bTemp, bWorst, bFailing;
    char    szModel[SMART_MAX_MODEL+1], szSerial[SMART_MAX_SERIAL+1];

    bOutputSTDERR = !isatty( fileno( stdout ) );

    if( !Initialize() )
    {
        DocError( "Initialize" );
        return( 1 );
    }

    if( !iDrives )
    {
        ConPuts( "\nNo HDDs with S.M.A.R.T. support detected\n\n" );
        return( 0 );
    }

    ConPrintf( "\n%d HDD%s with S.M.A.R.T. support detected\n\n", iDrives, (iDrives > 1)? "s" : "" );

    for( iTemp = iWorst = iDrive = 0; iDrive < iDrives; iDrive++ )
    {
        if( !SMARTGetDriveIndex( iDrive, &iIndex ) )
        {
            DocError( "SMARTGetDriveIndex" );
            return( 1 );
        }

        if( !SMARTGetDriveCapabilities( iDrive, &bTemp, &bWorst ) )
        {
            DocError( "SMARTGetDriveCapabilities" );
            return( 1 );
        }

        if( !SMARTGetDriveInformation( iDrive, szModel, szSerial, &iSize ) )
        {
            DocError( "SMARTGetDriveInformation" );
            return( 1 );
        }

        if( !SMARTGetDrivePrediction( iDrive, &bFailing ) )
        {
            DocError( "SMARTGetDrivePrediction" );
            return( 1 );
        }

        if( bTemp )
        {
            if( !SMARTGetDriveTemperatures( iDrive, &iTemp, (bWorst)? &iWorst : NULL ) )
            {
                DocError( "SMARTGetDriveTemperatures" );
                return( 1 );
            }
        }

        ConPrintf( "Drive %d:\n\n", iDrive + 1 );
        ConPrintf( "    Model:    \"%s\"\n", szModel );
        ConPrintf( "    Serial:   \"%s\"\n", szSerial );
        ConPrintf( "    Size:     %dGB\n", iSize );
        ConPrintf( "    Index:    %s 0x%08X\n", (iDrive >= iIDEDrives)? "CSMI" : "IDE", iIndex );
        ConPrintf( "    Status:   %s\n", (bFailing)? "Failing" : "Healthy" );

        if( bTemp )
            ConPrintf( "    Temp:     %d\n", iTemp );
        else
            ConPrintf( "    Temp:     Unknown\n" );

        if( bWorst )
            ConPrintf( "    Worst:    %d\n", iWorst );
        else
            ConPrintf( "    Worst:    Unknown\n" );

        if( !SMARTDisplayDriveAttributes( iDrive, PutStrCallBack ) )
        {
            DocError( "SMARTDisplayDriveAttributes" );
            return( 1 );
        }
    }

    Cleanup();
    return( 0 );

    UNREFERENCED_PARAMETER( iArgs );
    UNREFERENCED_PARAMETER( pszArg );
}

#endif // def TESTING_DLL_FUNC

