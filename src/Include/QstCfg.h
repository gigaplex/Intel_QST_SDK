/****************************************************************************/
/*                                                                          */
/*  Module:         QstCfg.h                                                */
/*                                                                          */
/*  Description:    Provides definitions for the structures in the  Binary  */
/*                  Configuration  Payloads that are used to configure the  */
/*                  Quiet System Technology (QST) Subsystem.                */
/*                                                                          */
/*  Notes:      1.  Entity indices and instance numbers are 1-based within  */
/*                  the INI file but are 0-based within the binary data.    */
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

#ifndef _QSTCFG_H
#define _QSTCFG_H

#include "typedef.h"

#if _MSC_VER > 1000
#pragma once
#pragma warning( disable: 4142 )    // Ignore equivalent redefinitions
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
/* These Literals define the absolute maximum number of entities of each    */
/* particular type that needs to be supported by QST-aware applications. A  */
/* particular build of the QST F/W will support some specific subset of     */
/* these entities.                                                          */
/****************************************************************************/

#define QST_ABS_TEMP_MONITORS       32
#define QST_ABS_FAN_MONITORS        32
#define QST_ABS_VOLT_MONITORS       32
#define QST_ABS_CURR_MONITORS       32
#define QST_ABS_TEMP_RESPONSES      32
#define QST_ABS_FAN_CONTROLLERS     32

#define QST_ABS_CPUS_SUPPORTED      4

/****************************************************************************/
/* INT32LF - Special data type providing storage for integer values which   */
/* have an implied (10000's) decimal place.                                 */
/****************************************************************************/

typedef INT32                       INT32LF;

#ifndef _QSTCFGLEG_H
/****************************************************************************/
/* INT32F - Special data type providing storage for integer values which    */
/* have an implied (100's) decimal place.                                   */
/****************************************************************************/

typedef INT32                       INT32F;

/****************************************************************************/
/* Bit field counting is buggy in certain releases of the C/C++ compilers.  */
/* We handle this by maximizing the use of the fixed field length language  */
/* extension that is supported by many compilers (including Borland's and   */
/* Microsoft's 16- and 32-bit versions). The compilers enable this language */
/* extension by default; if you disable it, compilation errors will abound! */
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

#endif // ndef _QSTCFGLEG_H

/****************************************************************************/
/* QST_HEADER_STRUCT - Structure Header and associated definitions          */
/****************************************************************************/

typedef struct _QST_HEADER_STRUCT
{
   BIT_FIELD_IN_UINT8               EntityEnabled:1;
   BIT_FIELD_IN_UINT8               EntityType:4;
   BIT_FIELD_IN_UINT8               Filler:3;
   UINT8                            EntityIndex;
   UINT8                            EntityUsage;
   UINT8                            StructLength;

} QST_HEADER_STRUCT, *P_QST_HEADER_STRUCT;

/****************************************************************************/
/* Definitions for structure/entity types                                   */
/****************************************************************************/

#define QST_PAYLOAD_HEADER          0
#define QST_TEMP_MONITOR            1
#define QST_FAN_MONITOR             2
#define QST_VOLT_MONITOR            3
#define QST_CURR_MONITOR            4
#define QST_TEMP_RESPONSE           5
#define QST_FAN_CONTROLLER          6

/****************************************************************************/
/* QST_TEMP_MONITOR_STRUCT - Structure and associated definitions used for  */
/* configuration of Temperature Monitors.                                   */
/****************************************************************************/

typedef struct _QST_TEMP_MONITOR_STRUCT
{
   QST_HEADER_STRUCT                Header;
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

} QST_TEMP_MONITOR_STRUCT, *P_QST_TEMP_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Temperature Monitor Usage Field                          */
/****************************************************************************/

