/****************************************************************************/
/*                                                                          */
/*  Module:         StatTest.c                                              */
/*                                                                          */
/*  Description:    Implements sample program StatTest, which demonstrates  */
/*                  how  to  extract  and   display   status   information  */
/*                  from/about  the Intel(R) Quiet System Technology (QST)  */
/*                  Subsystem.                                              */
/*                                                                          */
/*  Notes:      1.  The  source  code  contains  conditional  support  for  */
/*                  building  executables  for  DOS, Windows and Linux. In  */
/*                  the case  of  Windows,  conditional  support  is  also  */
/*                  provided  for  static  (loadtime) or dynamic (runtime)  */
/*                  binding of  the  Communications  DLL.  Currently,  the  */
/*                  Visual  Studio  project for this program is set up for  */
/*                  dynamic binding. To utilize static binding, modify the  */
/*                  project to add library  ..\..\Bin\Windows\QstComm.lib,  */
/*                  remove  module  ..\Support\QstInst.c  and  also remove  */
/*                  definition DYNAMIC_DLL_LOADING.                         */
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
#include <errno.h>
#include <ctype.h>

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#include <windows.h>
#include <tchar.h>
#endif

#include "QstCfg.h"
#include "QstCmd.h"
#include "QstComm.h"

#include "AccessQst.h"
#include "UsageStr.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

static const char * const pszStatus[] =
{
   "Normal",
   "Non-Critical",
   "Critical",
   "Non-Recoverable"
};

static const char * const pszEntityType[] =
{
   "Payload Header",
   "TemperatureMonitor",
   "FanMonitor",
   "VoltageMonitor",
   "CurrentMonitor",
   "TemperatureResponse",
   "FanController",
   "Unknown"
};

static const char * const pszTempMonField[] =
{
   "Enabled",
   "Usage",
   "DeviceAddress",
   "GetReadingCommand",
   "RelativeReadings",
   "RelativeConversionFactor",
   "AccuracyCorrectionSlope",
   "AccuracyCorrectionOffset",
   "TimeoutNonCritical",
   "TimeoutCritical",
   "TimeoutNonRecoverable",
   "TemperatureNominal",
   "TemperatureNonCritical",
   "TemperatureCritical",
   "TemperatureNonRecoverable"
};

#define MAX_TEMP_MON_FIELD 15

static const char * const pszFanMonField[] =
{
   "Enabled",
   "Usage",
   "DeviceAddress",
   "GetAttributesCommand",
   "GetReadingCommand",
   "FanSpeedNominal",
   "FanSpeedNonCritical",
   "FanSpeedCritical",
   "FanSpeedNonRecoverable"
};

#define MAX_FAN_MON_FIELD 9

static const char * const pszVoltMonField[] =
{
   "Enabled",
   "Usage",
   "DeviceAddress",
   "GetReadingCommand",
   "AccuracyCorrectionSlope",
   "AccuracyCorrectionOffset",
   "VoltageNominal",
   "UnderVoltageNonCritical",
   "UnderVoltageCritical",
   "UnderVoltageNonRecoverable",
   "OverVoltageNonCritical",
   "OverVoltageCritical",
   "OverVoltageNonRecoverable"
};

#define MAX_VOLT_MON_FIELD 13

static const char * const pszCurrMonField[] =
{
   "Enabled",
   "Usage",
   "SensorType",
   "DeviceAddress",
   "GetReadingCommand",
   "AdjustmentSlope",
   "AdjustmentOffset",
   "NominalCurrent",
   "UnderCurrentNonCritical",
   "UnderCurrentCritical",
   "UnderCurrentNonRecoverable",
   "OverCurrentNonCritical",
   "OverCurrentCritical",
   "OverCurrentNonRecoverable"
};

#define MAX_CURR_MON_FIELD 14

static const char * const pszTempRspField[] =
{
   "Enabled",
   "Usage",
   "TemperatureMonitor",
   "SmoothingWindow",
   "TemperatureLimit",
   "ProportionalGain",
   "IntegralGain",
   "DerivativeGain",
   "IntegralTimeWindow",
   "DerivativeTimeWindow",
   "AllOnTemperature",
};

#define MAX_TEMP_RSP_FIELD 17

