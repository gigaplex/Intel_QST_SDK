/****************************************************************************/
/*                                                                          */
/*  Module:       QstInst.h                                                 */
/*                                                                          */
/*  Description:  Defines the functions and  associated  definitions  that  */
/*                are  necessary  to  utilize  the  services  of  the QST   */
/*                Instrumentation Layer DLL. This DLL exposes  the  Health  */
/*                Monitoring  services provided by the Intel(R) Quiet       */
/*                System Technology (QST) Subsystem.                        */
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

#ifndef _QSTINST_H
#define _QSTINST_H

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include "typedef.h"
#define  APIENTRY
#endif

#include <time.h>

#ifdef   _TIME32_T_DEFINED
#define  QST_TIME_T __time32_t
#else
#define  QST_TIME_T time_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

/****************************************************************************/
/* Limit Definitions                                                        */
/****************************************************************************/

#define QST_MAX_TEMPERATURE_SENSORS             32
#define QST_MAX_FAN_SPEED_SENSORS               32
#define QST_MAX_VOLTAGE_SENSORS                 32
#define QST_MAX_CURRENT_SENSORS                 32
#define QST_MAX_FAN_SPEED_CONTROLLERS           32

/****************************************************************************/
/* QST_SENSOR_TYPE - An enumeration that provides definitions for the       */
/*  types of sensors that may be monitored by the QST Subsystem.            */
/****************************************************************************/

typedef enum _QST_SENSOR_TYPE
{
    TEMPERATURE_SENSOR                          = 0,
    VOLTAGE_SENSOR                              = 1,
    FAN_SPEED_SENSOR                            = 2,
    CURRENT_SENSOR                              = 3

} QST_SENSOR_TYPE;

/****************************************************************************/
/* QST_FUNCTION - An enumeration that provides definitions for the monitor  */
/* and control functions that particular temperature/Voltage/Fan Speed      */
/* Sensors and Fan Speed Controllers are responsible for. Note that the     */
/* values assigned to the different types of Sensors and Controllers are in */
/* overlapping arrays; the sensor/controller type is required to provide    */
/* differentiation.                                                         */
/****************************************************************************/

typedef enum _QST_FUNCTION
{
    UNKNOWN_OTHER                               = 0,

    // Temperature Sensors

    CPU_CORE_TEMPERATURE                        = 1,
    CPU_DIE_TEMPERATURE                         = 2,
    ICH_TEMPERATURE                             = 3,
    MCH_TEMPERATURE                             = 4,
    VR_TEMPERATURE                              = 5,
    MEMORY_TEMPERATURE                          = 6,
    MOTHERBOARD_AMBIENT_TEMPERATURE             = 7,
    SYSTEM_AMBIENT_AIR_TEMPERATURE              = 8,
    CPU_INLET_AIR_TEMPERATURE                   = 9,
    SYSTEM_INLET_AIR_TEMPERATURE                = 10,
    SYSTEM_OUTLET_AIR_TEMPERATURE               = 11,
    PSU_HOTSPOT_TEMPERATURE                     = 12,
    PSU_INLET_AIR_TEMPERATURE                   = 13,
    PSU_OUTLET_AIR_TEMPERATURE                  = 14,
    DRIVE_TEMPERATURE                           = 15,
    GPU_TEMPERATURE                             = 16,
    IOH_TEMPERATURE                             = 17,
    PCH_TEMPERATURE                             = 18,

    // Voltage Sensors

    PLUS_12_VOLTS                               = 1,
    NEG_12_VOLTS                                = 2,
    PLUS_5_VOLTS                                = 3,
    PLUS_5_VOLT_BACKUP                          = 4,
    NEG_5_VOLTS                                 = 5,
    PLUS_3P3_VOLTS                              = 6,
    PLUS_2P5_VOLTS                              = 7,
    PLUS_1P5_VOLTS                              = 8,
    CPU_1_VCCP_VOLTAGE                          = 9,
    CPU_2_VCCP_VOLTAGE                          = 10,
    CPU_3_VCCP_VOLTAGE                          = 11,
    CPU_4_VCCP_VOLTAGE                          = 12,
    PSU_INPUT_VOLTAGE                           = 13,
    MCH_VCC_VOLTAGE                             = 14,
    PLUS_3P3_VOLT_STANDBY                       = 15,
    CPU_VTT_VOLTAGE                             = 16,
    PLUS_1P8_VOLTS                              = 17,
    PCH_VCC_VOLTAGE                             = 18,
    SDRAM_VCC_VOLTAGE                           = 19,

    // Current Sensors

    PLUS_12_VOLT_CURRENT                        = 1,
    NEG_12_VOLT_CURRENT                         = 2,
    PLUS_5_VOLT_CURRENT                         = 3,
    PLUS_5_VOLT_BACKUP_CURRENT                  = 4,
    NEG_5_VOLT_CURRENT                          = 5,
    PLUS_3P3_VOLT_CURRENT                       = 6,
    PLUS_2P5_VOLT_CURRENT                       = 7,
    PLUS_1P5_VOLT_CURRENT                       = 8,
    CPU_1_CURRENT                               = 9,
    CPU_2_CURRENT                               = 10,
    CPU_3_CURRENT                               = 11,
    CPU_4_CURRENT                               = 12,
    PSU_INPUT_CURRENT                           = 13,
    MCH_CURRENT                                 = 14,
    PLUS_3P3_VOLT_STANDBY_CURRENT               = 15,
    PLUS_1P8_VOLT_CURRENT                       = 16,
    PCH_CURRENT                                 = 17,
    SDRAM_CURRENT                               = 18,

    // Fan Speed Sensors and Controllers

    CPU_COOLING_FAN                             = 1,
    SYSTEM_COOLING_FAN                          = 2,
    MCH_COOLING_FAN                             = 3,
    VR_COOLING_FAN                              = 4,
    CHASSIS_COOLING_FAN                         = 5,
    CHASSIS_INLET_FAN                           = 6,
    CHASSIS_OUTLET_FAN                          = 7,
    PSU_COOLING_FAN                             = 8,
    PSU_INLET_FAN                               = 9,
    PSU_OUTLET_FAN                              = 10,
    DRIVE_COOLING_FAN                           = 11,
    GPU_COOLING_FAN                             = 12,
    AUX_COOLING_FAN                             = 13,
    IOH_COOLING_FAN                             = 14,
    PCH_COOLING_FAN                             = 15,
    MEM_COOLING_FAN                             = 16,

} QST_FUNCTION;