#define QST_OTHER_UNKNOWN           0
#define QST_CPU_CORE_TEMP           1
#define QST_CPU_DIE_TEMP            2
#define QST_ICH_TEMP                3
#define QST_MCH_TEMP                4
#define QST_VR_TEMP                 5
#define QST_MEM_TEMP                6
#define QST_MOBO_AMBIENT_TEMP       7
#define QST_SYS_AMBIENT_TEMP        8
#define QST_CPU_INLET_TEMP          9
#define QST_SYS_INLET_TEMP          10
#define QST_SYS_OUTLET_TEMP         11
#define QST_PSU_TEMP                12
#define QST_PSU_INLET_TEMP          13
#define QST_PSU_OUTLET_TEMP         14
#define QST_HARD_DRIVE_TEMP         15
#define QST_GPU_TEMP                16
#define QST_IOH_TEMP                17
#define QST_PCH_TEMP                18

#define QST_LAST_TEMP_USAGE         18

/****************************************************************************/
/* QST_FAN_MONITOR_STRUCT - Structure and associated definitions used for   */
/* configuration of Fan Speed Monitors.                                     */
/****************************************************************************/

typedef struct _QST_FAN_MONITOR_STRUCT
{
   QST_HEADER_STRUCT                Header;
   UINT8                            DeviceAddress;
   UINT8                            GetAttributesCommand;
   UINT8                            GetReadingCommand;
   UINT16                           SpeedNominal;
   UINT16                           SpeedNonCritical;
   UINT16                           SpeedCritical;
   UINT16                           SpeedNonRecoverable;

} QST_FAN_MONITOR_STRUCT, *P_QST_FAN_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Fan Monitor/Controller Usage Field                       */
/****************************************************************************/

#define QST_CPU_FAN                 1
#define QST_CPU_SYS_FAN             2
#define QST_MCH_FAN                 3
#define QST_VR_FAN                  4
#define QST_CHASSIS_FAN             5
#define QST_CHASSIS_INLET_FAN       6
#define QST_CHASSIS_OUTLET_FAN      7
#define QST_PSU_FAN                 8
#define QST_PSU_INLET_FAN           9
#define QST_PSU_OUTLET_FAN          10
#define QST_HARD_DRIVE_FAN          11
#define QST_GPU_FAN                 12
#define QST_AUX_FAN                 13
#define QST_IOH_FAN                 14
#define QST_PCH_FAN                 15
#define QST_MEM_FAN                 16

#define QST_LAST_FAN_USAGE          16

/****************************************************************************/
/* QST_VOLT_MONITOR_STRUCT - Structure and associated definitions used for  */
/* configuration of Voltage Monitors.                                       */
/****************************************************************************/

typedef struct _QST_VOLT_MONITOR_STRUCT
{
   QST_HEADER_STRUCT                Header;
   UINT8                            DeviceAddress;
   UINT8                            GetReadingCommand;
   INT32                            AccuracyCorrectionSlope;
   INT32                            AccuracyCorrectionOffset;
   INT32                            VoltageNominal;
   INT32                            UnderVoltageNonCritical;
   INT32                            UnderVoltageCritical;
   INT32                            UnderVoltageNonRecoverable;
   INT32                            OverVoltageNonCritical;
   INT32                            OverVoltageCritical;
   INT32                            OverVoltageNonRecoverable;

} QST_VOLT_MONITOR_STRUCT, *P_QST_VOLT_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Voltage Monitor usage                                    */
/****************************************************************************/

#define QST_12_VOLTS                1
#define QST_NEG_12_VOLTS            2
#define QST_5_VOLTS                 3
#define QST_5_VOLT_BACKUP           4
#define QST_NEG_5_VOLTS             5
#define QST_3P3_VOLTS               6
#define QST_2P5_VOLTS               7
#define QST_1P5_VOLTS               8
#define QST_CPU1_VCCP_VOLTS         9
#define QST_CPU2_VCCP_VOLTS         10
#define QST_CPU3_VCCP_VOLTS         11
#define QST_CPU4_VCCP_VOLTS         12
#define QST_PSU_INPUT_VOLTAGE       13
#define QST_MCH_VCC_VOLTAGE         14
#define QST_3P3_VOLT_STANDBY        15
#define QST_CPU_VTT_VOLTAGE         16
#define QST_1P8_VOLTS               17
#define QST_PCH_VCC_VOLTAGE         18
#define QST_SDRAM_VCC_VOLTAGE       19