static const char * const pszFanCtrlField[] =
{
   "Enabled",
   "Usage",
   "DeviceAddress",
   "GetAttributesCommand",
   "SetConfigurationCommand",
   "SetSpeedCommand",
   "GetSpeedCommand",
   "AssociatedFanSpeedMonitor1",
   "Fan1SetConfigurationCommand",
   "Fan1PulsesPerRevolution",
   "Fan1MinimumRPMRangeLow",
   "Fan1MinimumRPMRangeHigh",
   "Fan1DependentMeasurement",
   "AssociatedFanSpeedMonitor2",
   "Fan2SetConfigurationCommand",
   "Fan2PulsesPerRevolution",
   "Fan1MinimumRPMRangeLow",
   "Fan1MinimumRPMRangeHigh",
   "Fan2DependentMeasurement",
   "AssociatedFanSpeedMonitor3",
   "Fan3SetConfigurationCommand",
   "Fan3PulsesPerRevolution",
   "Fan1MinimumRPMRangeLow",
   "Fan1MinimumRPMRangeHigh",
   "Fan3DependentMeasurement",
   "AssociatedFanSpeedMonitor4",
   "Fan4SetConfigurationCommand",
   "Fan4PulsesPerRevolution",
   "Fan1MinimumRPMRangeLow",
   "Fan1MinimumRPMRangeHigh",
   "Fan4DependentMeasurement",
   "PhysicalControllerIndex",
   "MinOffMode",
   "DutyCycleMin",
   "DutyCycleOn",
   "AmbientFloorTemperatureMonitor",
   "AmbientFloorDutyCycleRange",
   "AmbientFloorTemperatureMinimum",
   "AmbientFloorTemperatureRange",
   "DutyCycleMax",
   "AmbientCeilingTemperatureMonitor",
   "AmbientCeilingDutyCycleRange",
   "AmbientCeilingTemperatureMinimum",
   "AmbientCeilingTemperatureRange",
   "SignalInvert",
   "SignalFrequency",
   "SpinUpTime",
   "TempResponse1Weighting",
   "TempResponse2Weighting",
   "TempResponse3Weighting",
   "TempResponse4Weighting",
   "TempResponse5Weighting",
   "TempResponse6Weighting",
   "TempResponse7Weighting",
   "TempResponse8Weighting",
   "TempResponse9Weighting",
   "TempResponse10Weighting",
   "TempResponse11Weighting",
   "TempResponse12Weighting",
   "TempResponse13Weighting",
   "TempResponse14Weighting",
   "TempResponse15Weighting",
   "TempResponse16Weighting",
   "TempResponse17Weighting",
   "TempResponse18Weighting",
   "TempResponse19Weighting",
   "TempResponse20Weighting",
   "TempResponse21Weighting",
   "TempResponse22Weighting",
   "TempResponse23Weighting",
   "TempResponse24Weighting",
   "TempResponse25Weighting",
   "TempResponse26Weighting",
   "TempResponse27Weighting",
   "TempResponse28Weighting",
   "TempResponse29Weighting",
   "TempResponse30Weighting",
   "TempResponse31Weighting",
   "TempResponse32Weighting"
};

#define MAX_FAN_CTRL_FIELD 79

/****************************************************************************/
/* Module Variables                                                         */
/****************************************************************************/

static QST_GENERIC_CMD                      stGenericCmd;
static QST_GET_SUBSYSTEM_STATUS_RSP         stStatusRsp;
static BOOL                                 bConfig = FALSE;
static BOOL                                 bQstCommInit = FALSE;

/****************************************************************************/
/* DisplayError() - Displays an error message. Has support for the embedded */
/* QST error codes that are generated by the AccessQst module.              */
/****************************************************************************/

typedef enum
{
   QST_ERROR,
   WINDOWS_ERROR,
   ERRNO_ERROR,
   PICK_ERROR,

} E_ERROR_TYPE;

static const char * const pszQstError[] =
{
   "QST Successful",
   "QST Rejected Command: Command Unsupported",
   "QST Rejected Command: Capability Locked",
   "QST Rejected Command: Invalid Parameter",
   "QST Rejected Command: Invalid Version",
   "QST Command Execution Failed: Communication Error",
   "QST Command Execution Failed: Sensor Error",
   "QST Command Execution Failed: No Memory",
   "QST Command Execution Failed: No Resources",
   "QST Rejected Command: Invalid In This Context",
   "QST Rejected Command: Improper Command Size",
   "QST Rejected Command: Improper Response Size",
   "Unknown Error Code"
};

