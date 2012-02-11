/****************************************************************************/
/*                                                                          */
/*  Module:         QstCmd.c                                                */
/*                                                                          */
/*  Description:    Provides  definitions for the 2.0 command packets that  */
/*                  are used to  query  and  control  the  Intel(R)  Quiet  */
/*                  System  Technology  (QST)  subsystem  and  the various  */
/*                  sensors and controllers that are managed by it.         */
/*                                                                          */
/*  Notes:      1.  The current F/W release supports only the 2.0  command  */
/*                  packets that are defined in this file; the 1.x command  */
/*                  packets  are  no  longer  accepted. For some period of  */
/*                  time,   the  Communications  Library  will  include  a  */
/*                  compatibility layer that allows legacy software to use  */
/*                  1.x commands (all except QST_SET_SUBSYSTEM_CONFIG), by  */
/*                  translating them into the equivalent 2.0 commands.      */
/*                                                                          */
/*              3.  While they  are  defined  within  this  file,  certain  */
/*                  commands  may/should only be sent to the QST Subsystem  */
/*                  by the BIOS during POST.                                */
/*                                                                          */
/*              4.  Commands that modify  the  configuration  of  the  QST  */
/*                  Subsystem may be rejected, depending upon the Security  */
/*                  Mask specified by the BIOS during POST. Similarly, SST  */
/*                  (Pass-Through)  commands that request the modification  */
/*                  of the operation of the sensors and controllers within  */
/*                  Chipset and within SST devices may also be rejected.    */
/*                                                                          */
/*              5.  Command and response  packets  that  include  no  data  */
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

#ifndef _QSTCMD_H
#define _QSTCMD_H

#include "QstCfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/****************************************************************************/
/************** Definitions for QST Subsystem Command Support ***************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* QST Subsystem Command Codes                                              */
/****************************************************************************/

#define QST_GET_SUBSYSTEM_INFO              0x00
#define QST_GET_SUBSYSTEM_STATUS            0x01
#define QST_GET_SUBSYSTEM_CONFIG            0x02
#define QST_GET_SUBSYSTEM_CONFIG_PROFILE    0x03
#define QST_SET_SUBSYSTEM_CONFIG            0x04
#define QST_LOCK_SUBSYSTEM                  0x05
#define QST_UPDATE_CPU_CONFIG               0x06
#define QST_GET_CPU_CONFIG_UPDATE           0x07
#define QST_UPDATE_CPU_DTS_CONFIG           0x08
#define QST_GET_CPU_DTS_CONFIG_UPDATE       0x09
#define QST_UPDATE_FAN_CONFIG               0x0A
#define QST_GET_FAN_CONFIG_UPDATE           0x0B
#define QST_SST_PASS_THROUGH                0x0C

#define QST_GET_TEMP_MON_UPDATE             0x0D
#define QST_GET_TEMP_MON_CONFIG             0x0E
#define QST_SET_TEMP_MON_THRESHOLDS         0x0F
#define QST_SET_TEMP_MON_READING            0x10
#define QST_NO_TEMP_MON_READINGS            0x11

#define QST_GET_FAN_MON_UPDATE              0x12
#define QST_GET_FAN_MON_CONFIG              0x13
#define QST_SET_FAN_MON_THRESHOLDS          0x14
#define QST_ENABLE_FAN_MON                  0x15
#define QST_DISABLE_FAN_MON                 0x16
#define QST_REDETECT_FAN_PRESENCE           0x17

#define QST_GET_VOLT_MON_UPDATE             0x18
#define QST_GET_VOLT_MON_CONFIG             0x19
#define QST_SET_VOLT_MON_THRESHOLDS         0x1A

#define QST_GET_CURR_MON_UPDATE             0x1B
#define QST_GET_CURR_MON_CONFIG             0x1C
#define QST_SET_CURR_MON_THRESHOLDS         0x1D

#define QST_GET_FAN_CTRL_UPDATE             0x1E
#define QST_GET_FAN_CTRL_CONFIG             0x1F
#define QST_SET_FAN_CTRL_DUTY               0x20
#define QST_SET_FAN_CTRL_AUTO               0x21
#define QST_RESET_FAN_CTRL_MIN_DUTY         0x22

#define QST_LAST_CMD_CODE                   0x22

/****************************************************************************/
/* Response Codes                                                           */
/****************************************************************************/

#define QST_CMD_SUCCESSFUL                  0x00
#define QST_CMD_REJECTED_UNSUPPORTED        0x01
#define QST_CMD_REJECTED_LOCKED             0x02
#define QST_CMD_REJECTED_PARAMETER          0x03
#define QST_CMD_REJECTED_VERSION            0x04
#define QST_CMD_FAILED_COMM_ERROR           0x05
#define QST_CMD_FAILED_SENSOR_ERROR         0x06
#define QST_CMD_FAILED_NO_MEMORY            0x07
#define QST_CMD_FAILED_NO_RESOURCES         0x08
#define QST_CMD_REJECTED_INVALID            0x09
#define QST_CMD_REJECTED_CMD_SIZE           0x0A
#define QST_CMD_REJECTED_RSP_SIZE           0x0B
#define QST_CMD_REJECTED_CONTEXT            0x0C

/****************************************************************************/
/* QST_CMD_HEADER - Command header structure definition                     */
/****************************************************************************/

typedef struct _QST_CMD_HEADER
{
   UINT8                        byCommand;
   UINT8                        byEntity;
   UINT16                       wCommandLength;
   UINT16                       wResponseLength;

} QST_CMD_HEADER, *P_QST_CMD_HEADER;

/****************************************************************************/
/* QST_GENERIC_CMD - Generic command structure definition                   */
/****************************************************************************/

typedef struct _QST_GENERIC_CMD
{
   QST_CMD_HEADER               stHeader;

} QST_GENERIC_CMD, *P_QST_GENERIC_CMD;

// Useful Macros

#define QST_CMD_DATA_SIZE(x)    (sizeof(x) - sizeof(QST_CMD_HEADER))

/****************************************************************************/
/* QST_GENERIC_RSP - Generic response structure definition                  */
/****************************************************************************/

typedef struct _QST_GENERIC_RSP
{
   UINT8                        byStatus;

} QST_GENERIC_RSP, *P_QST_GENERIC_RSP;

/****************************************************************************/
/* QST_GET_SUBSYSTEM_INFO_RSP - Command response structure definition.      */
/****************************************************************************/

typedef struct _QST_GET_SUBSYSTEM_INFO_RSP
{
   UINT8                        byStatus;
   UINT8                        uMajorVersionNumber;
   UINT8                        uMinorVersionNumber;
   UINT8                        uRevisionNumber;
   UINT16                       uBuildNumber;
   UINT8                        uSuppTempMonitors;
   UINT8                        uSuppFanMonitors;
   UINT8                        uSuppVoltMonitors;
   UINT8                        uSuppCurrMonitors;
   UINT8                        uSuppTempResponses;
   UINT8                        uSuppFanControllers;
   UINT16                       uMaxCmdSize;

} QST_GET_SUBSYSTEM_INFO_RSP, *P_QST_GET_SUBSYSTEM_INFO_RSP;

/****************************************************************************/
/* QST_SUBSYSTEM_STATUS - Subsystem Status byte structure definition        */
/****************************************************************************/

typedef struct _QST_SUBSYSTEM_STATUS
{
   BIT_FIELD_IN_UINT8           bSubsystemConfigured: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullError: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullCritical: 1;
   BIT_FIELD_IN_UINT8           bOverrideFullFailure: 1;
   BIT_FIELD_IN_UINT8           uReserved: 4;

} QST_SUBSYSTEM_STATUS, *P_QST_SUBSYSTEM_STATUS;

