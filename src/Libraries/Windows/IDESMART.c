
#undef  SKIP_DRIVERSTATUS_CHECK         // define this to ignore contents of
                                        // IOCTL DriverStatus field

/****************************************************************************/
/*                                                                          */
/*  Module:         IDESMART.c                                              */
/*                                                                          */
/*  Description:    Implements support for the enumeration and exposure of  */
/*                  failure predictions and attribute and  threshold  data  */
/*                  from  all IDE Hard Drives that support Self-Monitoring  */
/*                  Analysis and Reporting Technology (S.M.A.R.T.).         */
/*                                                                          */
/*  Notes:      1.  This module uses the IOCTL  interface  of  Microsoft's  */
/*                  standard  IDE  ATA/ATAPI device driver to identify the  */
/*                  drives that support S.M.A.R.T. and to obtain attribute  */
/*                  and threshold data from these drives.                   */
/*                                                                          */
/*              2.  The API exposed by this module is:                      */
/*                                                                          */
/*                    IDESMARTInitialize()       Enumerates HDDs & returns  */
/*                                               the  number  of HDDs that  */
/*                                               support S.M.A.R.T.         */
/*                                                                          */
/*                    IDESMARTGetPhysicalIndex() Gets the  physical  index  */
/*                                               for a particular HDD.      */
/*                                                                          */
/*                    IDESMARTGetIdentifyData()  Gets Identify data  block  */
/*                                               for the specified drive.   */
/*                                                                          */
/*                    IDESMARTGetPrediction()    Indicates if the  HDD  is  */
/*                                               predicting its demise.     */
/*                                                                          */
/*                    IDESMARTGetAttributeData() Gets Attribute data for a  */
/*                                               particular HDD.            */
/*                                                                          */
/*                    IDESMARTGetThresholdData() Gets Threshold data for a  */
/*                                               particular HDD.            */
/*                                                                          */
/*                    IDESMARTCleanup()          Cleans up after module.    */
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

#define  _WIN32_WINNT 0x0500   // Support Windows 2000 and later

#include <windows.h>
#include <winioctl.h>

#include "SMART.h"
#include "libSMART.h"
#include "IDESMART.h"

#pragma pack(1)

/****************************************************************************/
/* Miscellaneous Definitions                                                */
/****************************************************************************/

#define SMART_BASE_DRIVE_HEAD       0xA0                // for bDriveHeadReg in IDEREGS

/****************************************************************************/
/* GETVERSIONOUTPARAMS - For some unfathomable reason, WINIOCTL.H doesn't   */
/* provide a declaration for this structure. We define it in terms of       */
/* GETVERSIONINPARAMS...                                                    */
/****************************************************************************/

typedef struct _GETVERSIONINPARAMS  GETVERSIONOUTPARAMS;

/****************************************************************************/
/* XFERCMDOUTPARAMS - Version of SENDCMDOUTPARAMS with buffer space for     */
/* results of GET ATTRIBUTES/THRESHOLDS command                             */
/****************************************************************************/

typedef struct _XFERCMDOUTPARAMS
{
    SENDCMDOUTPARAMS                stSCOP;
    BYTE                            byBufferExtension[SMART_BUFFER_SIZE];

} XFERCMDOUTPARAMS;

/****************************************************************************/
/* DRIVE_DATA - Structure retaining information needed to access a drive    */
/****************************************************************************/

typedef struct _DRIVE_DATA
{
    BYTE                            byDrive;            // Actual index for drive
    HANDLE                          hDrive;             // Handle for connection to drive

} DRIVE_DATA;

/****************************************************************************/
/* eSTATUS - Enumeration defining the possible return codes for GetPrediction() */
/****************************************************************************/

typedef enum
{
    NO_FAILURE_PREDICTED            = 0,
    FAILURE_PREDICTED               = 1,
    PREDICTION_UNAVAILABLE          = 2

} eSTATUS;

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                          iDrives = 0;        // Number of IDE drives present
                                                        // that support S.M.A.R.T.

static DRIVE_DATA                   stDriveData[SMART_MAX_DRIVES]; // Info we need for
                                                        // each drive we must support

static DWORD                        dwReturned;         // Field to receive byte count
                                                        // returned by DeviceIoControl.
                                                        // Using a global field is not a
                                                        // thread-safe issue since we
                                                        // don't care about this info...

