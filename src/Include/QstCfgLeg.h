/****************************************************************************/
/*                                                                          */
/*  Module:       QstCfgLeg.h                                               */
/*                                                                          */
/*  Description:  Provides definitions for the structures  in  the  Binary  */
/*                Configuration  Payloads  that  are used to configure the  */
/*                Quiet System Technology (QST) Subsystem.                  */
/*                                                                          */
/*  Notes:     1. Entity  indices  and instance numbers are 1-based in the  */
/*                INI file but are 0-based in the binary data.              */
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

#ifndef _QSTCFGLEG_H
#define _QSTCFGLEG_H

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Simplify subsequent conditional compilation statements                   */
/****************************************************************************/

#if !defined(__MSDOS__) && (defined(MSDOS) || defined(_MSDOS) || defined(__DOS__))
#define __MSDOS__
#endif

#if !defined(__SOLARIS__) && defined(__sun__)
#define __SOLARIS__
#endif

#if !defined(__LINUX__) && defined(__linux__)
#define __LINUX__
#endif

#if !defined(__WIN32__) && (defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__))
#define __WIN32__
#endif

/****************************************************************************/
/* Definitions for structure maximums                                       */
/****************************************************************************/

#define QST_LEG_MAX_TEMP_MONITORS      12
#define QST_LEG_MAX_FAN_MONITORS       8
#define QST_LEG_MAX_VOLT_MONITORS      8
#define QST_LEG_MAX_TEMP_RESPONSES     12
#define QST_LEG_MAX_FAN_CONTROLLERS    8

#ifndef _QSTCFG_H
/****************************************************************************/
/* LONGF - Special data type providing storage for integer values which     */
/* have an implied (100's) decimal place. This is due to unavailability of  */
/* floating point support in the ME.                                        */
/****************************************************************************/

typedef INT32                       INT32F;

/****************************************************************************/
/* Bit field counting is buggy in certain C/C++ compilers. We will handle   */
/* this by maximizing the use of the fixed field length language extension  */
/* that is supported by many compilers, including Borland's and Microsoft's */
/* (16- and 32-bit versions). Note that this language extension is enabled  */
/* by default; if you disable it, compilation errors will abound!!          */
/****************************************************************************/

#if defined(__WIN32__) || defined(__MSDOS__)

#define BIT_FIELD_IN_UINT8          UINT8
#define BIT_FIELD_IN_UINT16         UINT16
#define BIT_FIELD_IN_UINT32         UINT32

#else

#define BIT_FIELD_IN_UINT8          unsigned
#define BIT_FIELD_IN_UINT16         unsigned
#define BIT_FIELD_IN_UINT32         unsigned

#endif

/****************************************************************************/
/* Definitions for the "NONE" value in fields of varying bit length.        */
/****************************************************************************/

#define QST_VALUE_NONE_INT          -1
#define QST_VALUE_NONE_UINT3        0x07
#define QST_VALUE_NONE_UINT4        0x0F
#define QST_VALUE_NONE_UINT5        0x1F
#define QST_VALUE_NONE_UINT8        0xFF
#define QST_VALUE_NONE_UINT16       0xFFFF
#define QST_VALUE_NONE_UINT32       0xFFFFFFFF

#endif   // ndef _QSTCFG_H

/****************************************************************************/
/* QST_LEG_HEADER_STRUCT - Structure Header and associated definitions      */
/****************************************************************************/

typedef struct _QST_LEG_HEADER_STRUCT
{
   BIT_FIELD_IN_UINT16              EntityType:3;
   BIT_FIELD_IN_UINT16              EntityIndex:5;
   BIT_FIELD_IN_UINT16              EntityEnabled:1;
   BIT_FIELD_IN_UINT16              StructLength:7;
   UINT8                            EntityUsage;

} QST_LEG_HEADER_STRUCT, *P_QST_LEG_HEADER_STRUCT;

/****************************************************************************/
/* Definitions for structure types                                          */
/****************************************************************************/

#define QST_LEG_PAYLOAD_HEADER         0
#define QST_LEG_TEMP_MONITOR           1
#define QST_LEG_FAN_MONITOR            2
#define QST_LEG_VOLT_MONITOR           3
#define QST_LEG_TEMP_RESPONSE          4
#define QST_LEG_FAN_CONTROLLER         5

/****************************************************************************/
/* QST_LEG_TEMP_MONITOR_STRUCT - Structure and associated definitions used  */
/* for configuration of Temperature Monitors.                               */
/****************************************************************************/