/****************************************************************************/
/* QST_HEALTH - An enumeration that provides definitions for the health     */
/* status indicators for Sensors and Controllers that are monitored or      */
/* maintained by the QST Subsystem.                                         */
/****************************************************************************/

typedef enum _QST_HEALTH
{
    HEALTH_NORMAL                               = 0,
    HEALTH_NONCRITICAL                          = 1,
    HEALTH_CRITICAL                             = 2,
    HEALTH_NONRECOVERABLE                       = 3

} QST_HEALTH;

/****************************************************************************/
/* FSC_CONTROL_STATE - An enumeration that provides definitions for the     */
/* control state of the fan speed controllers maintained by the QST         */
/* Subsystem.                                                               */
/****************************************************************************/

typedef enum _QST_CONTROL_STATE
{
    CONTROL_NORMAL                              = 0,
    CONTROL_OVERRIDE_SOFTWARE                   = 1,
    CONTROL_OVERRIDE_SENSOR_ERROR               = 2,
    CONTROL_OVERRIDE_CONTROLLER_ERROR           = 3

} QST_CONTROL_STATE;

/****************************************************************************/
/* Function Prototypes for implicit DLL loading (static binding)            */
/****************************************************************************/

BOOL APIENTRY QstGetSensorCount
(
    IN  QST_SENSOR_TYPE                         SensorType,
    OUT int                                     *pSensorCount
);

BOOL APIENTRY QstGetSensorConfiguration
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT QST_FUNCTION                            *pSensorFunction,
    OUT BOOL                                    *pRelativeReadings,
    OUT float                                   *pNominalReading
);

BOOL APIENTRY QstGetSensorThresholdsHigh
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pThresholdNonCritical,
    OUT float                                   *pThresholdCritical,
    OUT float                                   *pThresholdNonRecoverable
);

BOOL APIENTRY QstGetSensorThresholdsLow
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pThresholdNonCritical,
    OUT float                                   *pThresholdCritical,
    OUT float                                   *pThresholdNonRecoverable
);

BOOL APIENTRY QstGetSensorHealth
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT QST_HEALTH                              *pSensorHealth
);

BOOL APIENTRY QstGetSensorReading
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pSensorReading
);

BOOL APIENTRY QstSetSensorThresholdsHigh
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  float                                   ThresholdNonCritical,
    IN  float                                   ThresholdCritical,
    IN  float                                   ThresholdNonRecoverable
);

BOOL APIENTRY QstSensorThresholdsHighChanged
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

BOOL APIENTRY QstSetSensorThresholdsLow
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  float                                   ThresholdNonCritical,
    IN  float                                   ThresholdCritical,
    IN  float                                   ThresholdNonRecoverable
);

BOOL APIENTRY QstSensorThresholdsLowChanged
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

BOOL APIENTRY QstGetControllerCount
(
    OUT int                                     *pControllerCount
);

BOOL APIENTRY QstGetControllerConfiguration
(
    IN  int                                     ControllerIndex,
    OUT QST_FUNCTION                            *pControllerFunction
);

BOOL APIENTRY QstGetControllerState
(
    IN  int                                     ControllerIndex,
    OUT QST_HEALTH                              *pHealthState,
    OUT QST_CONTROL_STATE                       *pControlState
);

BOOL APIENTRY QstGetControllerDutyCycle
(
    IN  int                                     ControllerIndex,
    OUT float                                   *pControllerDuty
);

BOOL APIENTRY QstGetPollingInterval
(
    OUT DWORD                                   *pPollingInterval
);

BOOL APIENTRY QstSetPollingInterval
(
    IN  DWORD                                   PollingInterval
);

