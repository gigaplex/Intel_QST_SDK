/****************************************************************************/
/*                                                                          */
/*  Module:         CSMISMART.c                                             */
/*                                                                          */
/*  Description:    Implements support for the enumeration and exposure of  */
/*                  failure predictions and attribute and  threshold  data  */
/*                  from   SATA  Hard  Drives,  connected  to  controllers  */
/*                  enabled with the Common Storage  Management  Interface  */
/*                  (CSMI),  which  support  Self-Monitoring  Analysis and  */
/*                  Reporting Technology (S.M.A.R.T.).                      */
/*                                                                          */
/*  Notes:      1.  The module utilizes the  IOCTL  interface  of  Intel's  */
/*                  CSMI  device  driver to determine which drives support  */
/*                  S.M.A.R.T. and to obtain attribute and threshold  data  */
/*                  from these drives.                                      */
/*                                                                          */
/*              2.  The API exposed by this module is:                      */
/*                                                                          */
/*                    CSMISMARTInitialize()       Initializes  module  and  */
/*                                                returns  the  number  of  */
/*                                                SATA drives detected.     */
/*                                                                          */
/*                    CSMISMARTGetPhysicalIndex() Gets physical index  for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTGetIdentifyData()  Gets Identify data block  */
/*                                                for the specified drive.  */
/*                                                                          */
/*                    CSMISMARTGetPrediction()    Indicates whether or not  */
/*                                                the  specified  drive is  */
/*                                                predicting its demise.    */
/*                                                                          */
/*                    CSMISMARTGetAttributeData() Gets attribute data  for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTGetThresholdData() Gets  threshold data for  */
/*                                                the specified drive.      */
/*                                                                          */
/*                    CSMISMARTCleanup()          Cleans up resources used  */
/*                                                by the module.            */
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
#include "CSMIIOCTL.h"

#include "SMART.h"
#include "libSMART.h"
#include "CSMISMART.h"

#pragma pack(1)

/****************************************************************************/
/* Miscellaneous Definitions                                                */
/****************************************************************************/

#define ONE_SECTOR                  512
#define TWO_SECTORS                 1024

/****************************************************************************/
/* CSMI_SAS_STP_PT_WITH_BUFFER - Version of CSMI_SAS_STP_PASSTHRU_BUFFER    */
/* with buffer space for results of GET ATTRIBUTES/THRESHOLDS commands      */
/****************************************************************************/

typedef struct _CSMI_SAS_STP_PT_WITH_BUFFER
{
    CSMI_SAS_STP_PASSTHRU_BUFFER    stPTB;
    BYTE                            byExtBuff[TWO_SECTORS-1];

} CSMI_SAS_STP_PT_WITH_BUFFER;

/****************************************************************************/
/* CSMI_DRIVE_ERROR - Structure retaining information about the most recent */
/* driver/drive operation failure                                           */
/****************************************************************************/

typedef struct _CSMI_DRIVE_ERROR
{
    DWORD                           dwLastError;        // Value from GetLastError()
                                                        //  - Only set if DeviceIoControl() fails
                                                        //  - Only field set if this is the case
    DWORD                           dwIoctlRetCode;     // stIoctlHeader.ReturnCode
    BYTE                            byConnStatus;       // stStatus.bConnectionStatus
    BYTE                            byFISStatus;        // stStatus.bStatusFIS.Status
                                                        //  - Low-Order Bit indicates device error

} CSMI_DRIVE_ERROR;

/****************************************************************************/
/* CSMI_DRIVE_DATA - Structure retaining information needed to access drive */
/****************************************************************************/

typedef struct _CSMI_DRIVE_DATA
{
    HANDLE                          hDevice;            // Handle for SAS device
    BYTE                            bySASAddress[8];    // Address for SAS Device

} CSMI_DRIVE_DATA;

/****************************************************************************/
/* eSTATUS - Enumeration defining possible return codes for GetStatus()     */
/****************************************************************************/