#define QST_LAST_VOLT_USAGE         19

/****************************************************************************/
/* QST_CURR_MONITOR_STRUCT - Structure and associated definitions used for  */
/* configuration of Current Monitors.                                       */
/****************************************************************************/

typedef struct _QST_CURR_MONITOR_STRUCT
{
   QST_HEADER_STRUCT                Header;
   UINT8                            SensorType;
   UINT8                            DeviceAddress;
   UINT8                            GetReadingCommand;
   INT32                            AdjustmentSlope;
   INT32                            AdjustmentOffset;
   INT32                            CurrentNominal;
   INT32                            UnderCurrentNonCritical;
   INT32                            UnderCurrentCritical;
   INT32                            UnderCurrentNonRecoverable;
   INT32                            OverCurrentNonCritical;
   INT32                            OverCurrentCritical;
   INT32                            OverCurrentNonRecoverable;

} QST_CURR_MONITOR_STRUCT, *P_QST_CURR_MONITOR_STRUCT;

/****************************************************************************/
/* Definitions for Current Monitor Sensor Types                             */
/****************************************************************************/

#define QST_CURRENT_SENSOR          0       // True current sensor (adjustment
                                            // slope/offset used for accuracy)

#define QST_VOLTAGE_SENSOR          1       // Current derived from voltage
                                            // (using adjustment slope/offset)

/****************************************************************************/
/* Definitions for Current Monitor usage                                    */
/****************************************************************************/

#define QST_12V_CURRENT             1
#define QST_NEG_12V_CURRENT         2
#define QST_5V_CURRENT              3
#define QST_5V_BACKUP_CURRENT       4
#define QST_NEG_5V_CURRENT          5
#define QST_3P3V_CURRENT            6
#define QST_2P5V_CURRENT            7
#define QST_1P5V_CURRENT            8
#define QST_CPU1_CURRENT            9
#define QST_CPU2_CURRENT            10
#define QST_CPU3_CURRENT            11
#define QST_CPU4_CURRENT            12
#define QST_PSU_INPUT_CURRENT       13
#define QST_MCH_CURRENT             14
#define QST_3P3V_STANDBY_CURRENT    15
#define QST_1P8V_CURRENT            16
#define QST_PCH_CURRENT             17
#define QST_SDRAM_CURRENT           18

#define QST_LAST_CURR_USAGE         18

/****************************************************************************/
/* QST_TEMP_RESPONSE_STRUCT - Structure and associated definitions used     */
/* for configuration of Temperature Responses.                              */
/****************************************************************************/

typedef struct _QST_TEMP_RESPONSE_STRUCT
{
   QST_HEADER_STRUCT                Header;
   UINT8                            TemperatureMonitor;
   BIT_FIELD_IN_UINT16              SmoothingWindow:5;
   BIT_FIELD_IN_UINT16              IntegralTimeWindow:5;
   BIT_FIELD_IN_UINT16              DerivativeTimeWindow:5;
   BIT_FIELD_IN_UINT16              Filler:1;
   INT32F                           ProportionalGain;
   INT32F                           IntegralGain;
   INT32F                           DerivativeGain;
   INT32F                           TempLimit;
   INT32F                           TempAllOn;

} QST_TEMP_RESPONSE_STRUCT, *P_QST_TEMP_RESPONSE_STRUCT;

/****************************************************************************/
/* Definitions for the Temperature Response algorithm variable limits       */
/****************************************************************************/

#define QST_MAX_TEMP_SMOOTHING      10
#define QST_MAX_INTEGRAL_WINDOW     20
#define QST_MAX_DERIVATIVE_WINDOW   30

/****************************************************************************/
/* QST_FAN_SENSOR_STRUCT - Structure defining fields of the Fan Controller  */
/* configuration structure particular to a Fan Speed Sensor.                */
/****************************************************************************/

