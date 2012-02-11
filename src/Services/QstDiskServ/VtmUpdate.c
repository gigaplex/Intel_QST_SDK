/****************************************************************************/
/*                                                                          */
/*  Module:         VtmUpdate.c                                             */
/*                                                                          */
/*  Description:    Implements  support  for  updating the readings of the  */
/*                  Intel(R)  Quiet  System   Technology   (QST)   Virtual  */
/*                  Temperature  Monitors  (VTMs)  that are configured for  */
/*                  the monitoring of Hard Drive Temperatures.              */
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

#include <windows.h>

#include "QstDiskServ.h"

#include "QstComm.h"
#include "QstCfg.h"
#include "QstCmd.h"

/****************************************************************************/
/* Declarations                                                             */
/****************************************************************************/

static const DWORD dwWin32Map[] =
{
   ERROR_SUCCESS,                               // QST_CMD_SUCCESSFUL
   ERROR_NOT_SUPPORTED,                         // QST_CMD_REJECTED_UNSUPPORTED
   ERROR_ACCESS_DENIED,                         // QST_CMD_REJECTED_LOCKED
   ERROR_INVALID_PARAMETER,                     // QST_CMD_REJECTED_PARAMETER
   ERROR_REVISION_MISMATCH,                     // QST_CMD_REJECTED_VERSION
   ERROR_GEN_FAILURE,                           // QST_CMD_FAILED_COMM_ERROR
   ERROR_BAD_UNIT,                              // QST_CMD_FAILED_SENSOR_ERROR
   ERROR_NOT_ENOUGH_MEMORY,                     // QST_CMD_FAILED_NO_MEMORY
   ERROR_NO_SYSTEM_RESOURCES,                   // QST_CMD_FAILED_NO_RESOURCES
   ERROR_INVALID_FUNCTION,                      // QST_CMD_REJECTED_INVALID
   ERROR_BAD_COMMAND,                           // QST_CMD_REJECTED_CMD_SIZE
   ERROR_BAD_LENGTH                             // QST_CMD_REJECTED_RSP_SIZE
};

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int      iHDVtms = NO_RESULT_AVAIL;          // Number of VTMs for Hard Drives
static int      iHDVtmIndex[QST_ABS_TEMP_MONITORS]; // Physical Indexes for VTMs
static DWORD    dwInitError;                        // Error code from InitVtmUpdate()

/****************************************************************************/
/* InitVtmUpdate() - Obtains the QST configuration and collects information */
/* about the Virtual Temperature Monitors that are dedicated to hard drive  */
/* temperature monitoring. It returns the number of hard drive temperatures */
/* that are expected to be provided. If something goes wrong during this    */
/* process, NO_RESULT_AVAIL is returned.                                    */
/****************************************************************************/

int InitVtmUpdate( void )
{
    void    *pvBuffer;
    int     iIndex;

#ifdef DYNAMIC_DLL_LOADING

    // Initialize support for QST communication

    if( !QstInitialize() )
    {
        dwInitError = GetLastError();
        return( NO_RESULT_AVAIL );
    }

#endif

    // Allocate a command buffer

    pvBuffer = calloc( 1, sizeof(QST_GET_SUBSYSTEM_CONFIG_RSP) );

    if( !pvBuffer )
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    else
    {
        QST_GENERIC_CMD                 stCmd;
        P_QST_GET_SUBSYSTEM_CONFIG_RSP  pstRsp = (P_QST_GET_SUBSYSTEM_CONFIG_RSP)pvBuffer;

        // Prepare the command header

        stCmd.stHeader.byCommand        = QST_GET_SUBSYSTEM_CONFIG;
        stCmd.stHeader.byEntity         = 0;
        stCmd.stHeader.wCommandLength   = 0;
        stCmd.stHeader.wResponseLength  = sizeof(QST_GET_SUBSYSTEM_CONFIG_RSP);

        // Send the command to the QST Subsystem

        if( QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), pstRsp, sizeof(QST_GET_SUBSYSTEM_CONFIG_RSP) ) )
        {
            // Process response returned by QST Subsystem

            if( pstRsp->byStatus )
                SetLastError( ((pstRsp->byStatus <= QST_CMD_REJECTED_RSP_SIZE)? dwWin32Map[pstRsp->byStatus] : ERROR_NOT_SUPPORTED) );
            else
            {
                for( iIndex = 0, iHDVtms = 0; iIndex < QST_ABS_TEMP_MONITORS; iIndex++ )
                {
                    if(     pstRsp->stConfigPayload.TempMon[iIndex].Header.EntityEnabled                          // TM is enabled
                        && (pstRsp->stConfigPayload.TempMon[iIndex].DeviceAddress == QST_VALUE_NONE_UINT8)        // TM is virtual
                        && (pstRsp->stConfigPayload.TempMon[iIndex].Header.EntityUsage == QST_HARD_DRIVE_TEMP) )  // TM is for HDs
                        iHDVtmIndex[iHDVtms++] = pstRsp->stConfigPayload.TempMon[iIndex].Header.EntityIndex;
                }
            }
        }

        free( pvBuffer );
    }

    if( iHDVtms == NO_RESULT_AVAIL )
    {
        dwInitError = GetLastError();

#ifdef DYNAMIC_DLL_LOADING

        QstCleanup();
        SetLastError( dwInitError );

#endif

    }
    else
        dwInitError = NO_ERROR;

    return( iHDVtms );
}