/****************************************************************************/
/* QST_LOCK_MASK - Lock Mask structure definition                           */
/****************************************************************************/

typedef struct _QST_LOCK_MASK
{
   BIT_FIELD_IN_UINT16          bLockConfiguration: 1;
   BIT_FIELD_IN_UINT16          bLockSSTBusAccess: 1;
   BIT_FIELD_IN_UINT16          bLockThresholds: 1;
   BIT_FIELD_IN_UINT16          bLockManualFanControl: 1;
   BIT_FIELD_IN_UINT16          bLockChipsetAccess: 1;
   BIT_FIELD_IN_UINT16          uReserved: 11;

} QST_LOCK_MASK;

/****************************************************************************/
/* QST_CONFIG_STATUS - Configuration Status byte structure definition       */
/****************************************************************************/

typedef struct _QST_CONFIG_STATUS
{
   BIT_FIELD_IN_UINT32          bConfigSuccessful: 1;
   BIT_FIELD_IN_UINT32          uFailingEntityType: 4;
   BIT_FIELD_IN_UINT32          uReserved1: 3;
   BIT_FIELD_IN_UINT32          uFailingEntityIndex: 8;
   BIT_FIELD_IN_UINT32          uFailingEntityParam: 8;
   BIT_FIELD_IN_UINT32          uReserved2: 8;

} QST_CONFIG_STATUS;

/****************************************************************************/
/* QST_GET_SUBSYSTEM_STATUS_RSP - Command response structure definition.    */
/****************************************************************************/

typedef struct _QST_GET_SUBSYSTEM_STATUS_RSP
{
   UINT8                        byStatus;
   QST_SUBSYSTEM_STATUS         stSubsystemStatus;
   QST_LOCK_MASK                stLockMask;
   QST_CONFIG_STATUS            stConfigStatus;

} QST_GET_SUBSYSTEM_STATUS_RSP, *P_QST_GET_SUBSYSTEM_STATUS_RSP;

/****************************************************************************/
/* QST_GET_SUBSYSTEM_CONFIG_RSP - Command response structure definition.    */
/* Note: QST_PAYLOAD_STRUCT is defined in Header file QstCfg.h.             */
/****************************************************************************/

typedef struct _QST_GET_SUBSYSTEM_CONFIG_RSP
{
   UINT8                        byStatus;
   QST_ABS_PAYLOAD_STRUCT       stConfigPayload;

} QST_GET_SUBSYSTEM_CONFIG_RSP, *P_QST_GET_SUBSYSTEM_CONFIG_RSP;

/****************************************************************************/
/* QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP - Command response structure        */
/* definition                                                               */
/****************************************************************************/

typedef struct _QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP
{
   UINT8                        byStatus;
   UINT32                       dwTempMonsConfigured;
   UINT32                       dwFanMonsConfigured;
   UINT32                       dwVoltMonsConfigured;
   UINT32                       dwCurrMonsConfigured;
   UINT32                       dwTempRespConfigured;
   UINT32                       dwFanCtrlsConfigured;

} QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP, *P_QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP;

// Useful Macros

#ifndef SET_BIT
#define SET_BIT(var,bit)        ((var) |= 1 << (bit))
#define BIT_SET(var,bit)        (((var) & (1 << (bit))) != 0)
#define CLR_BIT(var,bit)        ((var) &= (~(1 << (bit))))
#endif

/****************************************************************************/
/* QST_SET_SUBSYSTEM_CONFIG_CMD - Command structure definition.             */
/* Note: QST_PAYLOAD_STRUCT is defined in Header file QstCfg.h.             */
/****************************************************************************/

typedef struct _QST_SET_SUBSYSTEM_CONFIG_CMD
{
   QST_CMD_HEADER               stHeader;
   QST_ABS_PAYLOAD_STRUCT       stConfigPayload;

} QST_SET_SUBSYSTEM_CONFIG_CMD, *P_QST_SET_SUBSYSTEM_CONFIG_CMD;

/****************************************************************************/
/* QST_LOCK_SUBSYSTEM_CMD - Command structure definition                    */
/****************************************************************************/

typedef struct _QST_LOCK_SUBSYSTEM_CMD
{
   QST_CMD_HEADER               stHeader;
   QST_LOCK_MASK                stLockMask;

} QST_LOCK_SUBSYSTEM_CMD, *P_QST_LOCK_SUBSYSTEM_CMD;

/****************************************************************************/
/* QST_UPDATE_CPU_CONFIG_CMD - Command structure definition                 */
/****************************************************************************/

typedef struct _QST_UPDATE_CPU_CONFIG_CMD
{
   QST_CMD_HEADER               stHeader;
   INT8                         bRelativeTemp;
   INT8                         bMchTempSupported;
   INT32F                       lfTcontrolTemp;
   INT32F                       lfCorrectionOffset;
   INT32F                       lfCorrectionSlope;
   INT32F                       lfProportionalGain;
   INT32F                       lfIntegralGain;
   INT32F                       lfDerivativeGain;
   UINT8                        uIntegralTimeWindow;
   UINT8                        uDerivativeTimeWindow;

} QST_UPDATE_CPU_CONFIG_CMD, *P_QST_UPDATE_CPU_CONFIG_CMD;

/****************************************************************************/
/* QST_GET_CPU_CONFIG_UPDATE_RSP - Command response structure definition    */
/****************************************************************************/

typedef struct _QST_GET_CPU_CONFIG_UPDATE_RSP
{
   UINT8                        byStatus;
   INT8                         bUpdateAvailable;
   INT8                         bRelativeTemp;
   INT8                         bMchTempSupported;
   INT32F                       lfTcontrolTemp;
   INT32F                       lfCorrectionOffset;
   INT32F                       lfCorrectionSlope;
   INT32F                       lfProportionalGain;
   INT32F                       lfIntegralGain;
   INT32F                       lfDerivativeGain;
   UINT8                        uIntegralTimeWindow;
   UINT8                        uDerivativeTimeWindow;

} QST_GET_CPU_CONFIG_UPDATE_RSP, *P_QST_GET_CPU_CONFIG_UPDATE_RSP;

/****************************************************************************/
/* QST_PSICA_INTERSECT_POINT - Structure definition for coefficient pairs   */
/* on PSI-CA RPM CURVE.                                                     */
/****************************************************************************/

#define QST_INTERSECT_POINTS    6

typedef struct _QST_PSICA_INTERSECT_POINT
{
   INT32LF                      lfPSICAPoint;
   UINT16                       wRPMValue;

} QST_PSICA_INTERSECT_POINT;

/****************************************************************************/
/* QST_UPDATE_CPU_DTS_CONFIG_CMD - Command Structure Definition             */
/****************************************************************************/

typedef struct _QST_UPDATE_CPU_DTS_CONFIG_CMD
{
   QST_CMD_HEADER               stHeader;
   INT32LF                      lfPSICAAtTcontrolOffset;
   INT32LF                      lfPSICAAtTcontrolSlope;
   INT32LF                      lfPSICAAtNeg1Offset;
   INT32LF                      lfPSICAAtNeg1Slope;
   QST_PSICA_INTERSECT_POINT    stRPMAtPSICAIntersect[QST_INTERSECT_POINTS];
   INT32F                       lfMaxTambient;
   UINT8                        uAmbientTempMon;
   UINT8                        uAssocFanMon;
   INT32F                       lfProportionalGain;
   INT32F                       lfIntegralGain;
   INT32F                       lfDerivativeGain;
   UINT8                        uIntegralTimeWindow;
   UINT8                        uDerivativeTimeWindow;

} QST_UPDATE_CPU_DTS_CONFIG_CMD, *P_QST_UPDATE_CPU_DTS_CONFIG_CMD;