BOOL APIENTRY QstPollingIntervalChanged
(
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

/****************************************************************************/
/* Initialization functions                                                 */
/****************************************************************************/

#if defined(DYNAMIC_DLL_LOADING) || defined(_DOS) || defined(__DOS__) || defined(MSDOS)
BOOL QstInstInitialize( void );
void QstInstCleanup( void );
#endif

#if defined(_WIN32) || defined(__WIN32__)
/****************************************************************************/
/* Function Prototypes for explicit Windows DLL loading                     */
/****************************************************************************/

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_COUNT)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    OUT int                                     *pSensorCount
);

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_CONFIGURATION)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT QST_FUNCTION                            *pSensorFunction,
    OUT BOOL                                    *pRelativeReadings,
    OUT float                                   *pNominalReading
);

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_THRESHOLDS_HIGH)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pThresholdNonCritical,
    OUT float                                   *pThresholdCritical,
    OUT float                                   *pThresholdNonRecoverable
);

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_THRESHOLDS_LOW)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pThresholdNonCritical,
    OUT float                                   *pThresholdCritical,
    OUT float                                   *pThresholdNonRecoverable
);

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_HEALTH)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT QST_HEALTH                              *pSensorHealth
);

typedef BOOL (APIENTRY *PFN_QST_GET_SENSOR_READING)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    OUT float                                   *pSensorReading
);

typedef BOOL (APIENTRY *PFN_QST_SET_SENSOR_THRESHOLDS_HIGH)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  float                                   ThresholdNonCritical,
    IN  float                                   ThresholdCritical,
    IN  float                                   ThresholdNonRecoverable
);

typedef BOOL (APIENTRY *PFN_QST_SENSOR_THRESHOLDS_HIGH_CHANGED)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

typedef BOOL (APIENTRY *PFN_QST_SET_SENSOR_THRESHOLDS_LOW)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  float                                   ThresholdNonCritical,
    IN  float                                   ThresholdCritical,
    IN  float                                   ThresholdNonRecoverable
);

typedef BOOL (APIENTRY *PFN_QST_SENSOR_THRESHOLDS_LOW_CHANGED)
(
    IN  QST_SENSOR_TYPE                         SensorType,
    IN  int                                     SensorIndex,
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

typedef BOOL (APIENTRY *PFN_QST_GET_CONTROLLER_COUNT)
(
    OUT int                                     *pControllerCount
);

typedef BOOL (APIENTRY *PFN_QST_GET_CONTROLLER_CONFIGURATION)
(
    IN  int                                     ControllerIndex,
    OUT QST_FUNCTION                            *pControllerFunction
);

typedef BOOL (APIENTRY *PFN_QST_GET_CONTROLLER_STATE)
(
    IN  int                                     ControllerIndex,
    OUT QST_HEALTH                              *pHealthState,
    OUT QST_CONTROL_STATE                       *pControlState
);

typedef BOOL (APIENTRY *PFN_QST_GET_CONTROLLER_DUTY_CYCLE)
(
    IN  int                                     ControllerIndex,
    OUT float                                   *pControllerDuty
);

typedef BOOL (APIENTRY *PFN_QST_GET_POLLING_INTERVAL)
(
    OUT DWORD                                   *pPollingInterval
);

typedef BOOL (APIENTRY *PFN_QST_SET_POLLING_INTERVAL)
(
    IN  DWORD                                   PollingInterval
);

typedef BOOL (APIENTRY *PFN_QST_POLLING_INTERVAL_CHANGED)
(
    IN  QST_TIME_T                              LastUpdate,
    OUT BOOL                                    *pUpdated
);

/****************************************************************************/
/* Function Ordinals and definitions for explicit DLL loading               */
/****************************************************************************/

#define QST_INST_DLL                            __TEXT("QstInst.dll")

#define QST_ORD_GET_SENSOR_COUNT                1
#define QST_ORD_GET_SENSOR_CONFIGURATION        2
#define QST_ORD_GET_SENSOR_THRESHOLDS_HIGH      3
#define QST_ORD_GET_SENSOR_THRESHOLDS_LOW       4
#define QST_ORD_GET_SENSOR_HEALTH               5
#define QST_ORD_GET_SENSOR_READING              6
#define QST_ORD_SET_SENSOR_THRESHOLDS_HIGH      7
#define QST_ORD_SENSOR_THRESHOLDS_HIGH_CHANGED  8
#define QST_ORD_SET_SENSOR_THRESHOLDS_LOW       9
#define QST_ORD_SENSOR_THRESHOLDS_LOW_CHANGED   10
#define QST_ORD_GET_CONTROLLER_COUNT            11
#define QST_ORD_GET_CONTROLLER_CONFIGURATION    12
#define QST_ORD_GET_CONTROLLER_STATE            13
#define QST_ORD_GET_CONTROLLER_DUTY_CYCLE       14
#define QST_ORD_GET_POLLING_INTERVAL            15
#define QST_ORD_SET_POLLING_INTERVAL            16
#define QST_ORD_POLLING_INTERVAL_CHANGED        17

#endif // defined(_WIN32) || defined(__WIN32__)

#pragma pack()

#ifdef __cplusplus
}
#endif

#endif // !defined(_QSTINST_H)