typedef struct _QST_LEG_TEMP_MONITOR_STRUCT
{
   QST_LEG_HEADER_STRUCT            Header;
   UINT8                            DeviceAddress;
   UINT8                            GetReadingCommand;
   UINT8                            RelativeReadings;
   UINT8                            TimeoutNonCritical;
   UINT8                            TimeoutCritical;
   UINT8                            TimeoutNonRecoverable;
   INT32F                           RelativeConversion;
   INT32F                           AccuracyCorrectionSlope;
   INT32F                           AccuracyCorrectionOffset;
   INT32F                           TemperatureNominal;
   INT32F                           TemperatureNonCritical;
   INT32F                           TemperatureCritical;
   INT32F                           TemperatureNonRecoverable;

} QST_LEG_TEMP_MONITOR_STRUCT, *P_QST_LEG_TEMP_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Temperature Monitor Usage Field                          */
/****************************************************************************/

#define QST_LEG_OTHER_UNKNOWN          0
#define QST_LEG_CPU_CORE_TEMP          1
#define QST_LEG_CPU_DIE_TEMP           2
#define QST_LEG_ICH_TEMP               3
#define QST_LEG_MCH_TEMP               4
#define QST_LEG_VR_TEMP                5
#define QST_LEG_MEM_TEMP               6
#define QST_LEG_MOBO_AMBIENT_TEMP      7
#define QST_LEG_SYS_AMBIENT_TEMP       8
#define QST_LEG_CPU_INLET_TEMP         9
#define QST_LEG_SYS_INLET_TEMP         10
#define QST_LEG_SYS_OUTLET_TEMP        11
#define QST_LEG_PSU_TEMP               12
#define QST_LEG_PSU_INLET_TEMP         13
#define QST_LEG_PSU_OUTLET_TEMP        14
#define QST_LEG_HARD_DRIVE_TEMP        15
#define QST_LEG_GPU_TEMP               16
#define QST_LEG_IOH_TEMP               17

#define QST_LEG_LAST_TEMP_USAGE        17

/****************************************************************************/
/* QST_LEG_FAN_MONITOR_STRUCT - Structure and associated definitions used   */
/* for configuration of Fan Speed Monitors.                                 */
/****************************************************************************/

typedef struct _QST_LEG_FAN_MONITOR_STRUCT
{
   QST_LEG_HEADER_STRUCT            Header;
   UINT8                            DeviceAddress;
   UINT8                            GetAttributesCommand;
   UINT8                            GetReadingCommand;
   UINT16                           SpeedNominal;
   UINT16                           SpeedNonCritical;
   UINT16                           SpeedCritical;
   UINT16                           SpeedNonRecoverable;

} QST_LEG_FAN_MONITOR_STRUCT, *P_QST_LEG_FAN_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Fan Monitor/Controller Usage Field                       */
/****************************************************************************/

#define QST_LEG_CPU_FAN                1
#define QST_LEG_CPU_SYS_FAN            2
#define QST_LEG_MCH_FAN                3
#define QST_LEG_VR_FAN                 4
#define QST_LEG_CHASSIS_FAN            5
#define QST_LEG_CHASSIS_INLET_FAN      6
#define QST_LEG_CHASSIS_OUTLET_FAN     7
#define QST_LEG_PSU_FAN                8
#define QST_LEG_PSU_INLET_FAN          9
#define QST_LEG_PSU_OUTLET_FAN         10
#define QST_LEG_HARD_DRIVE_FAN         11
#define QST_LEG_GPU_FAN                12
#define QST_LEG_AUX_FAN                13
#define QST_LEG_IOH_FAN                14

#define QST_LEG_LAST_FAN_USAGE         14

/****************************************************************************/
/* QST_LEG_VOLT_MONITOR_STRUCT - Structure and associated definitions used  */
/* for configuration of Voltage Monitors. Note: Need 32 bits for voltage    */
/* values because CIM/DMI/etc. require this to be in millivolts and we need */
/* to be able to represent 220 volts once we add support for sensors in the */
/* PSU.                                                                     */
/****************************************************************************/

typedef struct _QST_LEG_VOLT_MONITOR_STRUCT
{
   QST_LEG_HEADER_STRUCT            Header;
   UINT8                            DeviceAddress;
   UINT8                            GetReadingCommand;
   INT32                            VoltageNominal;
   INT32                            UnderVoltageNonCritical;
   INT32                            UnderVoltageCritical;
   INT32                            UnderVoltageNonRecoverable;
   INT32                            OverVoltageNonCritical;
   INT32                            OverVoltageCritical;
   INT32                            OverVoltageNonRecoverable;

} QST_LEG_VOLT_MONITOR_STRUCT, *P_QST_LEG_VOLT_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Voltage Monitor usage                                    */
/****************************************************************************/

