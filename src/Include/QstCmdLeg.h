/****************************************************************************/
/*                                                                          */
/*  Module:         QstCmdLeg.c                                             */
/*                                                                          */
/*  Description:    Provides definitions for legacy  1.x  command  packets  */
/*                  that were used to query & control previous generations  */
/*                  of  the  Intel(R)  Quiet  System  Technology (QST) F/W  */
/*                  subsystem and the various sensors and controllers that  */
/*                  were managed by it. For a time, the QST Communications  */
/*                  Library will provide a compatibility layer that allows  */
/*                  the  use  of  most  of  these 1.x commands (all except  */
/*                  QST_SET_SUBSYSTEM_CONFIG) from legacy applications.     */
/*                                                                          */
/*  Notes:      1.  Applications  developers  should  endeavor  to upgrade  */
/*                  their applications to use the 2.0 command set as  soon  */
/*                  as  possible.  Support  for the Compatibility Layer is  */
/*                  likely available for only a single F/W generation...    */
/*                                                                          */
/*              2.  While they  are  defined  within  this  file,  certain  */
/*                  commands  may only be sent to the QST Subsystem by the  */
/*                  BIOS during POST.                                       */
/*                                                                          */
/*              3.  Commands that modify  the  configuration  of  the  QST  */
/*                  Subsystem may be rejected, depending upon the Security  */
/*                  Mask specified by the BIOS during POST. Similarly, SST  */
/*                  (Pass-Through)  commands that request the modification  */
/*                  of the operation of the sensors and controllers within  */
/*                  Chipset and SST Devices may also be rejected.           */
/*                                                                          */
/*              4.  Command and response  packets  that  include  no  data  */
/*                  (i.e.  which  require  only  the header/status fields)  */
/*                  will not have structures defined for them.              */
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

#ifndef _QSTCMDLEG_H
#define _QSTCMDLEG_H

#include "QstCfgLeg.h"

#pragma pack(1)

/****************************************************************************/
/****************************************************************************/
/************** Definitions for QST Subsystem Command Support ***************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Definitions for structure maximums                                       */
/****************************************************************************/

#define QST_LEG_MAX_TEMP_MONITORS               12
#define QST_LEG_MAX_FAN_MONITORS                8
#define QST_LEG_MAX_VOLT_MONITORS               8
#define QST_LEG_MAX_TEMP_RESPONSES              12
#define QST_LEG_MAX_FAN_CONTROLLERS             8

/****************************************************************************/
/* QST Subsystem Command Codes                                              */
/****************************************************************************/

#define QST_LEG_GET_SUBSYSTEM_STATUS            0x00
#define QST_LEG_GET_SUBSYSTEM_CONFIG            0x01 // cannot be used
#define QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE    0x02
#define QST_LEG_SET_SUBSYSTEM_CONFIG            0x03 // cannot be used
#define QST_LEG_LOCK_SUBSYSTEM                  0x04

#define QST_LEG_UPDATE_CPU_1_CONFIG             0x05
#define QST_LEG_UPDATE_CPU_2_CONFIG             0x06
#define QST_LEG_UPDATE_CPU_3_CONFIG             0x07
#define QST_LEG_UPDATE_CPU_4_CONFIG             0x08

#define QST_LEG_UPDATE_FAN_1_CONFIG             0x09
#define QST_LEG_UPDATE_FAN_2_CONFIG             0x0A
#define QST_LEG_UPDATE_FAN_3_CONFIG             0x0B
#define QST_LEG_UPDATE_FAN_4_CONFIG             0x0C
#define QST_LEG_UPDATE_FAN_5_CONFIG             0x0D
#define QST_LEG_UPDATE_FAN_6_CONFIG             0x0E
#define QST_LEG_UPDATE_FAN_7_CONFIG             0x0F
#define QST_LEG_UPDATE_FAN_8_CONFIG             0x10

#define QST_LEG_SST_PASS_THROUGH                0x11

#define QST_LEG_GET_TEMP_MON_UPDATE             0x12

#define QST_LEG_GET_TEMP_MON_1_CONFIG           0x13
#define QST_LEG_GET_TEMP_MON_2_CONFIG           0x14
#define QST_LEG_GET_TEMP_MON_3_CONFIG           0x15
#define QST_LEG_GET_TEMP_MON_4_CONFIG           0x16
#define QST_LEG_GET_TEMP_MON_5_CONFIG           0x17
#define QST_LEG_GET_TEMP_MON_6_CONFIG           0x18
#define QST_LEG_GET_TEMP_MON_7_CONFIG           0x19
#define QST_LEG_GET_TEMP_MON_8_CONFIG           0x1A
#define QST_LEG_GET_TEMP_MON_9_CONFIG           0x1B
#define QST_LEG_GET_TEMP_MON_10_CONFIG          0x1C
#define QST_LEG_GET_TEMP_MON_11_CONFIG          0x1D
#define QST_LEG_GET_TEMP_MON_12_CONFIG          0x1E

#define QST_LEG_SET_TEMP_MON_1_THRESHOLDS       0x1F
#define QST_LEG_SET_TEMP_MON_2_THRESHOLDS       0x20
#define QST_LEG_SET_TEMP_MON_3_THRESHOLDS       0x21
#define QST_LEG_SET_TEMP_MON_4_THRESHOLDS       0x22
#define QST_LEG_SET_TEMP_MON_5_THRESHOLDS       0x23
#define QST_LEG_SET_TEMP_MON_6_THRESHOLDS       0x24
#define QST_LEG_SET_TEMP_MON_7_THRESHOLDS       0x25
#define QST_LEG_SET_TEMP_MON_8_THRESHOLDS       0x26
#define QST_LEG_SET_TEMP_MON_9_THRESHOLDS       0x27
#define QST_LEG_SET_TEMP_MON_10_THRESHOLDS      0x28
#define QST_LEG_SET_TEMP_MON_11_THRESHOLDS      0x29
#define QST_LEG_SET_TEMP_MON_12_THRESHOLDS      0x2A