/****************************************************************************/
/* QST_GET_CPU_DTS_UPDATE_RSP - Command response structure definition       */
/****************************************************************************/

typedef struct _QST_GET_CPU_DTS_UPDATE_RSP
{
    UINT8                        byStatus;
    INT8                         bUpdateAvailable;
    INT32LF                      lfPSICAAtTcontrolOffset;
    INT32LF                      lfPSICAAtTcontrolSlope;
    INT32LF                      lfPSICAAtNeg1Offset;
    INT32LF                      lfPSICAAtNeg1Slope;
    QST_PSICA_INTERSECT_POINT    stRPMAtPSICAIntersect[QST_INTERSECT_POINTS];
    INT32F                       lfMaxTambient;
    UINT8                        uAmbientTempMon;
    UINT8                        uAssocFanMon;
    INT32F                       lfProportionalGain;
    INT32F                       lfIntegralGain;
    INT32F                       lfDerivativeGain;
    UINT8                        uIntegralTimeWindow;
    UINT8                        uDerivativeTimeWindow;

} QST_GET_CPU_DTS_UPDATE_RSP, *P_QST_GET_CPU_DTS_UPDATE_RSP;

// useful Macros

#define QST_INT32LF_TO_FLOAT(x)    ((float)(x) / 10000)
#define QST_INT32LF_TO_DOUBLE(x)   ((double)(x) / 10000)

#define QST_INT32LF_FROM_FLOAT(x)  ((INT32LF)((x) * 10000))
#define QST_INT32LF_FROM_DOUBLE(x) ((INT32LF)((x) * 10000))

/****************************************************************************/
/* QST_FAN_CTRL_CONFIG - Fan Controller Configuration Word structure        */
/* definition. Note: Literals for Signal Frequency and Spin-Up Time are     */
/* defined in QstCfg.h                                                      */
/****************************************************************************/

typedef struct _QST_FAN_CTRL_CONFIG
{
   BIT_FIELD_IN_UINT16          uPWMSignalFrequency: 3;
   BIT_FIELD_IN_UINT16          uSpinUpTime: 3;
   BIT_FIELD_IN_UINT16          bSignalInvert: 1;
   BIT_FIELD_IN_UINT16          uReserved: 9;

} QST_FAN_CTRL_CONFIG;

/****************************************************************************/
/* QST_FAN_SENSOR_CONFIG - Fan Sensor Configuration Word structure          */
/* definition. Definitions for uPulsesPerRevolution are in QstCfg.h         */
/****************************************************************************/

typedef struct _QST_FAN_SENSOR_CONFIG
{
   BIT_FIELD_IN_UINT16          uPulsesPerRevolution: 3;
   BIT_FIELD_IN_UINT16          bDependentMeasurement: 1;
   BIT_FIELD_IN_UINT16          uReserved: 11;
   BIT_FIELD_IN_UINT16          bSensorPresent: 1;

} QST_FAN_SENSOR_CONFIG;

/****************************************************************************/
/* QST_FAN_SENSOR_ASSOC - Associated Fan Sensor configuration field         */
/* structure definition.                                                    */
/****************************************************************************/

typedef struct _QST_FAN_SENSOR_ASSOC
{
   QST_FAN_SENSOR_CONFIG        stFanSensorConfig;
   UINT16                       wFanMinimumRPMRangeLow;
   UINT16                       wFanMinimumRPMRangeHigh;

} QST_FAN_SENSOR_ASSOC;

/****************************************************************************/
/* QST_UPDATE_FAN_CONFIG_CMD - Command structure definition                 */
/****************************************************************************/

typedef struct _QST_UPDATE_FAN_CONFIG_CMD
{
   QST_CMD_HEADER               stHeader;
   QST_FAN_CTRL_CONFIG          stFanCtrlConfig;
   QST_FAN_SENSOR_ASSOC         stFanSensorAssoc[QST_MAX_ASSOC_FAN_SENSORS];
   UINT8                        byMode;
   UINT8                        byDutyCycleMin;
   UINT8                        byDutyCycleOn;
   UINT8                        byDutyCycleMax;

} QST_UPDATE_FAN_CONFIG_CMD, *P_QST_UPDATE_FAN_CONFIG_CMD;

// MIN/OFF Mode (byMode field) Values

#define QST_MIN_MODE            0
#define QST_OFF_MODE            1

/****************************************************************************/
/* QST_GET_FAN_CONFIG_UPDATE_RSP - Command response structure definition    */
/****************************************************************************/

typedef struct _QST_GET_FAN_CONFIG_UPDATE_RSP
{
   UINT8                        byStatus;
   INT8                         bUpdateAvailable;
   QST_FAN_CTRL_CONFIG          stFanCtrlConfig;
   QST_FAN_SENSOR_ASSOC         stFanSensorAssoc[QST_MAX_ASSOC_FAN_SENSORS];
   UINT8                        byMode;
   UINT8                        byDutyCycleMin;
   UINT8                        byDutyCycleOn;
   UINT8                        byDutyCycleMax;

} QST_GET_FAN_CONFIG_UPDATE_RSP, *P_QST_GET_FAN_CONFIG_UPDATE_RSP;

/****************************************************************************/
/* QST_MON_HEALTH_STATUS - Monitor Health Status structure definition       */
/****************************************************************************/

typedef struct _QST_MON_HEALTH_STATUS
{
   BIT_FIELD_IN_UINT8           bMonitorEnabled: 1;
   BIT_FIELD_IN_UINT8           uMonitorStatus: 2;
   BIT_FIELD_IN_UINT8           uThresholdStatus: 2;
   BIT_FIELD_IN_UINT8           bSensorReadingError: 1;
   BIT_FIELD_IN_UINT8           bSensorCommError: 1;
   BIT_FIELD_IN_UINT8           uReserved: 1;

} QST_MON_HEALTH_STATUS;

// Health Status (uMonitorStatus field) Values

#define QST_STATUS_NORMAL          0
#define QST_STATUS_NON_CRITICAL    1
#define QST_STATUS_CRITICAL        2
#define QST_STATUS_NON_RECOVERABLE 3

/****************************************************************************/
/* QST_TEMP_MON_UPDATE - Temperature Monitor Update structure definition    */
/****************************************************************************/

typedef struct _QST_TEMP_MON_UPDATE
{
   QST_MON_HEALTH_STATUS        stMonitorStatus;
   INT32F                       lfCurrentReading;

} QST_TEMP_MON_UPDATE;

// useful Macros

#define QST_TEMP_TO_FLOAT(x)    ((float)(x) / 100)
#define QST_TEMP_TO_DOUBLE(x)   ((double)(x) / 100)