#define QST_LEG_12_VOLTS               1
#define QST_LEG_NEG_12_VOLTS           2
#define QST_LEG_5_VOLTS                3
#define QST_LEG_5_VOLT_BACKUP          4
#define QST_LEG_NEG_5_VOLTS            5
#define QST_LEG_3P3_VOLTS              6
#define QST_LEG_2P5_VOLTS              7
#define QST_LEG_1P5_VOLTS              8
#define QST_LEG_CPU1_VCCP_VOLTS        9
#define QST_LEG_CPU2_VCCP_VOLTS        10
#define QST_LEG_CPU3_VCCP_VOLTS        11
#define QST_LEG_CPU4_VCCP_VOLTS        12
#define QST_LEG_PSU_INPUT_VOLTAGE      13
#define QST_LEG_MCH_VCC_VOLTS          14
#define QST_LEG_3P3_VOLT_STANDBY       15
#define QST_LEG_CPU_VTT_VOLTAGE        16
#define QST_LEG_1P8_VOLTS              17

#define QST_LEG_LAST_VOLT_USAGE        17

/****************************************************************************/
/* QST_LEG_TEMP_RESPONSE_STRUCT - Structure and associated definitions used */
/* for configuration of Temperature Responses.                              */
/****************************************************************************/

typedef struct _QST_LEG_TEMP_RESPONSE_STRUCT
{
   QST_LEG_HEADER_STRUCT            Header;
   BIT_FIELD_IN_UINT32              TemperatureMonitor:5;
   BIT_FIELD_IN_UINT32              AmbientTemperatureMonitor:5;
   BIT_FIELD_IN_UINT32              PrimaryFanController:5;
   BIT_FIELD_IN_UINT32              SmoothingWindow:5;
   BIT_FIELD_IN_UINT32              IntegralTimeWindow:5;
   BIT_FIELD_IN_UINT32              DerivativeTimeWindow:5;
   BIT_FIELD_IN_UINT32              Filler:2;
   INT32F                           ProportionalGain;
   INT32F                           IntegralGain;
   INT32F                           DerivativeGain;
   INT32F                           TempLimit;
   INT32F                           TempAllOn;
   INT32F                           AmbientDutyCycleMin;
   INT32F                           AmbientTempMin;
   INT32F                           AmbientTempRange;
   INT32F                           AmbientDutyCycleReduction;

} QST_LEG_TEMP_RESPONSE_STRUCT, *P_QST_LEG_TEMP_RESPONSE_STRUCT;

/****************************************************************************/
/* Definitions for the Temperature Response algorithm variable limits       */
/****************************************************************************/

#define QST_LEG_MAX_TEMP_SMOOTHING     10
#define QST_LEG_MAX_INTEGRAL_WINDOW    20
#define QST_LEG_MAX_DERIVATIVE_WINDOW  30

/****************************************************************************/
/* QST_LEG_FAN_SENSOR_STRUCT - Structure defining fields of the Fan         */
/* Controller configuration structure particular to a Fan Speed Sensor.     */
/****************************************************************************/

typedef struct _QST_LEG_FAN_SENSOR_STRUCT
{
   BIT_FIELD_IN_UINT8               AssociatedFanSpeedMonitor:5;
   BIT_FIELD_IN_UINT8               PulsesPerRevolution:2;
   BIT_FIELD_IN_UINT8               DependentSpeedMeasurement:1;
   UINT8                            SetConfigurationCommand;
   UINT16                           MinimumRPM;

} QST_LEG_FAN_SENSOR_STRUCT;

/****************************************************************************/
/* QST_LEG_FAN_CONTROLLER_STRUCT - Structure and associated definitions     */
/* used for configuration of Fan Speed Controllers.                         */
/****************************************************************************/

#define QST_LEG_MAX_ASSOC_FAN_SENSORS  4

typedef struct _QST_LEG_FAN_CONTROLLER_STRUCT
{
   QST_LEG_HEADER_STRUCT            Header;
   UINT8                            DeviceAddress;
   UINT8                            GetAttributesCommand;
   UINT8                            SetConfigurationCommand;
   UINT8                            SetSpeedCommand;
   UINT8                            GetSpeedCommand;
   QST_LEG_FAN_SENSOR_STRUCT        FanSensor[QST_LEG_MAX_ASSOC_FAN_SENSORS];
   BIT_FIELD_IN_UINT16              PhysicalControllerIndex:3;
   BIT_FIELD_IN_UINT16              OFFMode:1;         // FALSE = MIN Mode
   BIT_FIELD_IN_UINT16              SignalInvert:1;
   BIT_FIELD_IN_UINT16              SignalFrequency:3;
   BIT_FIELD_IN_UINT16              SpinUpTime:3;
   BIT_FIELD_IN_UINT16              Filler:5;
   UINT16                           DutyCycleMin;
   UINT16                           DutyCycleOn;
   UINT16                           DutyCycleMax;
   INT32F                           ResponseWeighting[QST_LEG_MAX_TEMP_RESPONSES];

} QST_LEG_FAN_CONTROLLER_STRUCT, *P_QST_LEG_FAN_CONTROLLER_STRUCT;