static void DisplayError( char *pszMessage, E_ERROR_TYPE eError, DWORD dwError )
{
   printf( "\n*** %s!!\n", pszMessage );

   switch( eError )
   {
   case QST_ERROR:

      if( IS_QST_ERROR( dwError ) )
      {
         if( dwError == ERROR_QST_NOT_CONFIGURED )
         {
            puts( "   QST Error = 0x0100 (Not Configured)" );
            break;
         }

         dwError -= ERROR_QST_BASE;
      }

      if( dwError <= QST_CMD_REJECTED_RSP_SIZE )
         printf( "   QST Error = 0x%04X (%s)\n\n", dwError, pszQstError[dwError] );
      else if( dwError )
         printf( "   QST Error = 0x%04X (Unknown Error)\n\n", dwError );

      break;

   case ERRNO_ERROR:

      if( errno )
         printf( "   ERRNO = %d (%s)\n\n", errno, strerror(errno) );

      break;

   case PICK_ERROR:

#if defined(__MSDOS__) || defined(__LINUX__) || defined(__SOLARIS__)

      if( errno )
      {
         if( IS_QST_ERROR( errno ) )
         {
            if( errno <= ERROR_QST_IMPROPER_RESPONSE_SIZE )
               printf( "   QST Error = 0x%04X (%s)\n", errno - ERROR_QST_BASE, pszQstError[errno - ERROR_QST_BASE] );
            else if( errno == ERROR_QST_NOT_CONFIGURED )
               puts(   "   QST Error = 0x0100 (Not Configured)" );
            else
               printf( "   QST Error = 0x%04X (Unknown Error)\n", errno - ERROR_QST_BASE );
         }
         else
            printf( "   ERRNO = %d (%s)\n\n", errno, strerror(errno) );
      }

      break;

#elif defined(__WIN32__)

      dwError = GetLastError();

   case WINDOWS_ERROR:

      if( dwError )
      {
         if( IS_QST_ERROR( dwError ) )
         {
            if( dwError <= ERROR_QST_IMPROPER_RESPONSE_SIZE )
               printf( "   QST Error = 0x%04X (%s)\n", dwError - ERROR_QST_BASE, pszQstError[dwError - ERROR_QST_BASE] );
            else if( dwError == ERROR_QST_NOT_CONFIGURED )
               printf( "   QST Error = 0x%04X (Not Configured)\n", dwError - ERROR_QST_BASE );
            else
               printf( "   QST Error = 0x%04X (Unknown Error)\n", dwError - ERROR_QST_BASE );
         }
         else
         {
            char   *pszMsgBuf;
            size_t tLen;

            FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszMsgBuf, 0, NULL );

            tLen = strlen(pszMsgBuf);

            if( tLen > 2 )
            {
               if( pszMsgBuf[tLen - 2] == '\r' )
                  pszMsgBuf[tLen - 2] = '\0';

               else if( pszMsgBuf[tLen - 1] == '\n' )
                  pszMsgBuf[tLen - 1] = '\0';

               printf( "   Windows Error = 0x%08X (%s)\n", dwError, pszMsgBuf );
            }
            else
               printf( "   Windows Error = 0x%08X (Unknown Error)\n", dwError );

            LocalFree( pszMsgBuf );
         }
      }

      break;

#endif

   }
}

/****************************************************************************/
/* DisplayTempSensors() - Displays Temperature Sensor Information           */
/****************************************************************************/