static DRIVERSTATUS                 stStatus;           // for debugging failures, if a
                                                        // driver reports an error while
                                                        // executing a command for us, we
                                                        // set the last-error variable to
                                                        // ERROR_BAD_DRIVER_LEVEL and put
                                                        // the driver status here...

static SENDCMDINPARAMS              stInParams;         // Buffer for IOCTL input parameters
static XFERCMDOUTPARAMS             stOutParams;        // Buffer for IOCTL output parameters





/****************************************************************************/
/****************************************************************************/
/***************************** Support Functions ****************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* GetVersionInfo() - Returns S.M.A.R.T. version information.               */
/****************************************************************************/

static BOOL GetVersionInfo( int iDrive )
{
    memset( &stOutParams, 0, sizeof(XFERCMDOUTPARAMS) );

    return( DeviceIoControl( stDriveData[iDrive].hDrive, SMART_GET_VERSION, NULL, 0,
                             &stOutParams, sizeof(XFERCMDOUTPARAMS) - 1, &dwReturned, NULL ) );
}

/****************************************************************************/
/* DoIOCTL() - Performs an IDE driver operation                             */
/****************************************************************************/

static BOOL DoIOCTL( int iDrive, BYTE byCmd, BYTE byFeat, DWORD dwBuffSize, DWORD dwIOCTL )
{
    memset( &stInParams,  0, sizeof(SENDCMDINPARAMS) );
    memset( &stOutParams, 0, sizeof(XFERCMDOUTPARAMS) );

    stInParams.irDriveRegs.bFeaturesReg     = byFeat;
    stInParams.irDriveRegs.bSectorCountReg  = 1;
    stInParams.irDriveRegs.bSectorNumberReg = 1;
    stInParams.irDriveRegs.bCylLowReg       = (BYTE)((byCmd == SMART_CMD)? SMART_CYL_LOW : 0);
    stInParams.irDriveRegs.bCylHighReg      = (BYTE)((byCmd == SMART_CMD)? SMART_CYL_HI  : 0);
    stInParams.irDriveRegs.bCommandReg      = byCmd;
    stInParams.irDriveRegs.bDriveHeadReg    = (BYTE)(SMART_BASE_DRIVE_HEAD | ((stDriveData[iDrive].byDrive % 2) << 4));
    stInParams.bDriveNumber                 = stDriveData[iDrive].byDrive;
    stInParams.cBufferSize                  = dwBuffSize;

    if( !DeviceIoControl( stDriveData[iDrive].hDrive, dwIOCTL,
                          &stInParams,  sizeof(SENDCMDINPARAMS)  - 1,
                          &stOutParams, sizeof(XFERCMDOUTPARAMS) - 1,
                          &dwReturned, NULL ) )
        return( FALSE );

#ifndef SKIP_DRIVERSTATUS_CHECK

    if( stOutParams.stSCOP.DriverStatus.bDriverError )
    {
        memcpy( &stStatus, &stOutParams.stSCOP.DriverStatus, sizeof(DRIVERSTATUS) );
        SetLastError( ERROR_BAD_DRIVER_LEVEL );
        return( FALSE );
    }

#endif

    return( TRUE );
}

/****************************************************************************/
/* DoEnable() - Enables access to a drive's S.M.A.R.T. capabilities.        */
/****************************************************************************/

static BOOL DoEnable( int iDrive )
{
    return( DoIOCTL( iDrive, SMART_CMD, ENABLE_SMART, 0, SMART_SEND_DRIVE_COMMAND ) );
}

/****************************************************************************/
/* GetPrediction() - Returns reliability status for the drive.              */
/****************************************************************************/

static eSTATUS GetPrediction( int iDrive )
{
    if( DoIOCTL( iDrive, SMART_CMD, RETURN_SMART_STATUS, 0, SMART_SEND_DRIVE_COMMAND ) )
    {
        IDEREGS *pstIDERegs = (IDEREGS *)&stInParams.irDriveRegs;

        if( (pstIDERegs->bCylLowReg == SMART_CYL_LOW) && (pstIDERegs->bCylHighReg == SMART_CYL_HI) )
            return( NO_FAILURE_PREDICTED );

        if( (pstIDERegs->bCylLowReg == SMART_CYL_LOW_EXC) && (pstIDERegs->bCylHighReg == SMART_CYL_HI_EXC) )
            return( FAILURE_PREDICTED );

        SetLastError( ERROR_SUCCESS );
    }

    return( PREDICTION_UNAVAILABLE );
}