/****************************************************************************/
/* QST_GET_TEMP_MON_UPDATE_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_TEMP_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_TEMP_MON_UPDATE          stMonitorUpdate[QST_ABS_TEMP_MONITORS];

} QST_GET_TEMP_MON_UPDATE_RSP, *P_QST_GET_TEMP_MON_UPDATE_RSP;

#define QST_TEMP_MON_UPDATE_RSP_SIZE(Sensors)      ((sizeof(QST_TEMP_MON_UPDATE) * (Sensors)) + 1)
#define QST_TEMP_MON_UPDATE_RSP_SIZE_BAD(RspSize)  ((((RspSize) - 1) % sizeof(QST_TEMP_MON_UPDATE)) != 0)
#define QST_TEMP_MON_UPDATE_SENSOR_COUNT(RspSize)  (((RspSize) - 1) / sizeof(QST_TEMP_MON_UPDATE))

/****************************************************************************/
/* QST_GET_TEMP_MON_CONFIG_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_TEMP_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   INT8                         bRelativeReadings;
   INT32F                       lfTempNominal;
   INT32F                       lfTempNonCritical;
   INT32F                       lfTempCritical;
   INT32F                       lfTempNonRecoverable;

} QST_GET_TEMP_MON_CONFIG_RSP, *P_QST_GET_TEMP_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_SET_TEMP_MON_THRESHOLDS_CMD - Command structure definition           */
/****************************************************************************/

typedef struct _QST_SET_TEMP_MON_THRESHOLDS_CMD
{
   QST_CMD_HEADER               stHeader;
   INT32F                       lfTempNonCritical;
   INT32F                       lfTempCritical;
   INT32F                       lfTempNonRecoverable;

} QST_SET_TEMP_MON_THRESHOLDS_CMD, *P_QST_SET_TEMP_MON_THRESHOLDS_CMD;

// Useful Macros

#define QST_TEMP_FROM_FLOAT(x)  ((INT32F)((x) * 100))
#define QST_TEMP_FROM_DOUBLE(x) ((INT32F)((x) * 100))

/****************************************************************************/
/* QST_SET_TEMP_MON_READING_CMD - Command structure definition              */
/****************************************************************************/

typedef struct _QST_SET_TEMP_MON_READING_CMD
{
   QST_CMD_HEADER               stHeader;
   INT32F                       lfTempReading;

} QST_SET_TEMP_MON_READING_CMD, *P_QST_SET_TEMP_MON_READING_CMD;

/****************************************************************************/
/* QST_FAN_MON_UPDATE - Fan Speed Monitor Update structure definition       */
/****************************************************************************/

typedef struct _QST_FAN_MON_UPDATE
{
   QST_MON_HEALTH_STATUS        stMonitorStatus;
   UINT16                       uCurrentSpeed;

} QST_FAN_MON_UPDATE;

/****************************************************************************/
/* QST_GET_FAN_MON_CONFIG_RSP - Command response structure definition       */
/****************************************************************************/

typedef struct _QST_GET_FAN_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   UINT16                       uSpeedNominal;
   UINT16                       uSpeedNonCritical;
   UINT16                       uSpeedCritical;
   UINT16                       uSpeedNonRecoverable;

} QST_GET_FAN_MON_CONFIG_RSP, *P_QST_GET_FAN_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_GET_FAN_MON_UPDATE_RSP - Command response structure definition       */
/****************************************************************************/

typedef struct _QST_GET_FAN_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_FAN_MON_UPDATE           stMonitorUpdate[QST_ABS_FAN_MONITORS];

} QST_GET_FAN_MON_UPDATE_RSP, *P_QST_GET_FAN_MON_UPDATE_RSP;

#define QST_FAN_MON_UPDATE_RSP_SIZE(Sensors)       ((sizeof(QST_FAN_MON_UPDATE) * (Sensors)) + 1)
#define QST_FAN_MON_UPDATE_RSP_SIZE_BAD(RspSize)   ((((RspSize) - 1) % sizeof(QST_FAN_MON_UPDATE)) != 0)
#define QST_FAN_MON_UPDATE_SENSOR_COUNT(RspSize)   (((RspSize) - 1) / sizeof(QST_FAN_MON_UPDATE))

/****************************************************************************/
/* QST_SET_FAN_MON_THRESHOLDS_CMD - Command response structure definition   */
/****************************************************************************/

typedef struct _QST_SET_FAN_MON_THRESHOLDS_CMD
{
   QST_CMD_HEADER               stHeader;
   UINT16                       uSpeedNonCritical;
   UINT16                       uSpeedCritical;
   UINT16                       uSpeedNonRecoverable;

} QST_SET_FAN_MON_THRESHOLDS_CMD, *P_QST_SET_FAN_MON_THRESHOLDS_CMD;

/****************************************************************************/
/* QST_VOLT_MON_UPDATE - Voltage Monitor Update structure definition        */
/****************************************************************************/

typedef struct _QST_VOLT_MON_UPDATE
{
   QST_MON_HEALTH_STATUS        stMonitorStatus;
   INT32                        iCurrentVoltage;

} QST_VOLT_MON_UPDATE;

// Useful Macros

#define QST_VOLT_TO_FLOAT(x)    ((float)(x) / 1000)
#define QST_VOLT_TO_DOUBLE(x)   ((double)(x) / 1000)

/****************************************************************************/
/* QST_GET_VOLT_MON_UPDATE_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_VOLT_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_VOLT_MON_UPDATE          stMonitorUpdate[QST_ABS_VOLT_MONITORS];

} QST_GET_VOLT_MON_UPDATE_RSP, *P_QST_GET_VOLT_MON_UPDATE_RSP;

#define QST_VOLT_MON_UPDATE_RSP_SIZE(Sensors)      ((sizeof(QST_VOLT_MON_UPDATE) * (Sensors) ) + 1)
#define QST_VOLT_MON_UPDATE_RSP_SIZE_BAD(RspSize)  ((((RspSize) - 1) % sizeof(QST_VOLT_MON_UPDATE)) != 0)
#define QST_VOLT_MON_UPDATE_SENSOR_COUNT(RspSize)  (((RspSize) - 1) / sizeof(QST_VOLT_MON_UPDATE))

/****************************************************************************/
/* QST_GET_VOLT_MON_CONFIG_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_VOLT_MON_CONFIG_RSP
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

} QST_GET_VOLT_MON_CONFIG_RSP, *P_QST_GET_VOLT_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_SET_VOLT_MON_THRESHOLDS_CMD - Command structure definition           */
/****************************************************************************/

typedef struct _QST_SET_VOLT_MON_THRESHOLDS_CMD
{
   QST_CMD_HEADER               stHeader;
   INT32                        iUnderVoltageNonCritical;
   INT32                        iOverVoltageNonCritical;
   INT32                        iUnderVoltageCritical;
   INT32                        iOverVoltageCritical;
   INT32                        iUnderVoltageNonRecoverable;
   INT32                        iOverVoltageNonRecoverable;

} QST_SET_VOLT_MON_THRESHOLDS_CMD, *P_QST_SET_VOLT_MON_THRESHOLDS_CMD;

// Useful Macros

#define QST_VOLT_FROM_FLOAT(x)  ((INT32)((x) * 1000))
#define QST_VOLT_FROM_DOUBLE(x) ((INT32)((x) * 1000))

/****************************************************************************/
/* QST_CURR_MON_UPDATE - Current Monitor Update structure definition        */
/****************************************************************************/

typedef struct _QST_CURR_MON_UPDATE
{
   QST_MON_HEALTH_STATUS        stMonitorStatus;
   INT32                        iCurrentCurrent;

} QST_CURR_MON_UPDATE;

// Useful Macros

#define QST_CURR_TO_FLOAT(x)    ((float)(x) / 1000)
#define QST_CURR_TO_DOUBLE(x)   ((double)(x) / 1000)