/****************************************************************************/
/* Definitions for Fan Controller Pulses Per Revolution Fields              */
/****************************************************************************/

#define QST_LEG_1_PULSE                0
#define QST_LEG_2_PULSES               1
#define QST_LEG_3_PULSES               2
#define QST_LEG_4_PULSES               3

/****************************************************************************/
/* Definitions for Fan Controller Signal Frequency Field                    */
/****************************************************************************/

#define QST_LEG_FREQ_10                0
#define QST_LEG_FREQ_23                1
#define QST_LEG_FREQ_38                2
#define QST_LEG_FREQ_62                3
#define QST_LEG_FREQ_94                4
#define QST_LEG_FREQ_22000             5
#define QST_LEG_FREQ_25000             6
#define QST_LEG_FREQ_28000             7

/****************************************************************************/
/* Definitions for Fan Controller Spin Up Time Field                        */
/****************************************************************************/

#define QST_LEG_SPIN_0_MS              0
#define QST_LEG_SPIN_250_MS            1
#define QST_LEG_SPIN_500_MS            2
#define QST_LEG_SPIN_750_MS            3
#define QST_LEG_SPIN_1000_MS           4
#define QST_LEG_SPIN_1500_MS           5
#define QST_LEG_SPIN_2000_MS           6
#define QST_LEG_SPIN_4000_MS           7

/****************************************************************************/
/* QST_LEG_PAYLOAD_HEADER_STRUCT - Structure and associated definitions     */
/* used for defining the contents of the header for the Binary Payload      */
/****************************************************************************/

typedef struct _QST_LEG_PAYLOAD_HEADER_STRUCT
{
   UINT32                           Signature;
   UINT8                            VersionMajor;
   UINT8                            VersionMinor;
   UINT16                           PayloadLength;

} QST_LEG_PAYLOAD_HEADER_STRUCT, *P_QST_LEG_PAYLOAD_HEADER_STRUCT;

/****************************************************************************/
/* Definitions for Payload Header fields                                    */
/****************************************************************************/

#define QST_LEG_SIGNATURE              'AFSC'
#define QST_LEG_SIGNATURE_DWORD        "CSFA"

#define QST_LEG_CONFIG_VERSION_MAJOR   1
#define QST_LEG_CONFIG_VERSION_MINOR   0

/****************************************************************************/
/* QST_LEG_PAYLOAD_STRUCT - Structure defining the entire contents of the   */
/* Binary Payload.                                                          */
/****************************************************************************/

typedef struct _QST_LEG_PAYLOAD_STRUCT
{
   QST_LEG_PAYLOAD_HEADER_STRUCT       Header;
   QST_LEG_TEMP_MONITOR_STRUCT         TempMon[QST_LEG_MAX_TEMP_MONITORS];
   QST_LEG_FAN_MONITOR_STRUCT          FanMon[QST_LEG_MAX_FAN_MONITORS];
   QST_LEG_VOLT_MONITOR_STRUCT         VoltMon[QST_LEG_MAX_VOLT_MONITORS];
   QST_LEG_TEMP_RESPONSE_STRUCT        Response[QST_LEG_MAX_TEMP_RESPONSES];
   QST_LEG_FAN_CONTROLLER_STRUCT       FanCtrl[QST_LEG_MAX_FAN_CONTROLLERS];

} QST_LEG_PAYLOAD_STRUCT, *P_QST_LEG_PAYLOAD_STRUCT;

/****************************************************************************/
/* Structure Lengths                                                        */
/****************************************************************************/

#define QST_LEG_PAYLOAD_SIZE           sizeof(QST_LEG_PAYLOAD_STRUCT)
#define QST_LEG_TEMP_MONITOR_SIZE      sizeof(QST_LEG_TEMP_MONITOR_STRUCT)
#define QST_LEG_FAN_MONITOR_SIZE       sizeof(QST_LEG_FAN_MONITOR_STRUCT)
#define QST_LEG_VOLT_MONITOR_SIZE      sizeof(QST_LEG_VOLT_MONITOR_STRUCT)
#define QST_LEG_TEMP_RESPONSE_SIZE     sizeof(QST_LEG_TEMP_RESPONSE_STRUCT)
#define QST_LEG_FAN_CONTROLLER_SIZE    sizeof(QST_LEG_FAN_CONTROLLER_STRUCT)

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // _QSTCFG_H