static BOOL DisplayTempSensors( void )
{
   int     iIndex, iUsage, iStatus, iTemps = GetTempCountQst();
   float   fValue;
   THRESH  stThresh;

   for( iIndex = 0; iIndex < iTemps; iIndex++ )
   {
      if( (iStatus = GetTempHealthQst( iIndex )) != -1 )
      {
         if( (iUsage = GetTempUsageQst( iIndex )) != -1 )
         {
            if( (fValue  = GetTempReadingQst( iIndex )) != -1 )
            {
               if( GetTempThreshQst( iIndex, &stThresh ) )
               {
                  printf( "   Temperature Sensor %d:\n\n", GetTempIndexQst( iIndex ) + 1 );

                  printf( "      Health:            %s\n", pszStatus[iStatus] );
                  printf( "      Usage:             %s\n", GetTempUsageStr( iUsage ) );
                  printf( "      Reading:           %.2f\n\n", fValue );

                  printf( "      NonCrit:           %3.3f\n",   stThresh.fNonCrit );
                  printf( "      Crit:              %3.3f\n",   stThresh.fCrit );
                  printf( "      NonRecov:          %3.3f\n\n", stThresh.fNonRecov );

                  continue;
               }
            }
         }
      }

      DisplayError( "Unable to obtain Temperature Sensor information", PICK_ERROR, 0 );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* DisplayFanSensors() - Displays Fan Speed Sensor Information              */
/****************************************************************************/

static BOOL DisplayFanSensors( void )
{
   int     iIndex, iUsage, iStatus, iFans = GetFanCountQst();
   float   fValue;
   THRESH  stThresh;

   for( iIndex = 0; iIndex < iFans; iIndex++ )
   {
      if( (iStatus = GetFanHealthQst( iIndex )) != -1 )
      {
         if( (iUsage = GetFanUsageQst( iIndex )) != -1 )
         {
            if( (fValue = GetFanReadingQst( iIndex )) != -1 )
            {
               if( GetFanThreshQst( iIndex, &stThresh ) )
               {
                  printf( "   Fan Speed Sensor %d:\n\n", GetFanIndexQst( iIndex ) + 1 );

                  printf( "      Health:            %s\n", pszStatus[iStatus] );
                  printf( "      Usage:             %s\n", GetFanUsageStr( iUsage ) );
                  printf( "      Reading:           %d\n\n", (int)fValue );

                  printf( "      NonCrit:           %3.3f\n",   stThresh.fNonCrit );
                  printf( "      Crit:              %3.3f\n",   stThresh.fCrit );
                  printf( "      NonRecov:          %3.3f\n\n", stThresh.fNonRecov );

                  continue;
               }
            }
         }
      }

      DisplayError( "Unable to obtain Fan Speed Sensor information", PICK_ERROR, 0 );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* DisplayVoltSensors() - Displays Voltage Sensor Information               */
/****************************************************************************/

static BOOL DisplayVoltSensors( void )
{
   int     iIndex, iUsage, iStatus, iVolts = GetVoltCountQst();
   float   fValue;
   THRESH  stThreshLow, stThreshHigh;

   for( iIndex = 0; iIndex < iVolts; iIndex++ )
   {
      if( (iStatus = GetVoltHealthQst( iIndex )) != -1 )
      {
         if( (iUsage = GetVoltUsageQst( iIndex )) != -1 )
         {
            if( (fValue = GetVoltReadingQst( iIndex )) != -1 )
            {
               if( GetVoltThreshQst( iIndex, &stThreshLow, &stThreshHigh ) )
               {
                  printf( "   Voltage Sensor %d:\n\n", GetVoltIndexQst( iIndex ) + 1 );

                  printf( "      Health:            %s\n", pszStatus[iStatus] );
                  printf( "      Usage:             %s\n", GetVoltUsageStr( iUsage ) );
                  printf( "      Reading:           %.3f\n\n", fValue );

                  printf( "      NonCrit Low:       %2.3f\n",   stThreshLow.fNonCrit );
                  printf( "      Crit Low:          %2.3f\n",   stThreshLow.fCrit );
                  printf( "      NonRecov Low:      %2.3f\n\n", stThreshLow.fNonRecov );

                  printf( "      NonCrit High:      %2.3f\n",   stThreshHigh.fNonCrit );
                  printf( "      Crit High:         %2.3f\n",   stThreshHigh.fCrit );
                  printf( "      NonRecov High:     %2.3f\n\n", stThreshHigh.fNonRecov );

                  continue;
               }
            }
         }
      }

      DisplayError( "Unable to obtain Voltage Sensor information", PICK_ERROR, 0 );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* DisplayCurrSensors() - Displays Current Sensor Information               */
/****************************************************************************/

static BOOL DisplayCurrSensors( void )
{
   int     iIndex, iUsage, iStatus, iCurrs = GetCurrCountQst();
   float   fValue;
   THRESH  stThreshLow, stThreshHigh;

   for( iIndex = 0; iIndex < iCurrs; iIndex++ )
   {
      if( (iStatus = GetCurrHealthQst( iIndex )) != -1 )
      {
         if( (iUsage = GetCurrUsageQst( iIndex )) != -1 )
         {
            if( (fValue = GetCurrReadingQst( iIndex )) != -1 )
            {
               if( GetCurrThreshQst( iIndex, &stThreshLow, &stThreshHigh ) )
               {
                  printf( "   Current Sensor %d:\n\n", GetCurrIndexQst( iIndex ) + 1 );

                  printf( "      Health:            %s\n",     pszStatus[iStatus] );
                  printf( "      Usage:             %s\n",     GetCurrUsageStr( iUsage ) );
                  printf( "      Reading:           %.3f\n\n", fValue );

                  printf( "      NonCrit Low:       %.3f\n",   stThreshLow.fNonCrit );
                  printf( "      Crit Low:          %.3f\n",   stThreshLow.fCrit );
                  printf( "      NonRecov Low:      %.3f\n\n", stThreshLow.fNonRecov );

                  printf( "      NonCrit High:      %.3f\n",   stThreshHigh.fNonCrit );
                  printf( "      Crit High:         %.3f\n",   stThreshHigh.fCrit );
                  printf( "      NonRecov High:     %.3f\n\n", stThreshHigh.fNonRecov );

                  continue;
               }
            }
         }
      }

      DisplayError( "Unable to obtain Current Sensor information", PICK_ERROR, 0 );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* DisplayFanControllers() - Displays Fan Speed Controller Information      */
/****************************************************************************/

static BOOL DisplayFanControllers( void )
{
   int     iIndex, iUsage, iStatus, iCtrls = GetDutyCountQst();
   float   fValue;
   BOOL    bSWControl;

   for( iIndex = 0; iIndex < iCtrls; iIndex++ )
   {
      if( (iStatus = GetDutyHealthQst( iIndex )) != -1 )
      {
         if( (iUsage = GetDutyUsageQst( iIndex )) != -1 )
         {
            if( (fValue = GetDutySettingQst( iIndex )) != -1 )
            {
               if( GetDutyManualQst( iIndex, &bSWControl ) )
               {
                  printf( "   Fan Speed Controller %d:\n\n", GetDutyIndexQst( iIndex ) + 1 );

                  printf( "      Health:            %s\n", pszStatus[iStatus] );
                  printf( "      Usage:             %s\n", GetCtrlUsageStr( iUsage ) );
                  printf( "      Control:           %s\n", bSWControl? "Manual" : "Automatic" );
                  printf( "      Duty Cycle:        %.2f\n\n", fValue );

                  continue;
               }
            }
         }
      }

      DisplayError( "Unable to obtain Fan Speed Controller information", PICK_ERROR, 0 );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* DisplaySubsystemStatus() - Obtains status summary from the QST Sub-      */
/* system and details its contents                                          */
/****************************************************************************/

static BOOL DisplaySubsystemStatus( void )
{
   // Get Subsystem Status Information

   stGenericCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_STATUS;
   stGenericCmd.stHeader.byEntity        = 0;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_STATUS_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stStatusRsp, sizeof(QST_GET_SUBSYSTEM_STATUS_RSP) ) )
   {
      DisplayError( "Unable to communicate with the QST Subsystem", PICK_ERROR, 0 );
      return( FALSE );
   }

   if( stStatusRsp.byStatus )
   {
      DisplayError( "Unable to obtain status from QST Subsystem", QST_ERROR, stStatusRsp.byStatus );
      return( FALSE );
   }

   // If configured, display Subsystem status

   if( stStatusRsp.stSubsystemStatus.bSubsystemConfigured )
   {
      bConfig = TRUE;
      puts( "The QST Subsystem is configured and operational" );

      if( stStatusRsp.stSubsystemStatus.bOverrideFullError )
         puts( "\n   All fans overridden to full due to temperature sensor error!" );

      if( stStatusRsp.stSubsystemStatus.bOverrideFullCritical )
         puts( "\n   All fans overridden to full due to critical temperature reading!" );

      if( stStatusRsp.stSubsystemStatus.bOverrideFullFailure )
         puts( "\n   All fans overridden to full due to fan speed controller failure!" );
   }

   // Display Lock Mask

   puts( "\nCurrent Lock Mask:\n" );

   printf( "   Configuration:        %socked\n", stStatusRsp.stLockMask.bLockConfiguration?    "L" : "Unl" );
   printf( "   Health Thresholds:    %socked\n", stStatusRsp.stLockMask.bLockThresholds?       "L" : "Unl" );
   printf( "   Manual Fan Control:   %socked\n", stStatusRsp.stLockMask.bLockManualFanControl? "L" : "Unl" );

   printf( "   SST Bus Resources:    Read-%s\n", stStatusRsp.stLockMask.bLockSSTBusAccess?      "Only" : "Write" );
   printf( "   Chipset Resources:    Read-%s\n", stStatusRsp.stLockMask.bLockChipsetAccess?     "Only" : "Write" );

   // Display Configuration Attempt information

   if( stStatusRsp.stConfigStatus.bConfigSuccessful )
   {
      if( stStatusRsp.stConfigStatus.uFailingEntityType != QST_VALUE_NONE_UINT3 )
      {
         char *pszStr = "Unknown";

         puts( "\nUnsuccessful configuration attempt recorded:\n" );

         printf( "   Failing Entity:    %s%d\n", pszEntityType[stStatusRsp.stConfigStatus.uFailingEntityType],
                 stStatusRsp.stConfigStatus.uFailingEntityIndex + 1 );

         switch( stStatusRsp.stConfigStatus.uFailingEntityType )
         {
         case QST_TEMP_MONITOR:

            if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_TEMP_MON_FIELD )
               pszStr = (char *)pszTempMonField[stStatusRsp.stConfigStatus.uFailingEntityParam];

            break;

         case QST_FAN_MONITOR:

             if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_FAN_MON_FIELD )
                pszStr = (char *)pszFanMonField[stStatusRsp.stConfigStatus.uFailingEntityParam];

             break;

         case QST_VOLT_MONITOR:

             if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_VOLT_MON_FIELD )
                pszStr = (char *)pszVoltMonField[stStatusRsp.stConfigStatus.uFailingEntityParam];

             break;

         case QST_CURR_MONITOR:

             if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_CURR_MON_FIELD )
                pszStr = (char *)pszCurrMonField[stStatusRsp.stConfigStatus.uFailingEntityParam];

             break;

         case QST_TEMP_RESPONSE:

             if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_TEMP_RSP_FIELD )
                pszStr = (char *)pszTempRspField[stStatusRsp.stConfigStatus.uFailingEntityParam];

             break;

         case QST_FAN_CONTROLLER:

             if( stStatusRsp.stConfigStatus.uFailingEntityParam < MAX_FAN_CTRL_FIELD )
                pszStr = (char *)pszFanCtrlField[stStatusRsp.stConfigStatus.uFailingEntityParam];

             break;
         }

         printf( "   Failing Parameter: %s\n\n", pszStr );
      }
   }

   return( TRUE );
}

/****************************************************************************/
/* main() - Mainline for program                                            */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
{
   puts( "\nIntel(R) Quiet System Technology Status Display Demo" );
   puts( "Copyright (C) 2007-2008, Intel Corporation. All Rights Reserved.\n" );

   // Initialize Sensor/Controller access

   if( !InitializeQst() )
   {

#if defined(__WIN32__)
      DWORD dwError = GetLastError();
#else
      UINT32 dwError = (UINT32)errno;
#endif

      if( dwError == ERROR_QST_NOT_CONFIGURED )
      {
         puts( "The QST Subsystem is NOT configured!" );

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

         // Initialize basic communications

         if( !QstInitialize() )
         {
            DisplayError( "Unable to communicate with QST Subsystem", PICK_ERROR, 0 );

#if defined(__WIN32__)
            dwError = GetLastError();
#else
            dwError = (UINT32)errno;
#endif

            return( (int)dwError );
         }

         bQstCommInit = TRUE;

#endif

      }
      else
      {
         DisplayError( "Unable to communicate with QST Subsystem", PICK_ERROR, 0 );
         return( (int)dwError );
      }
   }

   // Display Subsystem Status Summary

   if( !DisplaySubsystemStatus() )      // Note: sets bConfig
      return( 1 );

   // Display information about configured Sensors/Controllers

   if( bConfig )
   {
      puts( "\nSensor Configuration/Status Summary:\n" );

      DisplayTempSensors();
      DisplayFanSensors();
      DisplayVoltSensors();
      DisplayCurrSensors();

      puts( "Fan Speed Controller Configuration/Status Summary:\n" );

      DisplayFanControllers();
   }

   // We're done!

   puts( "\nEnd of Report\n" );

   // Cleanup

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

   if( bQstCommInit )
      QstCleanup();
   else

#endif

      CleanupQst();

   return( 0 );

#if _MSC_VER > 1000
   UNREFERENCED_PARAMETER( iArgs );
   UNREFERENCED_PARAMETER( pszArg );
#endif

}