/****************************************************************************/
/* GetAttributes() - Returns S.M.A.R.T. Attribute current data.             */
/****************************************************************************/

static BOOL GetAttributes( int iDrive, SMART_ATTRIB *pstAttrib )
{
    if( DoIOCTL( iDrive, SMART_CMD, READ_ATTRIBUTES, SMART_BUFFER_SIZE, SMART_RCV_DRIVE_DATA ) )
    {
        SMART_ATTRIBS *pstAttribs = (SMART_ATTRIBS *)stOutParams.stSCOP.bBuffer;

        if( pstAttrib )
            memcpy( pstAttrib, pstAttribs->Attribute, sizeof(SMART_ATTRIB) * SMART_MAX_ATTRIBS );

        return( TRUE );
    }

    return( FALSE );
}

/****************************************************************************/
/* GetThresholds() - Returns S.M.A.R.T. Attribute threshold data.           */
/****************************************************************************/

static BOOL GetThresholds( int iDrive, SMART_THRESH *pstThresh )
{
    if( DoIOCTL( iDrive, SMART_CMD, READ_THRESHOLDS, SMART_BUFFER_SIZE, SMART_RCV_DRIVE_DATA ) )
    {
        SMART_THRESHS *pstThreshs = (SMART_THRESHS *)stOutParams.stSCOP.bBuffer;

        if( pstThresh )
            memcpy( pstThresh, pstThreshs->Threshold, sizeof(SMART_THRESH) * SMART_MAX_ATTRIBS );

        return( TRUE );
    }

    return( FALSE );
}

/****************************************************************************/
/* DoIdentify() - Performs an IDENTIFY operation to get drive information   */
/****************************************************************************/

static BOOL DoIdentify( int iDrive, void *pvBuffer )
{
    if( DoIOCTL( iDrive, ID_CMD, 0, SMART_BUFFER_SIZE, SMART_RCV_DRIVE_DATA ) )
    {
        memcpy( pvBuffer, stOutParams.stSCOP.bBuffer, SMART_BUFFER_SIZE );
        return( TRUE );
    }

    return( FALSE );
}






/****************************************************************************/
/****************************************************************************/
/***************************** Public Functions *****************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* IDESMARTInitialize() - Initializes module. Returns number of drives      */
/* supporting S.M.A.R.T.                                                    */
/****************************************************************************/

BOOL IDESMARTInitialize( int *piDrives )
{
    int         iDrive;
    char        szDrive[32];
    unsigned    uPrevErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    if( !piDrives )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    for( iDrives = iDrive = 0; iDrive < SMART_MAX_DRIVES; iDrive++ )
    {
        sprintf( szDrive, "\\\\.\\PHYSICALDRIVE%d", iDrive );

        stDriveData[iDrives].hDrive = CreateFile( szDrive, GENERIC_READ | GENERIC_WRITE,
                                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                  NULL, OPEN_EXISTING, 0, NULL );

        if( stDriveData[iDrives].hDrive != INVALID_HANDLE_VALUE )
        {
            stDriveData[iDrives].byDrive = (BYTE)iDrive;

            // See if drive supports S.M.A.R.T. and attribute data is available
            // (Note: RAID volume won't give attributes)

            if( GetVersionInfo( iDrives ) && DoEnable( iDrives ) && GetAttributes( iDrives, NULL ) )
            {
                ++iDrives;
                continue;
            }

            CloseHandle( stDriveData[iDrives].hDrive );
        }
    }

    SetErrorMode( uPrevErrorMode );
    *piDrives = iDrives;
    return( TRUE );
}

/****************************************************************************/
/* IDESMARTGetPhysicalIndex() - Gets the physical index for a drive         */
/****************************************************************************/

BOOL IDESMARTGetPhysicalIndex( int iDrive, int *piIndex )
{
    if( iDrives == 0 )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !piIndex )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    *piIndex = stDriveData[iDrive].byDrive;
    return( TRUE );
}

