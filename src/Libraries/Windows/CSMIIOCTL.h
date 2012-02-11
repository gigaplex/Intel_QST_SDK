/****************************************************************************/
/*                                                                          */
/*  Module:         CSMIIOCTL.h                                             */
/*                                                                          */
/*  Description:    Provides the necessary definitions for the use of  the  */
/*                  subset  of  the  Common  Storage  Management Interface  */
/*                  (CSMI) IOCTLs that are necessary to obtain  S.M.A.R.T.  */
/*                  information from SATA hard drives being managed by the  */
/*                  Intel  Storage  Driver  for Windows. This includes the  */
/*                  IOCTLs that are necessary to enumerate the drives that  */
/*                  are present and  the  IOCTLs  that  are  necessary  to  */
/*                  deliver S.M.A.R.T. requests to the drives themselves.   */
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

#ifndef _CSMIIOCTL_H
#define _CSMIIOCTL_H

#pragma warning(disable:4201 4214)

#ifndef BASIC_DATA_TYPES
#define BASIC_DATA_TYPES
/****************************************************************************/
/* Basic Data Types                                                         */
/****************************************************************************/

typedef unsigned char           BYTE;
typedef unsigned short          WORD;
typedef unsigned long           DWORD;
typedef int                     BOOL;

#define FALSE                   0
#define False                   0
#define false                   0

#define TRUE                    1
#define True                    1
#define true                    1

#endif // ndef BASIC_DATA_TYPES

/****************************************************************************/
/* iaStor Port driver registry settings for security access to IOCTLs. We   */
/* need to change from DEFAULT to FULL access in order to send S.M.A.R.T.   */
/* requests to the drives...                                                */
/****************************************************************************/

#define CSMI_SECURITY_ACCESS_KEY            __TEXT("System\\CurrentControlSet\\Services\\iaStor\\Parameters")
#define CSMI_SECURITY_ACCESS_ENTRY          __TEXT("CSMI")

#define CSMI_SECURITY_ACCESS_NONE           0
#define CSMI_SECURITY_ACCESS_RESTRICTED     1
#define CSMI_SECURITY_ACCESS_LIMITED        2
#define CSMI_SECURITY_ACCESS_FULL           3
#define CSMI_SECURITY_ACCESS_DEFAULT        CSMI_SECURITY_ACCESS_LIMITED

/****************************************************************************/
/* Definitions for the IOCTL Header Structure                               */
/****************************************************************************/

// Use definitions from NTDDSCSI.H for base definition

typedef struct _SRB_IO_CONTROL
{
    DWORD                           HeaderLength;
    BYTE                            Signature[8];
    DWORD                           Timeout;
    DWORD                           ControlCode;
    DWORD                           ReturnCode;
    DWORD                           Length;

} SRB_IO_CONTROL, *PSRB_IO_CONTROL, IOCTL_HEADER, *PIOCTL_HEADER;

// Control Codes (IoctlHeader.ControlCode)

#define CC_CSMI_SAS_GET_DRIVER_INFO         1
#define CC_CSMI_SAS_GET_RAID_INFO           10
#define CC_CSMI_SAS_GET_RAID_CONFIG         11
#define CC_CSMI_SAS_STP_PASSTHRU            25

// Return codes (IoctlHeader.ReturnCode)

#define CSMI_SAS_STATUS_SUCCESS             0
#define CSMI_SAS_STATUS_FAILED              1
#define CSMI_SAS_STATUS_BAD_CNTL_CODE       2
#define CSMI_SAS_STATUS_INVALID_PARAMETER   3
#define CSMI_SAS_STATUS_WRITE_ATTEMPTED     4

// Signature value (IoctlHeader.Signature)

#define CSMI_SAS_SIGNATURE                  "CSMISAS"
#define CSMI_ALL_SIGNATURE                  "CSMIALL"
#define CSMI_RAID_SIGNATURE                 "CSMIARY"

// Timeout value (IoctlHeader.Timeout)

#define CSMI_SAS_TIMEOUT                    60

/****************************************************************************/
/* CC_CSMI_SAS_DRIVER_INFO - IOCTL control block structure                  */
/****************************************************************************/

#pragma pack(8)

typedef struct _CSMI_SAS_DRIVER_INFO
{
    BYTE                            szName[81];
    BYTE                            szDescription[81];
    WORD                            usMajorRevision;
    WORD                            usMinorRevision;
    WORD                            usBuildRevision;
    WORD                            usReleaseRevision;
    WORD                            usCSMIMajorRevision;
    WORD                            usCSMIMinorRevision;

} CSMI_SAS_DRIVER_INFO, *PCSMI_SAS_DRIVER_INFO;