typedef enum
{
    NO_FAILURE_PREDICTED = 0,
    FAILURE_PREDICTED,
    PREDICTION_UNAVAILABLE

} eSTATUS;

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                          iDrives = 0;        // Number of CSMI drives present
                                                        // that support S.M.A.R.T.

static CSMI_DRIVE_DATA              stDriveData[SMART_MAX_DRIVES];// Info we need for
                                                        // each drive we must support

static BOOL                         bNeedWorkarounds;   // Indicates if we need workarounds
                                                        // for known bugs in the driver

static DWORD                        dwReturned;         // Field to receive byte count
                                                        // returned by DeviceIoControl.
                                                        // Using a global field is not a
                                                        // thread-safe issue since we
                                                        // don't care about this info...

static CSMI_DRIVE_ERROR             stStatus;           // for debugging failures, if a
                                                        // driver reports an error while
                                                        // executing a command for us, we
                                                        // set the last-error variable to
                                                        // ERROR_BAD_DRIVER_LEVEL and put
                                                        // the driver status here...

static CSMI_SAS_STP_PT_WITH_BUFFER  stPTBuff;           // Buffer for doing Pass-Thru Ops






/****************************************************************************/
/****************************************************************************/
/***************************** Support Functions ****************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* InitIoctlHeader() - Initializes CSMI IOCTL Header                        */
/****************************************************************************/

static void InitIoctlHeader( DWORD dwControlCode, BYTE *pbySignature, DWORD dwIoctlSize, DWORD dwBuffSize )
{
    memset( &stPTBuff, 0, sizeof(CSMI_SAS_STP_PT_WITH_BUFFER) );

    stPTBuff.stPTB.IoctlHeader.HeaderLength     = sizeof(IOCTL_HEADER);
    stPTBuff.stPTB.IoctlHeader.Timeout          = CSMI_SAS_TIMEOUT;
    stPTBuff.stPTB.IoctlHeader.ControlCode      = dwControlCode;
    stPTBuff.stPTB.IoctlHeader.Length           = dwIoctlSize - sizeof(IOCTL_HEADER) + dwBuffSize;

    memcpy( stPTBuff.stPTB.IoctlHeader.Signature, pbySignature, sizeof(CSMI_SAS_SIGNATURE) );
}

/****************************************************************************/
/* DoCSMIIoctl() - Performs a Pass-Thru CSMI IOCTL operation                */
/****************************************************************************/