#define QST_LEG_SET_TEMP_MON_1_READING          0x2B
#define QST_LEG_SET_TEMP_MON_2_READING          0x2C
#define QST_LEG_SET_TEMP_MON_3_READING          0x2D
#define QST_LEG_SET_TEMP_MON_4_READING          0x2E
#define QST_LEG_SET_TEMP_MON_5_READING          0x2F
#define QST_LEG_SET_TEMP_MON_6_READING          0x30
#define QST_LEG_SET_TEMP_MON_7_READING          0x31
#define QST_LEG_SET_TEMP_MON_8_READING          0x32
#define QST_LEG_SET_TEMP_MON_9_READING          0x33
#define QST_LEG_SET_TEMP_MON_10_READING         0x34
#define QST_LEG_SET_TEMP_MON_11_READING         0x35
#define QST_LEG_SET_TEMP_MON_12_READING         0x36

#define QST_LEG_GET_FAN_MON_UPDATE              0x37

#define QST_LEG_GET_FAN_MON_1_CONFIG            0x38
#define QST_LEG_GET_FAN_MON_2_CONFIG            0x39
#define QST_LEG_GET_FAN_MON_3_CONFIG            0x3A
#define QST_LEG_GET_FAN_MON_4_CONFIG            0x3B
#define QST_LEG_GET_FAN_MON_5_CONFIG            0x3C
#define QST_LEG_GET_FAN_MON_6_CONFIG            0x3D
#define QST_LEG_GET_FAN_MON_7_CONFIG            0x3E
#define QST_LEG_GET_FAN_MON_8_CONFIG            0x3F

#define QST_LEG_SET_FAN_MON_1_THRESHOLDS        0x40
#define QST_LEG_SET_FAN_MON_2_THRESHOLDS        0x41
#define QST_LEG_SET_FAN_MON_3_THRESHOLDS        0x42
#define QST_LEG_SET_FAN_MON_4_THRESHOLDS        0x43
#define QST_LEG_SET_FAN_MON_5_THRESHOLDS        0x44
#define QST_LEG_SET_FAN_MON_6_THRESHOLDS        0x45
#define QST_LEG_SET_FAN_MON_7_THRESHOLDS        0x46
#define QST_LEG_SET_FAN_MON_8_THRESHOLDS        0x47

#define QST_LEG_ENABLE_FAN_MON_1                0x48
#define QST_LEG_ENABLE_FAN_MON_2                0x49
#define QST_LEG_ENABLE_FAN_MON_3                0x4A
#define QST_LEG_ENABLE_FAN_MON_4                0x4B
#define QST_LEG_ENABLE_FAN_MON_5                0x4C
#define QST_LEG_ENABLE_FAN_MON_6                0x4D
#define QST_LEG_ENABLE_FAN_MON_7                0x4E
#define QST_LEG_ENABLE_FAN_MON_8                0x4F

#define QST_LEG_DISABLE_FAN_MON_1               0x50
#define QST_LEG_DISABLE_FAN_MON_2               0x51
#define QST_LEG_DISABLE_FAN_MON_3               0x52
#define QST_LEG_DISABLE_FAN_MON_4               0x53
#define QST_LEG_DISABLE_FAN_MON_5               0x54
#define QST_LEG_DISABLE_FAN_MON_6               0x55
#define QST_LEG_DISABLE_FAN_MON_7               0x56
#define QST_LEG_DISABLE_FAN_MON_8               0x57

#define QST_LEG_GET_VOLT_MON_UPDATE             0x58

#define QST_LEG_GET_VOLT_MON_1_CONFIG           0x59
#define QST_LEG_GET_VOLT_MON_2_CONFIG           0x5A
#define QST_LEG_GET_VOLT_MON_3_CONFIG           0x5B
#define QST_LEG_GET_VOLT_MON_4_CONFIG           0x5C
#define QST_LEG_GET_VOLT_MON_5_CONFIG           0x5D
#define QST_LEG_GET_VOLT_MON_6_CONFIG           0x5E
#define QST_LEG_GET_VOLT_MON_7_CONFIG           0x5F
#define QST_LEG_GET_VOLT_MON_8_CONFIG           0x60

#define QST_LEG_SET_VOLT_MON_1_THRESHOLDS       0x61
#define QST_LEG_SET_VOLT_MON_2_THRESHOLDS       0x62
#define QST_LEG_SET_VOLT_MON_3_THRESHOLDS       0x63
#define QST_LEG_SET_VOLT_MON_4_THRESHOLDS       0x64
#define QST_LEG_SET_VOLT_MON_5_THRESHOLDS       0x65
#define QST_LEG_SET_VOLT_MON_6_THRESHOLDS       0x66
#define QST_LEG_SET_VOLT_MON_7_THRESHOLDS       0x67
#define QST_LEG_SET_VOLT_MON_8_THRESHOLDS       0x68

#define QST_LEG_GET_FAN_CTRL_UPDATE             0x69

#define QST_LEG_GET_FAN_CTRL_1_CONFIG           0x6A
#define QST_LEG_GET_FAN_CTRL_2_CONFIG           0x6B
#define QST_LEG_GET_FAN_CTRL_3_CONFIG           0x6C
#define QST_LEG_GET_FAN_CTRL_4_CONFIG           0x6D
#define QST_LEG_GET_FAN_CTRL_5_CONFIG           0x6E
#define QST_LEG_GET_FAN_CTRL_6_CONFIG           0x6F
#define QST_LEG_GET_FAN_CTRL_7_CONFIG           0x70
#define QST_LEG_GET_FAN_CTRL_8_CONFIG           0x71