/****************************************************************************/
/* QST_GET_CURR_MON_UPDATE_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_CURR_MON_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_CURR_MON_UPDATE          stMonitorUpdate[QST_ABS_CURR_MONITORS];

} QST_GET_CURR_MON_UPDATE_RSP, *P_QST_GET_CURR_MON_UPDATE_RSP;

#define QST_CURR_MON_UPDATE_RSP_SIZE(Sensors)      ((sizeof(QST_CURR_MON_UPDATE) * (Sensors)) + 1)
#define QST_CURR_MON_UPDATE_RSP_SIZE_BAD(RspSize)  ((((RspSize) - 1) % sizeof(QST_CURR_MON_UPDATE)) != 0)
#define QST_CURR_MON_UPDATE_SENSOR_COUNT(RspSize)  (((RspSize) - 1) / sizeof(QST_CURR_MON_UPDATE))

/****************************************************************************/
/* QST_GET_CURR_MON_CONFIG_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_CURR_MON_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bMonitorEnabled;
   UINT8                        byMonitorUsage;
   INT32                        iCurrentNominal;
   INT32                        iUnderCurrentNonCritical;
   INT32                        iOverCurrentNonCritical;
   INT32                        iUnderCurrentCritical;
   INT32                        iOverCurrentCritical;
   INT32                        iUnderCurrentNonRecoverable;
   INT32                        iOverCurrentNonRecoverable;

} QST_GET_CURR_MON_CONFIG_RSP, *P_QST_GET_CURR_MON_CONFIG_RSP;

/****************************************************************************/
/* QST_SET_CURR_MON_THRESHOLDS_CMD - Command structure definition           */
/****************************************************************************/

typedef struct _QST_SET_CURR_MON_THRESHOLDS_CMD
{
   QST_CMD_HEADER               stHeader;
   INT32                        iUnderCurrentNonCritical;
   INT32                        iOverCurrentNonCritical;
   INT32                        iUnderCurrentCritical;
   INT32                        iOverCurrentCritical;
   INT32                        iUnderCurrentNonRecoverable;
   INT32                        iOverCurrentNonRecoverable;

} QST_SET_CURR_MON_THRESHOLDS_CMD, *P_QST_SET_CURR_MON_THRESHOLDS_CMD;

// Useful Macros

#define QST_CURR_FROM_FLOAT(x)  ((INT32)((x) * 1000))
#define QST_CURR_FROM_DOUBLE(x) ((INT32)((x) * 1000))

/****************************************************************************/
/* QST_FAN_CTRL_STATUS - Fan Controller Status structure definition         */
/****************************************************************************/

typedef struct _QST_FAN_CTRL_STATUS
{
   BIT_FIELD_IN_UINT8           bControllerEnabled: 1;
   BIT_FIELD_IN_UINT8           uControllerStatus: 2;
   BIT_FIELD_IN_UINT8           bOverrideSoftware: 1;
   BIT_FIELD_IN_UINT8           bOverrideFanController: 1;
   BIT_FIELD_IN_UINT8           bOverrideTemperatureSensor: 1;
   BIT_FIELD_IN_UINT8           bOverrideFanStall: 1;
   BIT_FIELD_IN_UINT8           bOverrideRedetection: 1;

} QST_FAN_CTRL_STATUS;

/****************************************************************************/
/* QST_FAN_CTRL_UPDATE - Fan Controller Update structure definition         */
/****************************************************************************/

typedef struct _QST_FAN_CTRL_UPDATE
{
   QST_FAN_CTRL_STATUS          stControllerStatus;
   UINT16                       uCurrentDutyCycle;

} QST_FAN_CTRL_UPDATE;

// Useful Macros

#define QST_DUTY_TO_FLOAT(x)    ((float)(x) / 100)
#define QST_DUTY_TO_DOUBLE(x)   ((double)(x) / 100)

/****************************************************************************/
/* QST_GET_FAN_CTRL_UPDATE_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_FAN_CTRL_UPDATE_RSP
{
   UINT8                        byStatus;
   QST_FAN_CTRL_UPDATE          stControllerUpdate[QST_ABS_FAN_CONTROLLERS];

} QST_GET_FAN_CTRL_UPDATE_RSP, *P_QST_GET_FAN_CTRL_UPDATE_RSP;

#define QST_FAN_CTRL_UPDATE_RSP_SIZE(Sensors)       (((Sensors) * sizeof(QST_FAN_CTRL_UPDATE)) + 1)
#define QST_FAN_CTRL_UPDATE_RSP_SIZE_BAD(RspSize)  ((((RspSize) - 1) % sizeof(QST_FAN_CTRL_UPDATE)) != 0)
#define QST_FAN_CTRL_UPDATE_SENSOR_COUNT(RspSize)  (((RspSize) - 1) / sizeof(QST_FAN_CTRL_UPDATE))

/****************************************************************************/
/* QST_GET_FAN_CTRL_CONFIG_RSP - Command response structure definition      */
/****************************************************************************/

typedef struct _QST_GET_FAN_CTRL_CONFIG_RSP
{
   UINT8                        byStatus;
   INT8                         bControllerEnabled;
   UINT8                        byControllerUsage;

} QST_GET_FAN_CTRL_CONFIG_RSP, *P_QST_GET_FAN_CTRL_CONFIG_RSP;

/****************************************************************************/
/* QST_SET_FAN_CTRL_DUTY_CMD - Command structure definition                 */
/****************************************************************************/

typedef struct _QST_SET_FAN_CTRL_DUTY_CMD
{
   QST_CMD_HEADER               stHeader;
   UINT16                       uDutyCycle;

} QST_SET_FAN_CTRL_DUTY_CMD, *P_QST_SET_FAN_CTRL_DUTY_CMD;

// Useful Macros

#define QST_DUTY_FROM_FLOAT(x)  ((UINT16)((x) * 100))
#define QST_DUTY_FROM_DOUBLE(x) ((UINT16)((x) * 100))

/****************************************************************************/
/****************************************************************************/
/******************* Definitions for SST Command Support ********************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Device Addresses                                                         */
/****************************************************************************/

// Chipset (special reservation inside QST Subsystem)

#define SST_CHIPSET_ADDRESS                 0x20


// CPUs on PECI Bus (special SST Bus segment operating at CPU voltage)

#define SST_FIRST_PECI_ADDRESS              0x30
#define SST_LAST_PECI_ADDRESS               0x3F

#define SST_CPU_1_ADDRESS                   0x30
#define SST_CPU_2_ADDRESS                   0x31
#define SST_CPU_3_ADDRESS                   0x32
#define SST_CPU_4_ADDRESS                   0x33

// Thorpe Mill (Simple Temperature Sensor) SST Bus Devices

#define SST_TM_ADDRESS_1                    0x48
#define SST_TM_ADDRESS_2                    0x49
#define SST_TM_ADDRESS_3                    0x4A
#define SST_TM_ADDRESS_4                    0x4B
#define SST_TM_ADDRESS_5                    0x4C
#define SST_TM_ADDRESS_6                    0x4D
#define SST_TM_ADDRESS_7                    0x4E
#define SST_TM_ADDRESS_8                    0x4F
#define SST_TM_ADDRESS_9                    0x50

// Viers Mill (Combo Temperature And Voltage Sensor) SST Bus Devices
// Note (intentional) overlap with Thorpe Mill device addresses

#define SST_VM_ADDRESS_1                    0x48
#define SST_VM_ADDRESS_2                    0x49
#define SST_VM_ADDRESS_3                    0x4A

/****************************************************************************/
/* Command Bytes                                                            */
/****************************************************************************/

// Common to all devices

#define SST_GET_DIB                         0xF7    // read length = 8 or 16
#define SST_RESET_DEVICE                    0xF6    // read length = 0

// Processor (PECI Interface)