/****************************************************************************/
/* DoneVtmUpdate() - Cleans up support forVTM update.                       */
/****************************************************************************/

void DoneVtmUpdate( void )
{
    if( iHDVtms >= 0 )
    {

#ifdef DYNAMIC_DLL_LOADING
        QstCleanup();
#endif

        iHDVtms = NO_RESULT_AVAIL;
    }
}

/****************************************************************************/
/* GetVtmIndex() - Returns the physical index of the physical Temperature   */
/* Monitor associated with the passed logical HD VTM index. If the index is */
/* invalid, NO_RESULT_AVAIL is returned.                                    */
/****************************************************************************/

int GetVtmIndex( int iIndex )
{
    if( iHDVtms == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iHDVtms) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
            return( iHDVtmIndex[iIndex] );

    return( NO_RESULT_AVAIL );
}

/****************************************************************************/
/* PutVtmTemp() - Delivers temperature update to the specified HD VTM.      */
/****************************************************************************/

BOOL PutVtmTemp( int iIndex, int iTemp )
{
    BOOL                             bSuccessful = FALSE;
    QST_SET_TEMP_MON_READING_CMD     stUpdCmd;
    QST_GENERIC_RSP                  stUpdRsp;

    if( iHDVtms == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iHDVtms) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
        {
            stUpdCmd.stHeader.byCommand       = QST_SET_TEMP_MON_READING;
            stUpdCmd.stHeader.byEntity        = (UINT8)iHDVtmIndex[iIndex];
            stUpdCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_SET_TEMP_MON_READING_CMD);
            stUpdCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);
            stUpdCmd.lfTempReading            = QST_TEMP_FROM_FLOAT( iTemp );

            // Send the command to the QST Subsystem

            if( QstCommand2( &stUpdCmd, sizeof(QST_SET_TEMP_MON_READING_CMD), &stUpdRsp, sizeof(QST_GENERIC_RSP) ) )
            {
                // Process response returned by QST Subsystem

                if( stUpdRsp.byStatus == QST_CMD_SUCCESSFUL )
                    bSuccessful = TRUE;
                else
                    SetLastError( (stUpdRsp.byStatus <= QST_CMD_REJECTED_RSP_SIZE)? dwWin32Map[stUpdRsp.byStatus] : ERROR_NOT_SUPPORTED );
            }
        }

    return( bSuccessful );
}

/****************************************************************************/
/* PutVtmStop() - Delivers No-Readings message to the specified HD VTM,     */
/* letting it know that there won't be any readings delivered for at least  */
/* a while.                                                                 */
/****************************************************************************/

BOOL PutVtmStop( int iIndex )
{
    BOOL                             bSuccessful = FALSE;
    QST_GENERIC_CMD                  stCmd;
    QST_GENERIC_RSP                  stRsp;

    if( iHDVtms == NO_RESULT_AVAIL )
        SetLastError( ( dwInitError )? dwInitError : ERROR_NOT_FOUND );
    else
        if( (iIndex < 0) || (iIndex >= iHDVtms) )
            SetLastError( ERROR_INVALID_PARAMETER );
        else
        {
            stCmd.stHeader.byCommand       = QST_NO_TEMP_MON_READINGS;
            stCmd.stHeader.byEntity        = (UINT8)iHDVtmIndex[iIndex];
            stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
            stCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);

            // Send the command to the QST Subsystem

            if( QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
            {
                // Process response returned by QST Subsystem

                if( stRsp.byStatus == QST_CMD_SUCCESSFUL )
                    bSuccessful = TRUE;
                else
                    SetLastError( (stRsp.byStatus <= QST_CMD_REJECTED_RSP_SIZE)? dwWin32Map[stRsp.byStatus] : ERROR_NOT_SUPPORTED );
            }
        }

    return( bSuccessful );
}