#define QST_LEG_SET_FAN_CTRL_1_DUTY             0x72
#define QST_LEG_SET_FAN_CTRL_2_DUTY             0x73
#define QST_LEG_SET_FAN_CTRL_3_DUTY             0x74
#define QST_LEG_SET_FAN_CTRL_4_DUTY             0x75
#define QST_LEG_SET_FAN_CTRL_5_DUTY             0x76
#define QST_LEG_SET_FAN_CTRL_6_DUTY             0x77
#define QST_LEG_SET_FAN_CTRL_7_DUTY             0x78
#define QST_LEG_SET_FAN_CTRL_8_DUTY             0x79

#define QST_LEG_SET_FAN_CTRL_1_AUTO             0x7A
#define QST_LEG_SET_FAN_CTRL_2_AUTO             0x7B
#define QST_LEG_SET_FAN_CTRL_3_AUTO             0x7C
#define QST_LEG_SET_FAN_CTRL_4_AUTO             0x7D
#define QST_LEG_SET_FAN_CTRL_5_AUTO             0x7E
#define QST_LEG_SET_FAN_CTRL_6_AUTO             0x7F
#define QST_LEG_SET_FAN_CTRL_7_AUTO             0x80
#define QST_LEG_SET_FAN_CTRL_8_AUTO             0x81

#define QST_LEG_GET_CPU_1_CONFIG_UPDATE         0x82
#define QST_LEG_GET_CPU_2_CONFIG_UPDATE         0x83
#define QST_LEG_GET_CPU_3_CONFIG_UPDATE         0x84
#define QST_LEG_GET_CPU_4_CONFIG_UPDATE         0x85

#define QST_LEG_GET_FAN_1_CONFIG_UPDATE         0x86
#define QST_LEG_GET_FAN_2_CONFIG_UPDATE         0x87
#define QST_LEG_GET_FAN_3_CONFIG_UPDATE         0x88
#define QST_LEG_GET_FAN_4_CONFIG_UPDATE         0x89
#define QST_LEG_GET_FAN_5_CONFIG_UPDATE         0x8A
#define QST_LEG_GET_FAN_6_CONFIG_UPDATE         0x8B
#define QST_LEG_GET_FAN_7_CONFIG_UPDATE         0x8C
#define QST_LEG_GET_FAN_8_CONFIG_UPDATE         0x8D

#define QST_LEG_RESET_FAN_CTRL_1_MIN_DUTY       0x8E
#define QST_LEG_RESET_FAN_CTRL_2_MIN_DUTY       0x8F
#define QST_LEG_RESET_FAN_CTRL_3_MIN_DUTY       0x90
#define QST_LEG_RESET_FAN_CTRL_4_MIN_DUTY       0x91
#define QST_LEG_RESET_FAN_CTRL_5_MIN_DUTY       0x92
#define QST_LEG_RESET_FAN_CTRL_6_MIN_DUTY       0x93
#define QST_LEG_RESET_FAN_CTRL_7_MIN_DUTY       0x94
#define QST_LEG_RESET_FAN_CTRL_8_MIN_DUTY       0x95

#define QST_LEG_REDETECT_FAN_PRESENCE           0x96

#define QST_LEG_NO_TEMP_MON_1_READINGS          0x97
#define QST_LEG_NO_TEMP_MON_2_READINGS          0x98
#define QST_LEG_NO_TEMP_MON_3_READINGS          0x99
#define QST_LEG_NO_TEMP_MON_4_READINGS          0x9A
#define QST_LEG_NO_TEMP_MON_5_READINGS          0x9B
#define QST_LEG_NO_TEMP_MON_6_READINGS          0x9C
#define QST_LEG_NO_TEMP_MON_7_READINGS          0x9D
#define QST_LEG_NO_TEMP_MON_8_READINGS          0x9E
#define QST_LEG_NO_TEMP_MON_9_READINGS          0x9F
#define QST_LEG_NO_TEMP_MON_10_READINGS         0xA0
#define QST_LEG_NO_TEMP_MON_11_READINGS         0xA1
#define QST_LEG_NO_TEMP_MON_12_READINGS         0xA2

#define QST_LEG_LAST_CMD_CODE                   0xA2

/****************************************************************************/
/* Response Codes                                                           */
/****************************************************************************/

#define QST_CMD_SUCCESSFUL                      0x00
#define QST_CMD_REJECTED_UNSUPPORTED            0x01
#define QST_CMD_REJECTED_LOCKED                 0x02
#define QST_CMD_REJECTED_PARAMETER              0x03
#define QST_CMD_REJECTED_VERSION                0x04
#define QST_CMD_FAILED_COMM_ERROR               0x05
#define QST_CMD_FAILED_SENSOR_ERROR             0x06
#define QST_CMD_FAILED_NO_MEMORY                0x07
#define QST_CMD_FAILED_NO_RESOURCES             0x08
#define QST_CMD_REJECTED_INVALID                0x09
#define QST_CMD_REJECTED_CMD_SIZE               0x0A
#define QST_CMD_REJECTED_RSP_SIZE               0x0B

/****************************************************************************/
/* QST_LEG_CMD_HEADER - Command header structure definition                 */
/****************************************************************************/

typedef struct _QST_LEG_CMD_HEADER
{
   UINT8                        byCommand;
   UINT16                       wCommandLength;
   UINT16                       wResponseLength;

} QST_LEG_CMD_HEADER, *P_QST_LEG_CMD_HEADER;

/****************************************************************************/
/* QST_LEG_GENERIC_CMD - Generic command structure definition               */
/****************************************************************************/