/****************************************************************************/
/* IDESMARTGetIdentifyData() - Gets the Identify data for specified drive.  */
/****************************************************************************/

BOOL IDESMARTGetIdentifyData( int iDrive, void *pvBuffer )
{
    if( iDrives == 0 )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !pvBuffer )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    // Get IDENTIFY data from the drive

    if( !DoIdentify( iDrive, pvBuffer ) )
        return( FALSE );

    return( TRUE );
}

/****************************************************************************/
/* IDESMARTGetPrediction() - Indicates whether drive is predicting demise   */
/****************************************************************************/

BOOL IDESMARTGetPrediction( int iDrive, BOOL *pbFailing )
{
    if( iDrives == 0 )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !pbFailing )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    switch( GetPrediction( iDrive ) )
    {
    case NO_FAILURE_PREDICTED:

        *pbFailing = FALSE;
        return( TRUE );

    case FAILURE_PREDICTED:

        *pbFailing = TRUE;
        return( TRUE );

    case PREDICTION_UNAVAILABLE:
    default:

        return( FALSE );
    }
}

/****************************************************************************/
/* IDESMARTGetAttributeData() - Gets Attribute data for a particular drive  */
/****************************************************************************/

BOOL IDESMARTGetAttributeData( int iDrive, SMART_ATTRIB *pstAttrib )
{
    if( iDrives == 0 )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !pstAttrib )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    return( GetAttributes( iDrive, pstAttrib ) );
}

/****************************************************************************/
/* IDESMARTGetThresholdData() - Gets Threshold data for a particular drive  */
/****************************************************************************/

BOOL IDESMARTGetThresholdData( int iDrive, SMART_THRESH *pstThresh )
{
    if( iDrives == 0 )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !pstThresh )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    return( GetThresholds( iDrive, pstThresh ) );
}

/****************************************************************************/
/* IDESMARTCleanup() - Cleans up resources used by the module               */
/****************************************************************************/

void IDESMARTCleanup( void )
{
    if( iDrives )
    {
        int iDrive;

        for( iDrive = 0; iDrive < iDrives; iDrive++ )
            CloseHandle( stDriveData[iDrives].hDrive );

        iDrives = 0;
    }
}





#ifdef TESTING
/****************************************************************************/
/****************************************************************************/
/****************************** Test Functions ******************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* DocError() - Documents an error                                          */
/****************************************************************************/

void DocError( char *pszMsg )
{
    char    *pszMsgBuf;
    size_t  tLen;
    DWORD   dwError = GetLastError();

    printf( "*** %s() failed, ccode = 0x%08X ", pszMsg, dwError );

    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszMsgBuf, 0, NULL );

    tLen = strlen(pszMsgBuf);

    if( tLen > 2 )
    {
        if( pszMsgBuf[tLen - 2] == '\r' )
            pszMsgBuf[tLen - 2] = '\0';

        else if( pszMsgBuf[tLen - 1] == '\n' )
            pszMsgBuf[tLen - 1] = '\0';

        printf( "(%s)\n", pszMsgBuf );
    }
    else
        puts( "(Unknown Error)" );

    LocalFree( pszMsgBuf );
}

/****************************************************************************/
/* main() - Mainline for testing module functionality                       */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
{
    int     iDrives, iDrive, iIndex;
    BOOL    bFailing;

    if( !IDESMARTInitialize( &iDrives ) )
    {
        DocError( "IDESMARTInitialize" );
        return( 1 );
    }

    printf( "\n%d IDE Drive%s with S.M.A.R.T. support detected\n\n", iDrives, (iDrives > 1)? "s" : "" );

    for( iDrive = 0; iDrive < iDrives; iDrive++ )
    {
        if( !IDESMARTGetPhysicalIndex( iDrive, &iIndex ) )
        {
            DocError( "IDESMARTGetPhysicalIndex" );
            break;
        }

        if( !IDESMARTGetPrediction( iDrive, &bFailing ) )
        {
            DocError( "IDESMARTGetPrediction" );
            break;
        }

        printf( "IDE/SATA Drive %d %s\n\n", iIndex, (bFailing)? "Predicting its demise!" : "Healthy" );
    }

    IDESMARTCleanup();
    return( (iDrive < iDrives)? 1 : 0 );
}

#endif // def TESTING