#define SST_PECI_GET_TEMP_DOMAIN_0          0x01
#define SST_PECI_GET_TEMP_DOMAIN_1          0x02

// Thorpe Mill Compliant Devices

#define SST_TM_GET_ALL_TEMPS                0x00    // read length = 4
#define SST_TM_GET_TEMP_INT                 0x00    // read length = 2
#define SST_TM_GET_TEMP_EXT                 0x01

// Viers Mill Compliant Devices

#define SST_VM_GET_ALL_TEMPS                0x00    // read length = 4
#define SST_VM_GET_TEMP_INT                 0x00    // read length = 2
#define SST_VM_GET_TEMP_EXT                 0x01

#define SST_VM_GET_ALL_VOLTAGES             0x10    // read length = 10
#define SST_VM_GET_VOLTAGE_12V              0x10    // read length = 2
#define SST_VM_GET_VOLTAGE_5V               0x11
#define SST_VM_GET_VOLTAGE_3P3V             0x12
#define SST_VM_GET_VOLTAGE_2P5V             0x13
#define SST_VM_GET_VOLTAGE_VCCP             0x14

// Chipset Sensor/Controller Access

#define SST_CS_GET_PCH_TEMP_READING         0x00
#define SST_CS_GET_MCH_TEMP_READING         0x01
#define SST_CS_GET_CPU_1_TEMP_READING       0x02
#define SST_CS_GET_CPU_2_TEMP_READING       0x03
#define SST_CS_GET_CPU_3_TEMP_READING       0x04    // currently not supported
#define SST_CS_GET_CPU_4_TEMP_READING       0x05    // currently not supported

#define SST_CS_GET_DIMM_1_TEMP_READING      0x06
#define SST_CS_GET_DIMM_2_TEMP_READING      0x07
#define SST_CS_GET_DIMM_3_TEMP_READING      0x08
#define SST_CS_GET_DIMM_4_TEMP_READING      0x09
#define SST_CS_GET_DIMM_5_TEMP_READING      0x0A    // currently not supported
#define SST_CS_GET_DIMM_6_TEMP_READING      0x0B    // currently not supported
#define SST_CS_GET_DIMM_7_TEMP_READING      0x0C    // currently not supported
#define SST_CS_GET_DIMM_8_TEMP_READING      0x0D    // currently not supported

#define SST_CS_GET_FAN_SENS_1_ATTRIBS       0x0E
#define SST_CS_GET_FAN_SENS_2_ATTRIBS       0x0F
#define SST_CS_GET_FAN_SENS_3_ATTRIBS       0x10
#define SST_CS_GET_FAN_SENS_4_ATTRIBS       0x11
#define SST_CS_GET_FAN_SENS_5_ATTRIBS       0x12    // currently not supported
#define SST_CS_GET_FAN_SENS_6_ATTRIBS       0x13    // currently not supported
#define SST_CS_GET_FAN_SENS_7_ATTRIBS       0x14    // currently not supported
#define SST_CS_GET_FAN_SENS_8_ATTRIBS       0x15    // currently not supported

#define SST_CS_SET_FAN_SENS_1_CONFIG        0x16
#define SST_CS_SET_FAN_SENS_2_CONFIG        0x17
#define SST_CS_SET_FAN_SENS_3_CONFIG        0x18
#define SST_CS_SET_FAN_SENS_4_CONFIG        0x19
#define SST_CS_SET_FAN_SENS_5_CONFIG        0x1A    // currently not supported
#define SST_CS_SET_FAN_SENS_6_CONFIG        0x1B    // currently not supported
#define SST_CS_SET_FAN_SENS_7_CONFIG        0x1C    // currently not supported
#define SST_CS_SET_FAN_SENS_8_CONFIG        0x1D    // currently not supported

#define SST_CS_GET_FAN_SENS_1_SPEED         0x1E
#define SST_CS_GET_FAN_SENS_2_SPEED         0x1F
#define SST_CS_GET_FAN_SENS_3_SPEED         0x20
#define SST_CS_GET_FAN_SENS_4_SPEED         0x21
#define SST_CS_GET_FAN_SENS_5_SPEED         0x22    // currently not supported
#define SST_CS_GET_FAN_SENS_6_SPEED         0x23    // currently not supported
#define SST_CS_GET_FAN_SENS_7_SPEED         0x24    // currently not supported
#define SST_CS_GET_FAN_SENS_8_SPEED         0x25    // currently not supported

#define SST_CS_GET_FAN_CTRL_1_ATTRIBS       0x26
#define SST_CS_GET_FAN_CTRL_2_ATTRIBS       0x27
#define SST_CS_GET_FAN_CTRL_3_ATTRIBS       0x28
#define SST_CS_GET_FAN_CTRL_4_ATTRIBS       0x29
#define SST_CS_GET_FAN_CTRL_5_ATTRIBS       0x2A    // currently not supported
#define SST_CS_GET_FAN_CTRL_6_ATTRIBS       0x2B    // currently not supported
#define SST_CS_GET_FAN_CTRL_7_ATTRIBS       0x2C    // currently not supported
#define SST_CS_GET_FAN_CTRL_8_ATTRIBS       0x2D    // currently not supported

#define SST_CS_SET_FAN_CTRL_1_CONFIG        0x2E
#define SST_CS_SET_FAN_CTRL_2_CONFIG        0x2F
#define SST_CS_SET_FAN_CTRL_3_CONFIG        0x30
#define SST_CS_SET_FAN_CTRL_4_CONFIG        0x31
#define SST_CS_SET_FAN_CTRL_5_CONFIG        0x32    // currently not supported
#define SST_CS_SET_FAN_CTRL_6_CONFIG        0x33    // currently not supported
#define SST_CS_SET_FAN_CTRL_7_CONFIG        0x34    // currently not supported
#define SST_CS_SET_FAN_CTRL_8_CONFIG        0x35    // currently not supported

#define SST_CS_SET_FAN_CTRL_1_SPEED         0x36
#define SST_CS_SET_FAN_CTRL_2_SPEED         0x37
#define SST_CS_SET_FAN_CTRL_3_SPEED         0x38
#define SST_CS_SET_FAN_CTRL_4_SPEED         0x39
#define SST_CS_SET_FAN_CTRL_5_SPEED         0x3A    // currently not supported
#define SST_CS_SET_FAN_CTRL_6_SPEED         0x3B    // currently not supported
#define SST_CS_SET_FAN_CTRL_7_SPEED         0x3C    // currently not supported
#define SST_CS_SET_FAN_CTRL_8_SPEED         0x3D    // currently not supported

#define SST_CS_GET_FAN_CTRL_1_SPEED         0x3E
#define SST_CS_GET_FAN_CTRL_2_SPEED         0x3F
#define SST_CS_GET_FAN_CTRL_3_SPEED         0x40
#define SST_CS_GET_FAN_CTRL_4_SPEED         0x41
#define SST_CS_GET_FAN_CTRL_5_SPEED         0x42    // currently not supported
#define SST_CS_GET_FAN_CTRL_6_SPEED         0x43    // currently not supported
#define SST_CS_GET_FAN_CTRL_7_SPEED         0x44    // currently not supported
#define SST_CS_GET_FAN_CTRL_8_SPEED         0x45    // currently not supported

/****************************************************************************/
/* SST_CMD_HEADER - Defines header fields for SST Packets.                  */
/****************************************************************************/

typedef struct _SST_CMD_HEADER
{
   UINT8                        bySSTAddress;
   UINT8                        byWriteLength;
   UINT8                        byReadLength;

} SST_CMD_HEADER;