typedef struct _QST_LEG_GENERIC_CMD
{
   QST_LEG_CMD_HEADER           stHeader;

} QST_LEG_GENERIC_CMD, *P_QST_LEG_GENERIC_CMD;

// Useful Macros

#define QST_LEG_CMD_DATA_SIZE(x) (sizeof(x) - sizeof(QST_LEG_CMD_HEADER))

/****************************************************************************/
/* QST_LEG_GENERIC_RSP - Generic response structure definition              */
/****************************************************************************/

typedef struct _QST_LEG_GENERIC_RSP
{
   UINT8                        byStatus;

} QST_LEG_GENERIC_RSP, *P_QST_LEG_GENERIC_RSP;

/****************************************************************************/
/* QST_LEG_SUBSYSTEM_STATUS - Subsystem Status byte structure definition    */
/****************************************************************************/

typedef struct _QST_LEG_SUBSYSTEM_STATUS
{
   BIT_FIELD_IN_UINT8           bSubsystemConfigured: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullError: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullCritical: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullFailure: 1;
   BIT_FIELD_IN_UINT8           uReserved: 4;

} QST_LEG_SUBSYSTEM_STATUS, *P_QST_LEG_SUBSYSTEM_STATUS;

/****************************************************************************/
/* QST_LEG_LOCK_MASK - Lock Mask structure definition                       */
/****************************************************************************/

typedef struct _QST_LEG_LOCK_MASK
{
   BIT_FIELD_IN_UINT16          bLockConfiguration: 1;
   BIT_FIELD_IN_UINT16          bLockSSTBusAccess: 1;
   BIT_FIELD_IN_UINT16          bLockThresholds: 1;
   BIT_FIELD_IN_UINT16          bLockManualFanControl: 1;
   BIT_FIELD_IN_UINT16          bLockChipsetAccess: 1;
   BIT_FIELD_IN_UINT16          uReserved: 11;

} QST_LEG_LOCK_MASK;

/****************************************************************************/
/* QST_LEG_CONFIG_STATUS - Configuration Status byte structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_CONFIG_STATUS
{
   BIT_FIELD_IN_UINT16          bConfigSuccessful: 1;
   BIT_FIELD_IN_UINT16          uFailingEntityType: 3;
   BIT_FIELD_IN_UINT16          uFailingEntityIndex: 4;
   BIT_FIELD_IN_UINT16          uFailingEntityParam: 6;
   BIT_FIELD_IN_UINT16          uReserved: 2;

} QST_LEG_CONFIG_STATUS;

/****************************************************************************/
/* QST_LEG_GET_SUBSYSTEM_STATUS_RSP - Command response structure definition */
/****************************************************************************/

typedef struct _QST_LEG_GET_SUBSYSTEM_STATUS_RSP
{
   UINT8                        byStatus;
   QST_LEG_SUBSYSTEM_STATUS     stSubsystemStatus;
   QST_LEG_LOCK_MASK            stLockMask;
   QST_LEG_CONFIG_STATUS        stConfigStatus;

} QST_LEG_GET_SUBSYSTEM_STATUS_RSP, *P_QST_LEG_GET_SUBSYSTEM_STATUS_RSP;

/****************************************************************************/
/* QST_LEG_GET_SUBSYSTEM_CONFIG_RSP - Command response structure definition */
/* Note: QST_LEG_PAYLOAD_STRUCT is defined in Header file QstCfg.h.         */
/****************************************************************************/

typedef struct _QST_LEG_GET_SUBSYSTEM_CONFIG_RSP
{
   UINT8                        byStatus;
   QST_LEG_PAYLOAD_STRUCT       stConfigPayload;

} QST_LEG_GET_SUBSYSTEM_CONFIG_RSP, *P_QST_LEG_GET_SUBSYSTEM_CONFIG_RSP;

/****************************************************************************/
/* QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP - Command response structure    */
/* definition                                                               */
/****************************************************************************/

typedef struct _QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP
{
   UINT8                        byStatus;
   UINT16                       wTempMonsConfigured;
   UINT8                        byVoltMonsConfigured;
   UINT8                        byFanMonsConfigured;
   UINT16                       byTempRespConfigured;
   UINT8                        byFanCtrlConfigured;

} QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP, *P_QST_LEG_GET_SUBSYSTEM_CONFIG_PROFILE_RSP;

// Useful Macros

#ifndef SET_BIT
#define SET_BIT(var,bit)                        ((var) |= 1 << (bit))
#define BIT_SET(var,bit)                        (((var) & (1 << (bit))) != 0)
#define CLR_BIT(var,bit)                        ((var) &= (~(1 << (bit))))
#endif

/****************************************************************************/
/* QST_LEG_SET_SUBSYSTEM_CONFIG_CMD - Command structure definition.         */
/* Note: QST_LEG_PAYLOAD_STRUCT is defined in Header file QstCfg.h.         */
/****************************************************************************/

typedef struct _QST_LEG_SET_SUBSYSTEM_CONFIG_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   QST_LEG_PAYLOAD_STRUCT       stConfigPayload;

} QST_LEG_SET_SUBSYSTEM_CONFIG_CMD, *P_QST_LEG_SET_SUBSYSTEM_CONFIG_CMD;

/****************************************************************************/
/* QST_LEG_LOCK_SUBSYSTEM_CMD - Command structure definition                */
/****************************************************************************/

typedef struct _QST_LEG_LOCK_SUBSYSTEM_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   QST_LEG_LOCK_MASK            stLockMask;

} QST_LEG_LOCK_SUBSYSTEM_CMD, *P_QST_LEG_LOCK_SUBSYSTEM_CMD;