static BOOL DoCSMIIoctl( int iDrive, BYTE byCmd, BYTE byFeat, DWORD dwBuffSize )
{
    H2D_RFIS *pstCmdFIS = (H2D_RFIS *)stPTBuff.stPTB.Parameters.bCommandFIS;
    D2H_RFIS *pstRspFIS = (D2H_RFIS *)stPTBuff.stPTB.Status.bStatusFIS;

    // Setup the IOCTL for a CSMI Pass-Thru operation

    memset( &stPTBuff, 0, sizeof(CSMI_SAS_STP_PT_WITH_BUFFER) );

    stPTBuff.stPTB.IoctlHeader.HeaderLength     = sizeof(IOCTL_HEADER);
    stPTBuff.stPTB.IoctlHeader.Timeout          = CSMI_SAS_TIMEOUT;
    stPTBuff.stPTB.IoctlHeader.ControlCode      = CC_CSMI_SAS_STP_PASSTHRU;
    stPTBuff.stPTB.IoctlHeader.Length           = sizeof(CSMI_SAS_STP_PASSTHRU_BUFFER) - 1 - sizeof(IOCTL_HEADER) + dwBuffSize;

    memcpy( stPTBuff.stPTB.IoctlHeader.Signature, CSMI_SAS_SIGNATURE, sizeof(CSMI_SAS_SIGNATURE) );

    stPTBuff.stPTB.Parameters.bPhyIdentifier    = stDriveData[iDrive].bySASAddress[CSMI_SAS_ADDRESS_PORT_ID];
    stPTBuff.stPTB.Parameters.uFlags            = CSMI_SAS_STP_READ | CSMI_SAS_STP_DMA;
    stPTBuff.stPTB.Parameters.uDataLength       = dwBuffSize;

    memcpy( stPTBuff.stPTB.Parameters.bDestinationSASAddress, stDriveData[iDrive].bySASAddress, 8 );

    // Setup the Frame Information Structure (FIS) for the Operation
    // We presume it's a S.M.A.R.T. operation (doesn't hurt if it isn't)...

    pstCmdFIS->FisType                          = FIS_TYPE_REG_H2D;
    pstCmdFIS->Command                          = byCmd;
    pstCmdFIS->Features                         = byFeat;
    pstCmdFIS->CylinderLow                      = SMART_CYL_LOW;
    pstCmdFIS->CylinderHigh                     = SMART_CYL_HI;

    // Perform the IOCTL operation

    if( !DeviceIoControl( stDriveData[iDrive].hDevice, IOCTL_SCSI_MINIPORT, &stPTBuff, sizeof(CSMI_SAS_STP_PT_WITH_BUFFER),
                          &stPTBuff, sizeof(CSMI_SAS_STP_PT_WITH_BUFFER), &dwReturned, NULL ) )
    {
        stStatus.dwLastError = GetLastError();
        return( FALSE );
    }

    // Check the various places for an indication of failure

    if( stPTBuff.stPTB.IoctlHeader.ReturnCode || stPTBuff.stPTB.Status.bConnectionStatus || (pstRspFIS->Status & 0x01) )
    {
        stStatus.dwLastError                    = 0;
        stStatus.dwIoctlRetCode                 = stPTBuff.stPTB.IoctlHeader.ReturnCode;
        stStatus.byConnStatus                   = stPTBuff.stPTB.Status.bConnectionStatus;
        stStatus.byFISStatus                    = (BYTE)pstRspFIS->Status;

        SetLastError( ERROR_BAD_DRIVER_LEVEL );
        return( FALSE );
    }

    return( TRUE );
}

/****************************************************************************/
/* DoEnable() - Enables access to a drive's S.M.A.R.T. capabilities         */
/****************************************************************************/

static BOOL DoEnable( int iDrive )
{
    return( DoCSMIIoctl( iDrive, SMART_CMD, ENABLE_SMART, 0 ) );
}

/****************************************************************************/
/* GetPrediction() - Returns drive's S.M.A.R.T. failure prediction          */
/****************************************************************************/

static eSTATUS GetPrediction( int iDrive )
{
    if( DoCSMIIoctl( iDrive, SMART_CMD, RETURN_SMART_STATUS, 0 ) )
    {
        D2H_RFIS *pstRspFIS = (D2H_RFIS *)stPTBuff.stPTB.Status.bStatusFIS;

        // Look for negative prediction

        if( (pstRspFIS->CylinderLow == SMART_CYL_LOW) && (pstRspFIS->CylinderHigh == SMART_CYL_HI) )
            return( NO_FAILURE_PREDICTED );

        // Look for positive prediction

        if( (pstRspFIS->CylinderLow == SMART_CYL_LOW_EXC) && (pstRspFIS->CylinderHigh == SMART_CYL_HI_EXC) )
            return( FAILURE_PREDICTED );

        // We shouldn't get here; if we do, however, differentiate from IOCTL failure

        SetLastError( ERROR_SUCCESS );
    }

    return( PREDICTION_UNAVAILABLE );
}

/****************************************************************************/
/* GetAttributes() - Returns drive's S.M.A.R.T. attribute data              */
/****************************************************************************/

static BOOL GetAttributes( int iDrive, SMART_ATTRIB *pstAttrib )
{
    if( !DoCSMIIoctl( iDrive, SMART_CMD, READ_ATTRIBUTES, ONE_SECTOR ) )
        return( FALSE );

    memcpy( pstAttrib, ((SMART_ATTRIBS *)stPTBuff.stPTB.bDataBuffer)->Attribute, sizeof(SMART_ATTRIB) * SMART_MAX_ATTRIBS );
    return( TRUE );
}