/****************************************************************************/
/* SST_CMD - Defines contents of SST Packets.                               */
/****************************************************************************/

typedef struct _SST_CMD
{
   SST_CMD_HEADER               stSSTHeader;
   UINT8                        byCommandByte;
   UINT16                       wCommandData[1];

} SST_CMD;

/****************************************************************************/
/* QST_SST_PASS_THROUGH_CMD - Command structure definition (with SST        */
/* Packet embedded).                                                        */
/****************************************************************************/

typedef struct _QST_SST_PASS_THROUGH_CMD
{
    QST_CMD_HEADER              stHeader;
    SST_CMD                     stSSTPacket;

} QST_SST_PASS_THROUGH_CMD, *P_QST_SST_PASS_THROUGH_CMD;

// Useful Macros

#define QST_SST_CMD_DATA(DataWords) (4 + (2 * (DataWords)))
#define QST_SST_CMD_SIZE(DataWords) (sizeof(QST_CMD_HEADER) + QST_SST_CMD_DATA(DataWords))

/****************************************************************************/
/* QST_SST_PASS_THROUGH_RSP - Response Structure Definition                 */
/****************************************************************************/

typedef struct _QST_SST_PASS_THROUGH_RSP
{
   UINT8                        byStatus;         // QST Status
   UINT16                       wValue[1];

} QST_SST_PASS_THROUGH_RSP, *P_QST_SST_PASS_THROUGH_RSP;

// Useful Macros

#define QST_SST_RSP_SIZE(NumDataWords) (1 + (2 * (NumDataWords)))

/****************************************************************************/
/* SST_DIB_DEV_CAPABILITIES - DIB Device Capabilities Byte                  */
/****************************************************************************/

typedef struct _SST_DIB_DEV_CAPABILITIES
{
    BIT_FIELD_IN_UINT8          bSlowDevice: 1;
    BIT_FIELD_IN_UINT8          bPMSupport: 1;
    BIT_FIELD_IN_UINT8          bAlertSupport: 1;
    BIT_FIELD_IN_UINT8          bWakeCapable: 1;
    BIT_FIELD_IN_UINT8          uReserved: 1;
    BIT_FIELD_IN_UINT8          uAddressType: 3;

} SST_DIB_DEV_CAPABILITIES;

/****************************************************************************/
/* SST_DIB_VER_REV - DIB Version/Revision Byte                              */
/****************************************************************************/

typedef struct _SST_DIB_VER_REV
{
    BIT_FIELD_IN_UINT8          uMinorRevision: 4;
    BIT_FIELD_IN_UINT8          uSSTVersion: 3;
    BIT_FIELD_IN_UINT8          bPreRelease: 1;

} SST_DIB_VER_REV;

/****************************************************************************/
/* SST_DIB_DEV_IF - DIB Device Interface Byte                               */
/****************************************************************************/

typedef struct _SST_DIB_DEV_IF
{
    BIT_FIELD_IN_UINT8          bVendorSpecificCapabilities: 1;
    BIT_FIELD_IN_UINT8          uReserved: 6;
    BIT_FIELD_IN_UINT8          bDevIFExtUsed: 1;

} SST_DIB_DEV_IF;

/****************************************************************************/
/* SST_DIB_DEV_IF_EXT - DIB Device Interface Extension Byte                 */
/****************************************************************************/

typedef struct _SST_DIB_DEV_IF_EXT
{
    BIT_FIELD_IN_UINT8          uReserved: 4;
    BIT_FIELD_IN_UINT8          uLocation: 4;

} SST_DIB_DEV_IF_EXT;

/****************************************************************************/
/* SST_SHORT_DIB - Short Device Identifier Block (DIB)                      */
/****************************************************************************/

typedef struct _SST_SHORT_DIB
{
   SST_DIB_DEV_CAPABILITIES     stDevCapabilities;
   SST_DIB_VER_REV              stVersionRevision;
   UINT16                       uSSTVendorId;
   UINT16                       uDeviceId;
   SST_DIB_DEV_IF               stDeviceIF;
   UINT8                        uFuncionIF;

} SST_SHORT_DIB, *P_SST_SHORT_DIB;

/****************************************************************************/
/* SST_GET_SHORT_DIB_RSP - Get Short DIB Response Packet                    */
/****************************************************************************/

typedef struct _SST_GET_SHORT_DIB_RSP
{
   UINT8                        byStatus;         // QST Status
   SST_SHORT_DIB                stShortDIB;

} SST_GET_SHORT_DIB_RSP, *P_SST_GET_SHORT_DIB_RSP;

/****************************************************************************/
/* SST_LONG_DIB - Long Device Identifier Block (DIB)                        */
/****************************************************************************/

typedef struct _SST_LONG_DIB
{
   SST_DIB_DEV_CAPABILITIES     stDevCapabilities;
   SST_DIB_VER_REV              stVersionRevision;
   UINT16                       uSSTVendorId;
   UINT16                       uDeviceId;
   SST_DIB_DEV_IF               stDeviceIF;
   UINT8                        uFuncionIF;
   SST_DIB_DEV_IF_EXT           stDeviceIFExt;
   UINT8                        uReserved[3];
   UINT8                        uVendorSpecId[3];
   UINT8                        uClientDevAddr;

} SST_LONG_DIB, *P_SST_LONG_DIB;

/****************************************************************************/
/* SST_GET_LONG_DIB_RSP - Get Long DIB Response Packet                      */
/****************************************************************************/

typedef struct _SST_GET_LONG_DIB_RSP
{
   UINT8                        byStatus;         // QST Status
   SST_LONG_DIB                 stLongDIB;

} SST_GET_LONG_DIB_RSP, *P_SST_GET_LONG_DIB_RSP;

/****************************************************************************/
/* SST_GET_TEMP_READING_RSP - Get Temperature Reading Response Packet       */
/****************************************************************************/

typedef struct _SST_GET_TEMP_READING_RSP
{
   UINT8                        byStatus;         // QST Status
   INT16                        iTempValue[1];

} SST_GET_TEMP_READING_RSP, *P_SST_GET_TEMP_READING_RSP;

// Special temperature encodings for sensor errors

#define SST_TEMP_GENERAL_SENSOR_ERROR 0x8000
#define SST_TEMP_DIODE_FAULT          0x8001
#define SST_TEMP_OP_RANGE_UNDERFLOW   0x8002
#define SST_TEMP_OP_RANGE_OVERFLOW    0x8003

// Useful Macros

#define SST_TEMP_TO_FLOAT(x)    ((float)(x) / 64)
#define SST_TEMP_TO_DOUBLE(x)   ((double)(x) / 64)

#define SST_TEMP_FROM_FLOAT(x)  (INT16)((x) * 64)
#define SST_TEMP_FROM_DOUBLE(x) (INT16)((x) * 64)

/****************************************************************************/
/* SST_GET_VOLT_READING_RSP - Get Voltage Reading Response Packet           */
/****************************************************************************/

typedef struct _SST_GET_VOLT_READING_RSP
{
   UINT8                        byStatus;         // QST Status
   INT16                        iVoltValue[1];

} SST_GET_VOLT_RSP, *P_SST_GET_VOLT_RSP;

// Useful Macros

#define SST_VOLT_TO_FLOAT(x)    ((float)(x) / 1024)
#define SST_VOLT_TO_DOUBLE(x)   ((double)(x) / 1024)

#define SST_VOLT_FROM_FLOAT(x)  ((UINT16)((x) * 1024))
#define SST_VOLT_FROM_DOUBLE(x) ((UINT16)((x) * 1024))