/****************************************************************************/
/* QST_LEG_CPU_CONFIG - CPU Configuration byte structure definition         */
/****************************************************************************/

typedef struct _QST_LEG_CPU_CONFIG
{
   BIT_FIELD_IN_UINT8           bRelativeTemp: 1;
   BIT_FIELD_IN_UINT8           bUsePECI: 1;
   BIT_FIELD_IN_UINT8           uTempInputs: 4;
   BIT_FIELD_IN_UINT8           uReserved: 2;

} QST_LEG_CPU_CONFIG;

/****************************************************************************/
/* QST_LEG_SST_CMD_DESCR - SST Command Descriptor structure definition      */
/****************************************************************************/

typedef struct _QST_LEG_SST_CMD_DESCR
{
   UINT8                        bySSTDeviceAddress;
   UINT8                        bySSTDeviceCommand;
   INT16                        wTempCorrectionOffset;
   INT16                        wTempCorrectionSlope;

} QST_LEG_SST_CMD_DESCR;

/****************************************************************************/
/* QST_LEG_UPDATE_CPU_CONFIG_CMD - Command structure definition             */
/****************************************************************************/

typedef struct _QST_LEG_UPDATE_CPU_CONFIG_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   INT16                        wTcontrolTemp;
   INT16                        wTempConversion;
   QST_LEG_CPU_CONFIG           stCPUConfig;
   QST_LEG_SST_CMD_DESCR        stSSTCmdDescr[16];

} QST_LEG_UPDATE_CPU_CONFIG_CMD, *P_QST_LEG_UPDATE_CPU_CONFIG_CMD;

// Useful Macros

#define QST_LEG_UPDATE_CPU_CMD_SIZE(NumDomains) (sizeof(QST_LEG_UPDATE_CPU_CONFIG_CMD) - (sizeof(QST_LEG_SST_CMD_DESCR) * (16 - (NumDomains))))
#define QST_LEG_UPDATE_CPU_CMD_DATA(NumDomains) (QST_LEG_UPDATE_CPU_CMD_SIZE(NumDomains) - sizeof(QST_LEG_CMD_HEADER))

/****************************************************************************/
/* QST_LEG_GET_CPU_CONFIG_UPDATE_RSP - Command response structure definition*/
/****************************************************************************/

#define QST_LEG_MAX_CPU_SENSORS     16

typedef struct _QST_LEG_GET_CPU_CONFIG_UPDATE_RSP
{
   UINT8                        byStatus;
   INT8                         bUpdateAvailable;
   INT16                        wTcontrolTemp;
   INT16                        wTempConversion;
   QST_LEG_CPU_CONFIG           stCPUConfig;
   QST_LEG_SST_CMD_DESCR        stSSTCmdDescr[QST_LEG_MAX_CPU_SENSORS];

} QST_LEG_GET_CPU_CONFIG_UPDATE_RSP, *P_QST_LEG_GET_CPU_CONFIG_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_FAN_CTRL_CONFIG - Fan Controller Configuration Word structure    */
/* definition. Note: Literals for Signal Frequency and Spin-Up Time are     */
/* defined in QstCfg.h                                                      */
/****************************************************************************/

typedef struct _QST_LEG_FAN_CTRL_CONFIG
{
   BIT_FIELD_IN_UINT16          uPWMSignalFrequency: 3;
   BIT_FIELD_IN_UINT16          uSpinUpTime: 3;
   BIT_FIELD_IN_UINT16          bSignalInvert: 1;
   BIT_FIELD_IN_UINT16          uReserved: 9;

} QST_LEG_FAN_CTRL_CONFIG;

/****************************************************************************/
/* QST_LEG_FAN_SENSOR_CONFIG - Fan Sensor Configuration Word structure      */
/* definition. Definitions for uPulsesPerRevolution are in QstCfg.h         */
/****************************************************************************/

typedef struct _QST_LEG_FAN_SENSOR_CONFIG
{
   BIT_FIELD_IN_UINT16          uPulsesPerRevolution: 3;
   BIT_FIELD_IN_UINT16          bDependentMeasurement: 1;
   BIT_FIELD_IN_UINT16          uReserved: 11;
   BIT_FIELD_IN_UINT16          bSensorPresent:1;

} QST_LEG_FAN_SENSOR_CONFIG;

/****************************************************************************/
/* QST_LEG_FAN_SENSOR_ASSOC - Associated Fan Sensor configuration field     */
/* structure definition.                                                    */
/****************************************************************************/

typedef struct _QST_LEG_FAN_SENSOR_ASSOC
{
   QST_LEG_FAN_SENSOR_CONFIG    stFanSensorConfig;
   UINT16                       wFanMinimumRPM;

} QST_LEG_FAN_SENSOR_ASSOC;

/****************************************************************************/
/* QST_LEG_UPDATE_FAN_CONFIG_CMD - Command structure definition             */
/****************************************************************************/

typedef struct _QST_LEG_UPDATE_FAN_CONFIG_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   QST_LEG_FAN_CTRL_CONFIG      stFanCtrlConfig;
   QST_LEG_FAN_SENSOR_ASSOC     stFanSensorAssoc[4];
   UINT8                        byMode;
   UINT8                        byDutyCycleMin;
   UINT8                        byDutyCycleOn;
   UINT8                        byDutyCycleMax;

} QST_LEG_UPDATE_FAN_CONFIG_CMD, *P_QST_LEG_UPDATE_FAN_CONFIG_CMD;

// MIN/OFF Mode (byMode field) Values

#define QST_MIN_MODE                            0
#define QST_OFF_MODE                            1

/****************************************************************************/
/* QST_LEG_GET_FAN_CONFIG_UPDATE_RSP - Command response structure definition*/
/****************************************************************************/