/****************************************************************************/
/* GetThresholds() - Returns drive's S.M.A.R.T. threshold data              */
/****************************************************************************/

static BOOL GetThresholds( int iDrive, SMART_THRESH *pstThresh )
{
    if( !DoCSMIIoctl( iDrive, SMART_CMD, READ_THRESHOLDS, ONE_SECTOR ) )
        return( FALSE );

    memcpy( pstThresh, ((SMART_THRESHS *)stPTBuff.stPTB.bDataBuffer)->Threshold, sizeof(SMART_THRESH) * SMART_MAX_ATTRIBS );
    return( TRUE );
}

/****************************************************************************/
/* DoIdentify() - Returns drive's IDENTIFY data                             */
/****************************************************************************/

static BOOL DoIdentify( int iDrive, void *pvBuffer )
{
    if( !DoCSMIIoctl( iDrive, ID_CMD, 0, ONE_SECTOR ) )
        return( FALSE );

    memcpy( pvBuffer, stPTBuff.stPTB.bDataBuffer, ONE_SECTOR );
    return( TRUE );
}

/****************************************************************************/
/* FixRegistry() - Modifies/Creates registry entry allowing the S.M.A.R.T.  */
/* operations to get through to Intel Storage driver to the physical drive. */
/*                                                                          */
/* Note: If the Intel Storage Driver is not installed, this key will not    */
/* exist. In this case, function RegOpenKeyEx() will return exception code  */
/* ERROR_FILE_NOT_EXIST. Thus, FixRegistry() should not be considered to    */
/* have failed if this exception code is seen...                            */
/****************************************************************************/

BOOL FixRegistry( void )
{
    HKEY    hRegKey;
    DWORD   dwValue = CSMI_SECURITY_ACCESS_FULL;
    BOOL    bSuccess;

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, CSMI_SECURITY_ACCESS_KEY, 0, KEY_ALL_ACCESS, &hRegKey ) )
    {
        stStatus.dwLastError = GetLastError();
        return( FALSE );
    }

    bSuccess = RegSetValueEx( hRegKey, CSMI_SECURITY_ACCESS_ENTRY, 0, REG_DWORD, (BYTE *)&dwValue, sizeof(DWORD) ) == ERROR_SUCCESS;
    RegCloseKey( hRegKey );
    return( bSuccess );
}





/****************************************************************************/
/****************************************************************************/
/***************************** Public Functions *****************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* CSMISMARTInitialize() - Initializes module. Returns number of drives     */
/* supporting S.M.A.R.T.                                                    */
/****************************************************************************/