/****************************************************************************/
/* SST_FAN_SENS_ATTRIBS - Fan Sensor Attributes                             */
/****************************************************************************/

typedef struct _SST_FAN_SENS_ATTRIBS
{
   BIT_FIELD_IN_UINT16          bReturnsClocks:1;
   BIT_FIELD_IN_UINT16          uClockPulseMultiplier: 15;

} SST_FAN_SENS_ATTRIBS;

/****************************************************************************/
/* SST_GET_FAN_SENS_ATTRIBS_RSP - Get Fan Sensor Attributes Response Packet */
/****************************************************************************/

typedef struct _SST_GET_FAN_SENS_ATTRIBS_RSP
{
   UINT8                        byStatus;         // QST Status
   SST_FAN_SENS_ATTRIBS         stAttribs;

} SST_GET_FAN_SENS_ATTRIBS_RSP, *P_SST_GET_FAN_SENS_ATTRIBS_RSP;

/****************************************************************************/
/* SST_FAN_SENS_CONFIG - Fan Sensor Configuration                           */
/****************************************************************************/

typedef struct _SST_FAN_SENS_CONFIG
{
   BIT_FIELD_IN_UINT16          uPulsesPerRevolution:3;
   BIT_FIELD_IN_UINT16          bDependentMeasurement:1;
   BIT_FIELD_IN_UINT16          uAssociatedController:2;
   BIT_FIELD_IN_UINT16          uReserved:9;
   BIT_FIELD_IN_UINT16          bSensorEnable:1;

} SST_FAN_SENS_CONFIG;

/****************************************************************************/
/* SST_SET_FAN_SENS_CONFIG_CMD - Set Fan Sensor Configuration Command       */
/* Packet. This is an QST Pass-Through Command with appropriate SST Packet  */
/****************************************************************************/

typedef struct _SST_SET_FAN_SENS_CONFIG_CMD
{
   QST_CMD_HEADER               stQSTHeader;
   SST_CMD_HEADER               stSSTHeader;
   UINT8                        bySSTCommandByte;
   SST_FAN_SENS_CONFIG          stFanSensConfig;

} SST_SET_FAN_SENS_CONFIG_CMD, *P_SST_SET_FAN_SENS_CONFIG_CMD;

/****************************************************************************/
/* SST_GET_FAN_SENS_READING_RSP - Get Fan Sensor Speed Reading Response Pkt */
/****************************************************************************/

typedef struct _SST_GET_FAN_SENS_READING_RSP
{
   UINT8                        byStatus;         // QST Status
   UINT16                       wFanSpeed;

} SST_GET_FAN_SENS_READING_RSP, *P_SST_GET_FAN_SENS_READING_RSP;

/****************************************************************************/
/* SST_FAN_CTRL_ATTRIBS - Fan Controller Attributes. Note: if controller    */
/* returns RPMs, you can use entire 16 bits of response for maximum RPM     */
/* value; the value returned in field uMaxFanRPMs is bits 15::1 (not 14::0) */
/* of value...                                                              */
/****************************************************************************/

typedef struct _SST_FAN_CTRL_ATTRIBS
{
   BIT_FIELD_IN_UINT16          bReturnsDutyCycle:1;
   BIT_FIELD_IN_UINT16          uMaxFanRPMs:15;

} SST_FAN_CTRL_ATTRIBS;

/****************************************************************************/
/* SST_GET_FAN_CTRL_ATTRIBS_RSP - Get Fan Controller Attributes Response Pkt*/
/****************************************************************************/

typedef struct _SST_GET_FAN_CTRL_ATTRIBS_RSP
{
   UINT8                        byStatus;         // QST Status
   SST_FAN_CTRL_ATTRIBS         uAttribs;

} SST_GET_FAN_CTRL_ATTRIBS_RSP, *P_SST_GET_FAN_CTRL_ATTRIBS_RSP;

/****************************************************************************/
/* SST_FAN_CTRL_CONFIG - Fan Controller Configuration. Note: Literals for   */
/* Signal Frequency and Spin-Up Time are defined in QstCfg.h                */
/****************************************************************************/

typedef struct _SST_FAN_CTRL_CONFIG
{
   BIT_FIELD_IN_UINT16          uPWMFrequency:3;
   BIT_FIELD_IN_UINT16          uFanSpinUpTime:3;
   BIT_FIELD_IN_UINT16          bSignalInvert:1;
   BIT_FIELD_IN_UINT16          bTimerPreempt:1;
   BIT_FIELD_IN_UINT16          uWatchdogTimerInterval:3;
   BIT_FIELD_IN_UINT16          uReserved:4;
   BIT_FIELD_IN_UINT16          bControllerEnable:1;

} SST_FAN_CTRL_CONFIG;

// Definitions for field uWatchdogTimerInterval

#define SST_WATCHDOG_TIMEOUT_DISABLED   0x00
#define SST_WATCHDOG_TIMEOUT_2_SECONDS  0x01
#define SST_WATCHDOG_TIMEOUT_4_SECONDS  0x02
#define SST_WATCHDOG_TIMEOUT_8_SECONDS  0x03
#define SST_WATCHDOG_TIMEOUT_16_SECONDS 0x05
#define SST_WATCHDOG_TIMEOUT_32_SECONDS 0x07

/****************************************************************************/
/* SST_SET_FAN_CTRL_CONFIG_CMD - Set Fan Controller Configuration Command   */
/* Packet. This is an QST Pass-Through Command with appropriate SST Packet  */
/****************************************************************************/

typedef struct _SST_SET_FAN_CTRL_CONFIG_CMD
{
   QST_CMD_HEADER               stQSTHeader;
   SST_CMD_HEADER               stSSTHeader;
   UINT8                        bySSTCommandByte;
   SST_FAN_CTRL_CONFIG          stFanCtrlConfig;

} SST_SET_FAN_CTRL_CONFIG_CMD, *P_SST_SET_FAN_CTRL_CONFIG_CMD;

/****************************************************************************/
/* SST_SET_FAN_CTRL_SPEED_CMD - Set Fan Controller Speed Command Packet.    */
/* This is an QST Pass-Through Command with appropriate SST Packet          */
/****************************************************************************/

typedef struct _SST_SET_FAN_CTRL_SPEED_CMD
{
   QST_CMD_HEADER               stQSTHeader;
   SST_CMD_HEADER               stSSTHeader;
   UINT8                        bySSTCommandByte;
   UINT16                       wSpeed;

} SST_SET_FAN_CTRL_SPEED_CMD, *P_SST_SET_FAN_CTRL_SPEED_CMD;

// Useful Macros (but only for fan controllers that take duty cycle values;
// remember, there could be SST devices with controllers that take RPMs...)

#define SST_DUTY_FROM_FLOAT(x)  ((UINT16)((((x) * 255) / 100) + 0.5))
#define SST_DUTY_FROM_DOUBLE(x) ((UINT16)((((x) * 255) / 100) + 0.5))

#define SST_DUTY_TO_FLOAT(x)    (((float)(x) * 100) / 255)
#define SST_DUTY_TO_DOUBLE(x)   (((double)(x) * 100) / 255)

/****************************************************************************/
/* SST_GET_FAN_CTRL_SPEED_RSP - Get Fan Controller Speed Response Packet.   */
/****************************************************************************/

typedef struct _SST_GET_FAN_CTRL_SPEED_RSP
{
   UINT8                        byStatus;         // QST Status
   UINT16                       wSpeed;

} SST_GET_FAN_CTRL_SPEED_RSP, *P_SST_GET_FAN_CTRL_SPEED_RSP;

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // _QSTCMD_H