typedef struct _QST_LEG_GET_FAN_CONFIG_UPDATE_RSP
{
   UINT8                        byStatus;
   INT8                         bUpdateAvailable;
   QST_LEG_FAN_CTRL_CONFIG      stFanCtrlConfig;
   QST_LEG_FAN_SENSOR_ASSOC     stFanSensorAssoc[4];
   UINT8                        byMode;
   UINT8                        byDutyCycleMin;
   UINT8                        byDutyCycleOn;
   UINT8                        byDutyCycleMax;

} QST_LEG_GET_FAN_CONFIG_UPDATE_RSP, *P_QST_LEG_GET_FAN_CONFIG_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_MON_HEALTH_STATUS - Monitor Health Status structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_MON_HEALTH_STATUS
{
   BIT_FIELD_IN_UINT8           bMonitorEnabled: 1;
   BIT_FIELD_IN_UINT8           uMonitorStatus: 2;
   BIT_FIELD_IN_UINT8           uThresholdStatus: 2;
   BIT_FIELD_IN_UINT8           bSensorReadingError: 1;
   BIT_FIELD_IN_UINT8           bSensorCommError: 1;
   BIT_FIELD_IN_UINT8           uReserved: 1;

} QST_LEG_MON_HEALTH_STATUS;

// Health Status (uMonitorStatus field) Values

#define QST_STATUS_NORMAL                       0
#define QST_STATUS_NON_CRITICAL                 1
#define QST_STATUS_CRITICAL                     2
#define QST_STATUS_NON_RECOVERABLE              3

/****************************************************************************/
/* QST_LEG_TEMP_MON_UPDATE - Temperature Monitor Update structure definition*/
/****************************************************************************/

typedef struct _QST_LEG_TEMP_MON_UPDATE
{
   QST_LEG_MON_HEALTH_STATUS    stMonitorStatus;
   INT32F                       lfCurrentReading;

} QST_LEG_TEMP_MON_UPDATE;

// useful Macros

#define QST_TEMP_TO_FLOAT(x)                    ((float)(x) / 100)
#define QST_TEMP_TO_DOUBLE(x)                   ((double)(x) / 100)

/****************************************************************************/
/* QST_LEG_GET_TEMP_MON_UPDATE_RSP - Command response structure definition  */
/****************************************************************************/

typedef struct _QST_LEG_GET_TEMP_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_LEG_TEMP_MON_UPDATE      stMonitorUpdate[QST_LEG_MAX_TEMP_MONITORS];

} QST_LEG_GET_TEMP_MON_UPDATE_RSP, *P_QST_LEG_GET_TEMP_MON_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_GET_TEMP_MON_CONFIG_RSP - Command response structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_GET_TEMP_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   INT8                         bRelativeReadings;
   INT32F                       lfTempNominal;
   INT32F                       lfTempNonCritical;
   INT32F                       lfTempCritical;
   INT32F                       lfTempNonRecoverable;

} QST_LEG_GET_TEMP_MON_CONFIG_RSP, *P_QST_LEG_GET_TEMP_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_LEG_SET_TEMP_MON_THRESHOLDS_CMD - Command structure definition       */
/****************************************************************************/

typedef struct _QST_LEG_SET_TEMP_MON_THRESHOLDS_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   INT32F                       lfTempNonCritical;
   INT32F                       lfTempCritical;
   INT32F                       lfTempNonRecoverable;

} QST_LEG_SET_TEMP_MON_THRESHOLDS_CMD, *P_QST_LEG_SET_TEMP_MON_THRESHOLDS_CMD;

// Useful Macros

#define QST_TEMP_FROM_FLOAT(x)              ((INT32F)((x) * 100))
#define QST_TEMP_FROM_DOUBLE(x)             ((INT32F)((x) * 100))

/****************************************************************************/
/* QST_LEG_SET_TEMP_MON_READING_CMD - Command structure definition          */
/****************************************************************************/

typedef struct _QST_LEG_SET_TEMP_MON_READING_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   INT32F                       lfTempReading;

} QST_LEG_SET_TEMP_MON_READING_CMD, *P_QST_LEG_SET_TEMP_MON_READING_CMD;

/****************************************************************************/
/* QST_LEG_FAN_MON_UPDATE - Fan Speed Monitor Update structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_FAN_MON_UPDATE
{
   QST_LEG_MON_HEALTH_STATUS    stMonitorStatus;
   UINT16                       uCurrentSpeed;

} QST_LEG_FAN_MON_UPDATE;

/****************************************************************************/
/* QST_LEG_GET_FAN_MON_CONFIG_RSP - Command response structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_GET_FAN_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   UINT16                       uSpeedNominal;
   UINT16                       uSpeedNonCritical;
   UINT16                       uSpeedCritical;
   UINT16                       uSpeedNonRecoverable;

} QST_LEG_GET_FAN_MON_CONFIG_RSP, *P_QST_LEG_GET_FAN_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_LEG_GET_FAN_MON_UPDATE_RSP - Command response structure definition   */
/****************************************************************************/

typedef struct _QST_LEG_GET_FAN_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_LEG_FAN_MON_UPDATE       stMonitorUpdate[QST_LEG_MAX_FAN_MONITORS];

} QST_LEG_GET_FAN_MON_UPDATE_RSP, *P_QST_LEG_GET_FAN_MON_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_SET_FAN_MON_THRESHOLDS_CMD - Command response structure def'n    */
/****************************************************************************/

typedef struct _QST_LEG_SET_FAN_MON_THRESHOLDS_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   UINT16                       uSpeedNonCritical;
   UINT16                       uSpeedCritical;
   UINT16                       uSpeedNonRecoverable;

} QST_LEG_SET_FAN_MON_THRESHOLDS_CMD, *P_QST_LEG_SET_FAN_MON_THRESHOLDS_CMD;