BOOL CSMISMARTInitialize( int *piDrives )
{
    int         iCtrl, iDrive, iMaxDrives, iDrivesDone, iSet, iSets, iPort, iPrev, iErrors = 0;
    BOOL        bFound;
    DWORD       dwError, dwLength;
    char        szDrive[16];
    unsigned    uPrevErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );

    if( !piDrives )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    if( !FixRegistry() )
    {
        // If FixRegistry() fails, either the Intel Storage Driver is not present or
        // something is buggered in the registry. For either case, we cannot continue.
        // The driver not being present is not a reason to say that the application
        // has failed, however, so we need to check for the specific exception code
        // that indicates that the driver is not present...

        *piDrives = 0;
        return( GetLastError() == ERROR_FILE_NOT_FOUND );
    }

    for( iDrives = iCtrl = 0; iCtrl < SMART_MAX_DRIVES; iCtrl++ )
    {
        // Attach to the controller

        sprintf( szDrive, "\\\\.\\SCSI%d:", iCtrl );

        stDriveData[iDrives].hDevice = CreateFile( szDrive, GENERIC_READ | GENERIC_WRITE,
                                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                   NULL, OPEN_EXISTING, 0, NULL );

        if( stDriveData[iDrives].hDevice != INVALID_HANDLE_VALUE )
        {
            // Verify that we are talking to the CSMI driver. We send the Get-Driver-Info
            // IOCTL, but only care whether the operation passes or fails...

            InitIoctlHeader( CC_CSMI_SAS_GET_DRIVER_INFO, (BYTE *)CSMI_ALL_SIGNATURE, sizeof(CSMI_SAS_DRIVER_INFO_BUFFER), 0 );

            if(    DeviceIoControl( stDriveData[iDrives].hDevice, IOCTL_SCSI_MINIPORT, &stPTBuff, sizeof(CSMI_SAS_DRIVER_INFO_BUFFER),
                                    &stPTBuff, sizeof(CSMI_SAS_DRIVER_INFO_BUFFER), &dwReturned, NULL )
                && !stPTBuff.stPTB.IoctlHeader.ReturnCode )
            {
                // (Talking to CSMI)

                // Check the version of the driver to see whether we need to enable
                // workarounds for known bugs in the storage driver...

                CSMI_SAS_DRIVER_INFO *pstInfo = &((CSMI_SAS_DRIVER_INFO_BUFFER *)&stPTBuff)->Information;

                bNeedWorkarounds =         (pstInfo->usMajorRevision <  CSMI_WORKAROUNDS_VER_MAJOR)
                                   || (    (pstInfo->usMajorRevision == CSMI_WORKAROUNDS_VER_MAJOR)
                                        && (pstInfo->usMinorRevision <= CSMI_WORKAROUNDS_VER_MINOR) );

                // Send Get-RAID-Info IOCTL. This will tell us how many drives and sets
                // (RAID arrays) there are...

                InitIoctlHeader( CC_CSMI_SAS_GET_RAID_INFO, (BYTE *)CSMI_RAID_SIGNATURE, sizeof(CSMI_SAS_RAID_INFO_BUFFER), ONE_SECTOR );

                if(    DeviceIoControl( stDriveData[iDrives].hDevice, IOCTL_SCSI_MINIPORT, &stPTBuff, sizeof(CSMI_SAS_RAID_INFO_BUFFER) + ONE_SECTOR,
                                        &stPTBuff, sizeof(CSMI_SAS_RAID_INFO_BUFFER) + ONE_SECTOR, &dwReturned, NULL )
                    && !stPTBuff.stPTB.IoctlHeader.ReturnCode )
                {
                    // (Know how many sets (arrays) and drives)

                    iSets       = ((CSMI_SAS_RAID_INFO_BUFFER *)&stPTBuff)->Information.uNumRaidSets;
                    iMaxDrives  = ((CSMI_SAS_RAID_INFO_BUFFER *)&stPTBuff)->Information.uMaxDrivesPerSet;
                    dwLength    = (iMaxDrives - 1) * sizeof(CSMI_SAS_RAID_DRIVES);

                    // Process drives in each set (array)

                    for( iDrivesDone = iSet = 0; iSet < iSets; iSet++ )
                    {
                        // Send Get-RAID-Config IOCTL to get info about the drives it uses

                        InitIoctlHeader( CC_CSMI_SAS_GET_RAID_CONFIG, (BYTE *)CSMI_RAID_SIGNATURE, sizeof(CSMI_SAS_RAID_CONFIG_BUFFER), dwLength );
                        ((CSMI_SAS_RAID_CONFIG_BUFFER *)&stPTBuff)->Configuration.uRaidSetIndex = iSet;

                        if(    DeviceIoControl( stDriveData[iDrives].hDevice, IOCTL_SCSI_MINIPORT, &stPTBuff, sizeof(CSMI_SAS_RAID_CONFIG_BUFFER) + dwLength,
                                                &stPTBuff, sizeof(CSMI_SAS_RAID_CONFIG_BUFFER) + dwLength, &dwReturned, NULL )
                            && !stPTBuff.stPTB.IoctlHeader.ReturnCode )
                        {
                            CSMI_SAS_RAID_CONFIG_BUFFER *pstConfig = (CSMI_SAS_RAID_CONFIG_BUFFER *)&stPTBuff;
                            int iDriveCount = pstConfig->Configuration.bDriveCount;

                            // Check if this array shares drives with other array(s) (so we don't duplicate the drives in our list)

                            for( iDrive = 0, bFound = FALSE; (iDrive < iDriveCount) && !bFound; iDrive++ )
                            {
                                // Fix SAS Address (if necessary)

                                if( bNeedWorkarounds )
                                {
                                    pstConfig->Configuration.Drives[iDrive].bSASAddress[CSMI_SAS_ADDRESS_PORT_ID] = pstConfig->Configuration.Drives[iDrive].bSASAddress[CSMI_SAS_ADDRESS_PATH_ID];
                                    pstConfig->Configuration.Drives[iDrive].bSASAddress[CSMI_SAS_ADDRESS_PATH_ID] = 0;
                                }

                                for( iPrev = 0; (iPrev < iDrives) && !bFound; iPrev++ )
                                {
                                    if( memcmp( stDriveData[iPrev].bySASAddress, pstConfig->Configuration.Drives[iDrive].bSASAddress, 8 ) == 0 )
                                        bFound = TRUE;
                                }
                            }

                            // If drives are unique, add them to the list

                            if( !bFound )
                            {
                                // First save off SAS Addresses, since we reuse the IOCTL buffer (in DoEnable())

                                for( iDrive = 0; iDrive < iDriveCount; iDrive++ )
                                    memcpy( stDriveData[iDrives+iDrive].bySASAddress, pstConfig->Configuration.Drives[iDrive].bSASAddress, 8 );

                                // Now check if we should support the drives (i.e. they support S.M.A.R.T.)

                                for( iDrive = 0; iDrive < iDriveCount; iDrive++ )
                                {
                                    // Enable S.M.A.R.T. Monitoring...

                                    if( DoEnable( iDrives ) )
                                    {
                                        // Successful; use next entry for checking subsequent drive(s)

                                        stDriveData[iDrives+1].hDevice = stDriveData[iDrives].hDevice;
                                        ++iDrives;
                                    }
                                }

                                // Count these drives as processed

                                iDrivesDone += iDriveCount;
                            }
                        }
                        else
                        {
                            // Get-RAID-Config IOCTL shouldn't have failed; something wrong...

                            dwError = GetLastError();
                            ++iErrors;
                            break;
                        }

                        if( iErrors )
                            break;
                    }

                    // In older (7.5 and earlier) versions of the storage driver, a bug caused drives that
                    // are not in RAID arrays (and which should be accessible using the standard S.M.A.R.T.
                    // IOCTLs) to appear to not support S.M.A.R.T. at all. For these older versions of the
                    // driver, we will handle these drives here (using the CSMI IOCTLs)...

                    if( bNeedWorkarounds )
                    {
                        // Find any drives that are not in a set (array)

                        for( iDrive = iDrivesDone; iDrive < iMaxDrives; iDrive++ )
                        {
                            memcpy( stDriveData[iDrives].bySASAddress, stDriveData[iDrives-1].bySASAddress, 8 );

                            // Search for the path (Phy/Port Id) assigned to the drive

                            for( iPort = 0; iPort < 8; iPort++ )
                            {
                                // Ignore paths of already identified drives

                                for( iPrev = iDrives - iDrivesDone; iPrev < iDrives; iPrev++ )
                                {
                                    if( iPort == stDriveData[iPrev].bySASAddress[CSMI_SAS_ADDRESS_PORT_ID] )
                                        break;
                                }

                                if( iPrev >= iDrives )
                                {
                                    // Found unhandled port id; see if drive is present on this port

                                    stDriveData[iDrives].bySASAddress[CSMI_SAS_ADDRESS_PORT_ID] = (BYTE)iPort;

                                    if( DoEnable( iDrives ) )
                                    {
                                        // Found a drive (and supports S.M.A.R.T.), assign it a home

                                        stDriveData[iDrives+1].hDevice = stDriveData[iDrives].hDevice;
                                        ++iDrives;
                                        break;
                                    }
                                }
                            }

                            // if we exhausted the paths, it means there's drives that don't support
                            // S.M.A.R.T. (which should be unlikely) or something went wrong. Either way,
                            // we aren't going to have any luck with any outstanding drives...

                            if( iPort >= 8 )
                                break;
                        }
                    }
                }
                else
                {
                    // Get-RAID-Info IOCTL shouldn't have failed; something wrong...

                    dwError = GetLastError();
                    ++iErrors;
                }

                if( !iErrors )
                    continue;   // On to next controller (keeping resources)
            }
            else
                dwError = GetLastError();

            CloseHandle( stDriveData[iDrives].hDevice );
        }

        if( iErrors )
            break;
    }

    SetErrorMode( uPrevErrorMode );

    if( iErrors )
    {
        for( iDrive = 0; iDrive < iDrives; iDrive++ )
            CloseHandle( stDriveData[iDrives].hDevice );

        *piDrives = 0;
        return( FALSE );
    }

    *piDrives = iDrives;
    return( TRUE );
}