typedef struct _CSMI_SAS_DRIVER_INFO_BUFFER
{
    IOCTL_HEADER                    IoctlHeader;
    CSMI_SAS_DRIVER_INFO            Information;

} CSMI_SAS_DRIVER_INFO_BUFFER, *PCSMI_SAS_DRIVER_INFO_BUFFER;

// Information about the last version of the storage driver that needs our
// workarounds for known bugs

#define CSMI_WORKAROUNDS_VER_MAJOR          7
#define CSMI_WORKAROUNDS_VER_MINOR          6

/****************************************************************************/
/* CC_CSMI_SAS_RAID_INFO - IOCTL control block structure                    */
/****************************************************************************/

typedef struct _CSMI_SAS_RAID_INFO
{
    DWORD                           uNumRaidSets;
    DWORD                           uMaxDrivesPerSet;
    BYTE                            bReserved[92];

} CSMI_SAS_RAID_INFO, *PCSMI_SAS_RAID_INFO;

typedef struct _CSMI_SAS_RAID_INFO_BUFFER
{
    IOCTL_HEADER                    IoctlHeader;
    CSMI_SAS_RAID_INFO              Information;

} CSMI_SAS_RAID_INFO_BUFFER, *PCSMI_SAS_RAID_INFO_BUFFER;

/****************************************************************************/
/* CC_CSMI_SAS_GET_RAID_CONFIG - IOCTL control block structure              */
/****************************************************************************/

typedef struct _CSMI_SAS_RAID_DRIVES
{
    BYTE                            bModel[40];
    BYTE                            bFirmware[8];
    BYTE                            bSerialNumber[40];
    BYTE                            bSASAddress[8];
    BYTE                            bSASLun[8];
    BYTE                            bDriveStatus;
    BYTE                            bDriveUsage;
    BYTE                            bReserved[30];

} CSMI_SAS_RAID_DRIVES, *PCSMI_SAS_RAID_DRIVES;

typedef struct _CSMI_SAS_RAID_CONFIG
{
    DWORD                           uRaidSetIndex;
    DWORD                           uCapacity;
    DWORD                           uStripeSize;
    BYTE                            bRaidType;
    BYTE                            bStatus;
    BYTE                            bInformation;
    BYTE                            bDriveCount;
    BYTE                            bReserved[20];
    CSMI_SAS_RAID_DRIVES            Drives[1];

} CSMI_SAS_RAID_CONFIG, *PCSMI_SAS_RAID_CONFIG;

typedef struct _CSMI_SAS_RAID_CONFIG_BUFFER
{
    IOCTL_HEADER                    IoctlHeader;
    CSMI_SAS_RAID_CONFIG            Configuration;

} CSMI_SAS_RAID_CONFIG_BUFFER, *PCSMI_SAS_RAID_CONFIG_BUFFER;

// Definitions for (byte) contents of SAS Addresses

#define CSMI_SAS_ADDRESS_LUN                0
#define CSMI_SAS_ADDRESS_TARGET_ID          1
#define CSMI_SAS_ADDRESS_PORT_ID            2
#define CSMI_SAS_ADDRESS_PATH_ID            3

/****************************************************************************/
/* CC_CSMI_SAS_STP_PASSTHRU - IOCTL control block structure                 */
/****************************************************************************/

typedef struct _CSMI_SAS_STP_PASSTHRU
{
    BYTE                            bPhyIdentifier;
    BYTE                            bPortIdentifier;
    BYTE                            bConnectionRate;
    BYTE                            bReserved;
    BYTE                            bDestinationSASAddress[8];
    BYTE                            bReserved2[4];
    BYTE                            bCommandFIS[20];
    DWORD                           uFlags;
    DWORD                           uDataLength;

} CSMI_SAS_STP_PASSTHRU, *PCSMI_SAS_STP_PASSTHRU;

typedef struct _CSMI_SAS_STP_PASSTHRU_STATUS
{
    BYTE                            bConnectionStatus;
    BYTE                            bReserved[3];
    BYTE                            bStatusFIS[20];
    DWORD                           uSCR[16];
    DWORD                           uDataBytes;

} CSMI_SAS_STP_PASSTHRU_STATUS, *PCSMI_SAS_STP_PASSTHRU_STATUS;