/****************************************************************************/
/* QST_LEG_VOLT_MON_UPDATE - Voltage Monitor Update structure definition    */
/****************************************************************************/

typedef struct _QST_LEG_VOLT_MON_UPDATE
{
   QST_LEG_MON_HEALTH_STATUS    stMonitorStatus;
   INT32                        uCurrentVoltage;

} QST_LEG_VOLT_MON_UPDATE;

// Useful Macros

#define QST_VOLT_TO_FLOAT(x)                    ((float)(x) / 1000)
#define QST_VOLT_TO_DOUBLE(x)                   ((double)(x) / 1000)

/****************************************************************************/
/* QST_LEG_GET_VOLT_MON_UPDATE_RSP - Command response structure definition  */
/****************************************************************************/

typedef struct _QST_LEG_GET_VOLT_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_LEG_VOLT_MON_UPDATE      stMonitorUpdate[QST_LEG_MAX_VOLT_MONITORS];

} QST_LEG_GET_VOLT_MON_UPDATE_RSP, *P_QST_LEG_GET_VOLT_MON_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_GET_VOLT_MON_CONFIG_RSP - Command response structure definition  */
/****************************************************************************/

typedef struct _QST_LEG_GET_VOLT_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   INT32                        iVoltageNominal;
   INT32                        iUnderVoltageNonCritical;
   INT32                        iOverVoltageNonCritical;
   INT32                        iUnderVoltageCritical;
   INT32                        iOverVoltageCritical;
   INT32                        iUnderVoltageNonRecoverable;
   INT32                        iOverVoltageNonRecoverable;

} QST_LEG_GET_VOLT_MON_CONFIG_RSP, *P_QST_LEG_GET_VOLT_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_LEG_SET_VOLT_MON_THRESHOLDS_CMD - Command structure definition       */
/****************************************************************************/

typedef struct _QST_LEG_SET_VOLT_MON_THRESHOLDS_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   INT32                        iUnderVoltageNonCritical;
   INT32                        iOverVoltageNonCritical;
   INT32                        iUnderVoltageCritical;
   INT32                        iOverVoltageCritical;
   INT32                        iUnderVoltageNonRecoverable;
   INT32                        iOverVoltageNonRecoverable;

} QST_LEG_SET_VOLT_MON_THRESHOLDS_CMD, *P_QST_LEG_SET_VOLT_MON_THRESHOLDS_CMD;

// Useful Macros

#define QST_VOLT_FROM_FLOAT(x)                  ((INT32)((x) * 1000))
#define QST_VOLT_FROM_DOUBLE(x)                 ((INT32)((x) * 1000))

/****************************************************************************/
/* QST_LEG_FAN_CTRL_STATUS - Fan Controller Status structure definition     */
/****************************************************************************/

typedef struct _QST_LEG_FAN_CTRL_STATUS
{
   BIT_FIELD_IN_UINT8           bControllerEnabled: 1;
   BIT_FIELD_IN_UINT8           uControllerStatus: 2;
   BIT_FIELD_IN_UINT8           bOverrideSoftware: 1;
   BIT_FIELD_IN_UINT8           bOverrideFanController: 1;
   BIT_FIELD_IN_UINT8           bOverrideTemperatureSensor: 1;
   BIT_FIELD_IN_UINT8           bOverrideFanStall: 1;
   BIT_FIELD_IN_UINT8           bOverrideRedetection: 1;

} QST_LEG_FAN_CTRL_STATUS;

/****************************************************************************/
/* QST_LEG_FAN_CTRL_UPDATE - Fan Controller Update structure definition     */
/****************************************************************************/

typedef struct _QST_LEG_FAN_CTRL_UPDATE
{
   QST_LEG_FAN_CTRL_STATUS      stControllerStatus;
   UINT16                       uCurrentDutyCycle;

} QST_LEG_FAN_CTRL_UPDATE;

// Useful Macros

#define QST_DUTY_TO_FLOAT(x)                    ((float)(x) / 100)
#define QST_DUTY_TO_DOUBLE(x)                   ((double)(x) / 100)

/****************************************************************************/
/* QST_LEG_GET_FAN_CTRL_UPDATE_RSP - Command response structure definition  */
/****************************************************************************/

typedef struct _QST_LEG_GET_FAN_CTRL_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_LEG_FAN_CTRL_UPDATE      stControllerUpdate[QST_LEG_MAX_FAN_CONTROLLERS];

} QST_LEG_GET_FAN_CTRL_UPDATE_RSP, *P_QST_LEG_GET_FAN_CTRL_UPDATE_RSP;

/****************************************************************************/
/* QST_LEG_GET_FAN_CTRL_CONFIG_RSP - Command response structure definition  */
/****************************************************************************/

typedef struct _QST_LEG_GET_FAN_CTRL_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bControllerEnabled;
   UINT8                        byControllerUsage;

} QST_LEG_GET_FAN_CTRL_CONFIG_RSP, *P_QST_LEG_GET_FAN_CTRL_CONFIG_RSP;

/****************************************************************************/
/* QST_LEG_SET_FAN_CTRL_DUTY_CMD - Command structure definition             */
/****************************************************************************/

typedef struct _QST_LEG_SET_FAN_CTRL_DUTY_CMD
{
   QST_LEG_CMD_HEADER           stHeader;
   UINT16                       uDutyCycle;

} QST_LEG_SET_FAN_CTRL_DUTY_CMD, *P_QST_LEG_SET_FAN_CTRL_DUTY_CMD;

// Useful Macros

#define QST_DUTY_FROM_FLOAT(x)                  ((UINT16)((x) * 100))
#define QST_DUTY_FROM_DOUBLE(x)                 ((UINT16)((x) * 100))