/****************************************************************************/
/* CSMISMARTGetPhysicalIndex() - Gets physical index for specified drive    */
/****************************************************************************/

BOOL CSMISMARTGetPhysicalIndex( int iDrive, int *piIndex )
{
    if( !iDrives )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        return( FALSE );
    }

    if( (iDrive < 0) || (iDrive >= iDrives) || !piIndex )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    *piIndex = *(int *)stDriveData[iDrive].bySASAddress;
    return( TRUE );
}

/****************************************************************************/
/* CSMISMARTGetIdentifyData() - Gets the Identify data for specified drive  */
/****************************************************************************/

BOOL CSMISMARTGetIdentifyData( int iDrive, void *pvBuffer )
{
    if( !iDrives )
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
/* CSMISMARTGetPrediction() - Indicates whether drive is predicting demise  */
/****************************************************************************/

BOOL CSMISMARTGetPrediction( int iDrive, BOOL *pbFailing )
{
    if( !iDrives )
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
/* CSMISMARTGetAttributeData() - Gets Attribute data for a particular drive */
/****************************************************************************/

BOOL CSMISMARTGetAttributeData( int iDrive, SMART_ATTRIB *pstAttrib )
{
    if( !iDrives )
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
/* CSMISMARTGetThresholdData() - Gets Threshold data for a particular drive */
/****************************************************************************/

BOOL CSMISMARTGetThresholdData( int iDrive, SMART_THRESH *pstThresh )
{
    if( !iDrives )
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
/* CSMISMARTCleanup() - Cleans up resources used by the module              */
/****************************************************************************/

void CSMISMARTCleanup( void )
{
    if( iDrives )
    {
        int iDrive;

        for( iDrive = 0; iDrive < iDrives; iDrive++ )
            CloseHandle( stDriveData[iDrives].hDevice );

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
    int     iDriveCount, iDrive, iIndex;
    BOOL    bFailing;

    if( !CSMISMARTInitialize( &iDriveCount ) )
    {
        DocError( "CSMISMARTInitialize" );
        return( 1 );
    }

    printf( "\n%d SATA Drive%s with S.M.A.R.T. support detected\n\n", iDriveCount, (iDriveCount > 1)? "s" : "" );

    for( iDrive = 0; iDrive < iDriveCount; iDrive++ )
    {
        if( !CSMISMARTGetPhysicalIndex( iDrive, &iIndex ) )
        {
            DocError( "CSMISMARTGetPhysicalIndex" );
            break;
        }

        if( !CSMISMARTGetPrediction( iDrive, &bFailing ) )
        {
            DocError( "CSMISMARTGetPrediction" );
            break;
        }

        printf( "SATA Drive 0x%08X %s\n\n", iIndex, (bFailing)? "Predicting its demise!" : "Healthy" );
    }

    CSMISMARTCleanup();
    return( iDrive < iDriveCount );
}

#endif // def TESTING