typedef struct _CSMI_SAS_STP_PASSTHRU_BUFFER
{
    IOCTL_HEADER                    IoctlHeader;
    CSMI_SAS_STP_PASSTHRU           Parameters;
    CSMI_SAS_STP_PASSTHRU_STATUS    Status;
    BYTE                            bDataBuffer[1];

} CSMI_SAS_STP_PASSTHRU_BUFFER, *PCSMI_SAS_STP_PASSTHRU_BUFFER;

// STP Flags (Parameters.uFlags)

#define CSMI_SAS_STP_READ                   0x00000001
#define CSMI_SAS_STP_DMA                    0x00000020

/****************************************************************************/
/* Frame Information Structures (Host-to-Device, Device-to-Host)            */
/****************************************************************************/

// FIS Types

#define FIS_TYPE_REG_H2D                    0x27
#define FIS_TYPE_REG_D2H                    0x34

// H2D Register FIS

typedef struct _H2D_RFIS
{
    // DWORD 0

    union
    {
        DWORD                       Dw0;
        struct
        {
            DWORD                   FisType:8;
            DWORD                   PmPort:4;
            DWORD                   Reserved1:1;
            DWORD                   Reserved2:1;
            DWORD                   Reserved3:1;
            DWORD                   CommandOrDeviceControl:1;
            DWORD                   Command:8;
            DWORD                   Features:8;
        };
    };

    // DWORD 1
    union
    {
        DWORD                       Dw1;
        struct
        {
            DWORD                   SectorNumber:8;
            DWORD                   CylinderLow:8;
            DWORD                   CylinderHigh:8;
            DWORD                   DeviceHead:8;
        };
    };

    // DWORD 2

    union
    {
        DWORD                       Dw2;
        struct
        {
            DWORD                   SectorNumberExp:8;
            DWORD                   CylinderLowExp:8;
            DWORD                   CylinderHighExp:8;
            DWORD                   FeaturesExp:8;
        };
    };

    // DWORD 3
    union
    {
        DWORD                       Dw3;
        struct
        {
            DWORD                   SectorCount:8;
            DWORD                   SectorCountExp:8;
            DWORD                   Reserved4:8;
            DWORD                   Control:8;
        };
    };

    // DWORD 4
    union
    {
        DWORD                       Dw4;
        DWORD                       Reserved5;
    };

} H2D_RFIS, *PH2D_RFIS;

// D2H Register FIS

typedef struct _D2H_RFIS
{
    // DWORD 0
    union
    {
        DWORD                       Dw0;
        struct
        {
            DWORD                   FisType:8;
            DWORD                   PmPort:4;
            DWORD                   Reserved1:1;
            DWORD                   Reserved2:1;
            DWORD                   Interrupt:1;
            DWORD                   Reserved3:1;
            DWORD                   Status:8;
            DWORD                   Error:8;
        };
    };

    // DWORD 1
    union
    {
        DWORD                       Dw1;
        struct
        {
            DWORD                   SectorNumber:8;
            DWORD                   CylinderLow:8;
            DWORD                   CylinderHigh:8;
            DWORD                   DeviceHead:8;
        };
    };

    // DWORD 2
    union
    {
        DWORD                       Dw2;
        struct
        {
            DWORD                   SectorNumberExp:8;
            DWORD                   CylinderLowExp:8;
            DWORD                   CylinderHighExp:8;
            DWORD                   Reserved4:8;
        };
    };

    // DWORD 3
    union
    {
        DWORD                       Dw3;
        struct
        {
            DWORD                   SectorCount:8;
            DWORD                   SectorCountExp:8;
            DWORD                   Reserved5:16;
        };
    };

    // DWORD 4
    union
    {
        DWORD                       Dw4;
        DWORD                       Reserved6;
    };

} D2H_RFIS, *PD2H_RFIS;

/****************************************************************************/
/* Definitions for S.M.A.R.T.                                               */
/****************************************************************************/

// IOCTLs

#define IOCTL_SCSI_BASE                     FILE_DEVICE_CONTROLLER
#define IOCTL_SCSI_MINIPORT                 CTL_CODE(IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// S.M.A.R.T. Commands

#define READ_ATTRIBUTES                     0xD0
#define READ_THRESHOLDS                     0xD1
#define ENABLE_SMART                        0xD8
#define RETURN_SMART_STATUS                 0xDA

#endif // ndef _CSMIIOCTL_H