typedef struct _QST_FAN_SENSOR_STRUCT
{
   BIT_FIELD_IN_UINT8               AssociatedFanSpeedMonitor:5;
   BIT_FIELD_IN_UINT8               PulsesPerRevolution:2;
   BIT_FIELD_IN_UINT8               DependentSpeedMeasurement:1;
   UINT8                            SetConfigurationCommand;
   UINT16                           MinDutyRPMMin;
   UINT16                           MinDutyRPMMax;

} QST_FAN_SENSOR_STRUCT;

/****************************************************************************/
/* QST_AMBIENT_LIMITER - Structure defining fields of the Fan Controller    */
/* configuration structure that are particular to ambient temperature floor */
/* or ceiling implementation.                                               */
/****************************************************************************/

typedef struct _QST_AMBIENT_LIMITER
{
   UINT8                            uTempMonitor;
   INT32F                           lfDutyCycleRange;
   INT32F                           lfTempMin;
   INT32F                           lfTempRange;

} QST_AMBIENT_LIMITER;

/****************************************************************************/
/* QST_FAN_CONTROLLER_STRUCT - Structure and associated definitions used    */
/* for configuration of Fan Speed Controllers.                              */
/****************************************************************************/

#define QST_MAX_ASSOC_FAN_SENSORS   4

#define DEFINE_QST_FAN_CONTROLLER_STRUCT(StructName, Responses)                  \
typedef struct _##StructName                                                     \
{                                                                                \
   QST_HEADER_STRUCT                Header;                                      \
   UINT8                            DeviceAddress;                               \
   UINT8                            GetAttributesCommand;                        \
   UINT8                            SetConfigurationCommand;                     \
   UINT8                            SetSpeedCommand;                             \
   UINT8                            GetSpeedCommand;                             \
   QST_FAN_SENSOR_STRUCT            FanSensor[QST_MAX_ASSOC_FAN_SENSORS];        \
   BIT_FIELD_IN_UINT16              PhysicalControllerIndex:3;                   \
   BIT_FIELD_IN_UINT16              OFFMode:1;                                   \
   BIT_FIELD_IN_UINT16              SignalInvert:1;                              \
   BIT_FIELD_IN_UINT16              SignalFrequency:3;                           \
   BIT_FIELD_IN_UINT16              SpinUpTime:3;                                \
   BIT_FIELD_IN_UINT16              Filler:5;                                    \
   UINT16                           DutyCycleMin;                                \
   UINT16                           DutyCycleOn;                                 \
   QST_AMBIENT_LIMITER              AmbientFloor;                                \
   UINT16                           DutyCycleMax;                                \
   QST_AMBIENT_LIMITER              AmbientCeiling;                              \
   INT32F                           ResponseWeighting[Responses];                \
                                                                                 \
} StructName, *P_##StructName;

DEFINE_QST_FAN_CONTROLLER_STRUCT(QST_FAN_CONTROLLER_STRUCT, QST_ABS_TEMP_RESPONSES)

/****************************************************************************/
/* Definitions for Fan Controller Pulses Per Revolution Fields              */
/****************************************************************************/

#define QST_1_PULSE                 0
#define QST_2_PULSES                1
#define QST_3_PULSES                2
#define QST_4_PULSES                3

/****************************************************************************/
/* Definitions for Fan Controller Signal Frequency Field                    */
/****************************************************************************/

#define QST_FREQ_10                 0
#define QST_FREQ_23                 1
#define QST_FREQ_38                 2
#define QST_FREQ_62                 3
#define QST_FREQ_94                 4
#define QST_FREQ_22000              5
#define QST_FREQ_25000              6
#define QST_FREQ_28000              7

/****************************************************************************/
/* Definitions for Fan Controller Spin Up Time Field                        */
/****************************************************************************/