/****************************************************************************/
/****************************************************************************/
/******************* Definitions for SST Command Support ********************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Please see QstCmd.h for common SST commands and structures.              */
/****************************************************************************/

/****************************************************************************/
/* SST Legacy Chipset Commands                                              */
/****************************************************************************/

#define SST_LEG_CS_GET_ICH_TEMP_READING         0x00
#define SST_LEG_CS_GET_MCH_TEMP_READING         0x01
#define SST_LEG_CS_GET_CPU_1_TEMP_READING       0x02
#define SST_LEG_CS_GET_CPU_2_TEMP_READING       0x03
#define SST_LEG_CS_GET_CPU_3_TEMP_READING       0x04
#define SST_LEG_CS_GET_CPU_4_TEMP_READING       0x05

#define SST_LEG_CS_GET_FAN_SENS_1_ATTRIBS       0x06
#define SST_LEG_CS_GET_FAN_SENS_2_ATTRIBS       0x07
#define SST_LEG_CS_GET_FAN_SENS_3_ATTRIBS       0x08
#define SST_LEG_CS_GET_FAN_SENS_4_ATTRIBS       0x09
#define SST_LEG_CS_GET_FAN_SENS_5_ATTRIBS       0x0A     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_6_ATTRIBS       0x0B     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_7_ATTRIBS       0x0C     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_8_ATTRIBS       0x0D     // currently not supported

#define SST_LEG_CS_SET_FAN_SENS_1_CONFIG        0x0E
#define SST_LEG_CS_SET_FAN_SENS_2_CONFIG        0x0F
#define SST_LEG_CS_SET_FAN_SENS_3_CONFIG        0x10
#define SST_LEG_CS_SET_FAN_SENS_4_CONFIG        0x11
#define SST_LEG_CS_SET_FAN_SENS_5_CONFIG        0x12     // currently not supported
#define SST_LEG_CS_SET_FAN_SENS_6_CONFIG        0x13     // currently not supported
#define SST_LEG_CS_SET_FAN_SENS_7_CONFIG        0x14     // currently not supported
#define SST_LEG_CS_SET_FAN_SENS_8_CONFIG        0x15     // currently not supported

#define SST_LEG_CS_GET_FAN_SENS_1_SPEED         0x16
#define SST_LEG_CS_GET_FAN_SENS_2_SPEED         0x17
#define SST_LEG_CS_GET_FAN_SENS_3_SPEED         0x18
#define SST_LEG_CS_GET_FAN_SENS_4_SPEED         0x19
#define SST_LEG_CS_GET_FAN_SENS_5_SPEED         0x1A     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_6_SPEED         0x1B     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_7_SPEED         0x1C     // currently not supported
#define SST_LEG_CS_GET_FAN_SENS_8_SPEED         0x1D     // currently not supported

#define SST_LEG_CS_GET_FAN_CTRL_1_ATTRIBS       0x1E
#define SST_LEG_CS_GET_FAN_CTRL_2_ATTRIBS       0x1F
#define SST_LEG_CS_GET_FAN_CTRL_3_ATTRIBS       0x20
#define SST_LEG_CS_GET_FAN_CTRL_4_ATTRIBS       0x21

#define SST_LEG_CS_SET_FAN_CTRL_1_CONFIG        0x22
#define SST_LEG_CS_SET_FAN_CTRL_2_CONFIG        0x23
#define SST_LEG_CS_SET_FAN_CTRL_3_CONFIG        0x24
#define SST_LEG_CS_SET_FAN_CTRL_4_CONFIG        0x25

#define SST_LEG_CS_SET_FAN_CTRL_1_SPEED         0x26
#define SST_LEG_CS_SET_FAN_CTRL_2_SPEED         0x27
#define SST_LEG_CS_SET_FAN_CTRL_3_SPEED         0x28
#define SST_LEG_CS_SET_FAN_CTRL_4_SPEED         0x29

#define SST_LEG_CS_GET_FAN_CTRL_1_SPEED         0x2A
#define SST_LEG_CS_GET_FAN_CTRL_2_SPEED         0x2B
#define SST_LEG_CS_GET_FAN_CTRL_3_SPEED         0x2C
#define SST_LEG_CS_GET_FAN_CTRL_4_SPEED         0x2D

/****************************************************************************/
/* QST_LEG_SST_PASS_THROUGH_CMD - Command structure definition (with SST    */
/* Packet embedded).                                                        */
/****************************************************************************/

typedef struct _QST_LEG_SST_PASS_THROUGH_CMD
{
    QST_LEG_CMD_HEADER          stHeader;
    SST_CMD                     stSSTPacket;

} QST_LEG_SST_PASS_THROUGH_CMD, *P_QST_LEG_SST_PASS_THROUGH_CMD;

// Useful Macros

#define QST_LEG_SST_CMD_DATA(NumDataWords)  (4 + (2 * (NumDataWords)))
#define QST_LEG_SST_CMD_SIZE(NumDataWords)  (sizeof(QST_LEG_CMD_HEADER) + QST_LEG_SST_CMD_DATA(NumDataWords))

/****************************************************************************/
/* QST_LEG_SST_PASS_THROUGH_RSP - Response Structure Definition             */
/****************************************************************************/

typedef struct _QST_LEG_SST_PASS_THROUGH_RSP
{
   UINT8                        byStatus;         // QST Status
   UINT16                       wValue[1];

} QST_LEG_SST_PASS_THROUGH_RSP, *P_QST_LEG_SST_PASS_THROUGH_RSP;

// Useful Macros

#define QST_SST_RSP_SIZE(NumDataWords)      (1 + (2 * (NumDataWords)))

#pragma pack()

#endif // ndef _QSTCMDLEG_H