#define QST_SPIN_0_MS               0
#define QST_SPIN_250_MS             1
#define QST_SPIN_500_MS             2
#define QST_SPIN_750_MS             3
#define QST_SPIN_1000_MS            4
#define QST_SPIN_1500_MS            5
#define QST_SPIN_2000_MS            6
#define QST_SPIN_4000_MS            7

/****************************************************************************/
/* QST_PAYLOAD_HEADER_STRUCT - Structure and associated definitions used    */
/* for defining the contents of the header for the Binary Payload           */
/****************************************************************************/

typedef struct _QST_PAYLOAD_HEADER_STRUCT
{
   UINT32                           Signature;
   UINT8                            VersionMajor;
   UINT8                            VersionMinor;
   UINT16                           PayloadLength;

} QST_PAYLOAD_HEADER_STRUCT, *P_QST_PAYLOAD_HEADER_STRUCT;

/****************************************************************************/
/* Definitions for Payload Header fields                                    */
/****************************************************************************/

#define QST_SIGNATURE               'AFSC'
#define QST_SIGNATURE_DWORD         "CSFA"

#define QST_CONFIG_VERSION_MAJOR    2
#define QST_CONFIG_VERSION_MINOR    0

/****************************************************************************/
/* QST_ABS_PAYLOAD_STRUCT - Structure defining the contents of a maximal    */
/* Binary Configuration Payload.                                            */
/****************************************************************************/

typedef struct _QST_ABS_PAYLOAD_STRUCT
{
   QST_PAYLOAD_HEADER_STRUCT        Header;
   QST_TEMP_MONITOR_STRUCT          TempMon[QST_ABS_TEMP_MONITORS];
   QST_FAN_MONITOR_STRUCT           FanMon[QST_ABS_FAN_MONITORS];
   QST_VOLT_MONITOR_STRUCT          VoltMon[QST_ABS_VOLT_MONITORS];
   QST_CURR_MONITOR_STRUCT          CurrMon[QST_ABS_CURR_MONITORS];
   QST_TEMP_RESPONSE_STRUCT         Response[QST_ABS_TEMP_RESPONSES];
   QST_FAN_CONTROLLER_STRUCT        FanCtrl[QST_ABS_FAN_CONTROLLERS];

} QST_ABS_PAYLOAD_STRUCT, *P_QST_ABS_PAYLOAD_STRUCT;

/****************************************************************************/
/* Structure Lengths.                                                       */
/****************************************************************************/

#define QST_ABS_PAYLOAD_SIZE        sizeof(QST_ABS_PAYLOAD_STRUCT)
#define QST_TEMP_MONITOR_SIZE       sizeof(QST_TEMP_MONITOR_STRUCT)
#define QST_FAN_MONITOR_SIZE        sizeof(QST_FAN_MONITOR_STRUCT)
#define QST_VOLT_MONITOR_SIZE       sizeof(QST_VOLT_MONITOR_STRUCT)
#define QST_CURR_MONITOR_SIZE       sizeof(QST_CURR_MONITOR_STRUCT)
#define QST_TEMP_RESPONSE_SIZE      sizeof(QST_TEMP_RESPONSE_STRUCT)

/****************************************************************************/
/* Special macros for Fan Controller structures that allow you to calculate */
/* the size of a particular Fan Controller structure instance, based upon   */
/* the number of weightings (that will be) included, and to calculate the   */
/* number of weightings included in a particular Fan Controller structure   */
/* instance, based upon the size of this structure instance.                */
/****************************************************************************/

#define QST_FAN_CONTROLLER_SIZE( NumResponses ) \
    (sizeof(QST_FAN_CONTROLLER_STRUCT) - ((QST_ABS_TEMP_RESPONSES - (NumResponses)) * sizeof(INT32F)))

#define QST_FAN_CONTROLLER_RESPONSES( CurrSize ) \
    ((CurrSize - (sizeof(QST_FAN_CONTROLLER_STRUCT) - (QST_ABS_FAN_CONTROLLERS * sizeof(INT32F)))) / sizeof(INT32F))

#pragma pack()

#endif // _QSTCFG_H

