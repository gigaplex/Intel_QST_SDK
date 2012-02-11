/****************************************************************************/
/*                                                                          */
/*  Module:         AccessQst.c                                             */
/*                                                                          */
/*  Description:    Provides the functions that are used to  obtain/update  */
/*                  readings/duty   cycle  settings  from/to  the  various  */
/*                  sensors/controllers, through direct query of the  QST   */
/*                  Subsystem.                                              */
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
#include <ctype.h>
#include <errno.h>
#include <time.h>

#if defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif

#include "AccessQst.h"
#include "QstComm.h"
#include "QstCmd.h"

#ifdef BE_THREAD_SAFE
#include "CritSect.h"
#define CRIT_SECT_TYPE 0xAF5C040
#endif

/****************************************************************************/
/****************************************************************************/
/*************************** Miscellaneous Support **************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Global Variables                                                         */
/****************************************************************************/

#ifdef BE_THREAD_SAFE
static HCRITSECT hCritSect;     // handle for critical section operator
#endif

/****************************************************************************/
/* SetError() - Places specified error code in appropriate error variable   */
/****************************************************************************/

static void SetError( int iError )
{

#ifdef __WIN32__
   SetLastError( (DWORD)iError );
#else
   errno = iError;
#endif

}

/****************************************************************************/
/* SetQSTError() - Places specified QST error code in appropriate error     */
/* variable.                                                                */
/****************************************************************************/

static void SetQSTError( UINT8 byStatus )
{

#ifdef __WIN32__
   SetLastError( QST_STATUS_TO_ERROR( byStatus ) );
#else
   errno = QST_STATUS_TO_ERRNO( byStatus );
#endif

}

/****************************************************************************/
/* EnterCritical() - Enters critical section (if necessary)                 */
/****************************************************************************/

static void EnterCritical( void )
{

#ifdef BE_THREAD_SAFE
   EnterCritSect( hCritSect );
#endif

   return;
}

/****************************************************************************/
/* ExitCritical() - Exits from the critical section (if necessary). The     */
/* contents of the persistent error variable are protected across this      */
/* operation.                                                               */
/****************************************************************************/

static void ExitCritical( void )
{

#ifdef BE_THREAD_SAFE

#ifdef __WIN32__

   DWORD dwError = GetLastError();
   LeaveCritSect( hCritSect );
   SetLastError( dwError );

#else

   int iError = errno;
   LeaveCritSect( hCritSect );
   errno = iError;

#endif

#endif

   return;
}





/****************************************************************************/
/****************************************************************************/
/************************ Temperature Monitor Support ***********************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                              iTempMons = 0;
static int                              iTempMonIndex[QST_ABS_TEMP_MONITORS];
static QST_GET_TEMP_MON_CONFIG_RSP      stTempMonConfigRsp[QST_ABS_TEMP_MONITORS];
static QST_GET_TEMP_MON_UPDATE_RSP      stTempMonUpdateRsp;
static time_t                           tLastTempMonUpdate = 0;

/****************************************************************************/
/* GetTempMonConfig() - Get configuration for temperature monitor           */
/****************************************************************************/

static BOOL GetTempMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD stGenericCmd;

   // Send the Temperature Monitor Configuration request

   stGenericCmd.stHeader.byCommand       = QST_GET_TEMP_MON_CONFIG;
   stGenericCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_TEMP_MON_CONFIG_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stTempMonConfigRsp[iLocSensor], sizeof(QST_GET_TEMP_MON_CONFIG_RSP) ) )
      return( FALSE );

   // Can't go any further if Subsystem rejected the command

   if( stTempMonConfigRsp[iLocSensor].byStatus )
   {
      SetQSTError( stTempMonConfigRsp[iLocSensor].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetTempMonUpdate() - Gets updated readings from temperature monitors     */
/****************************************************************************/

static BOOL GetTempMonUpdate( void )
{
   EnterCritical();

   // Only update if readings are at least a second old

   if( tLastTempMonUpdate < time( NULL ) )
   {
      QST_GENERIC_CMD stGenericCmd;

      // Send the Temperature Monitor Update request

      stGenericCmd.stHeader.byCommand       = QST_GET_TEMP_MON_UPDATE;
      stGenericCmd.stHeader.byEntity        = 0;
      stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_TEMP_MON_UPDATE_RSP);

      if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stTempMonUpdateRsp, sizeof(QST_GET_TEMP_MON_UPDATE_RSP) ) )
      {
         ExitCritical();
         return( FALSE );
      }

      // Can't go any further if Subsystem rejected the command

      if( stTempMonUpdateRsp.byStatus )
      {
         SetQSTError( stTempMonUpdateRsp.byStatus );
         ExitCritical();
         return( FALSE );
      }

      // Save the time of the update for later comparison

      tLastTempMonUpdate = time( NULL );
   }

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* GetTempCountQst() - Returns number of enabled temperature monitors       */
/****************************************************************************/

int GetTempCountQst( void )
{
   return( iTempMons );
}

/****************************************************************************/
/* GetTempIndexQst() - Returns physical index for temperature monitor       */
/****************************************************************************/

int GetTempIndexQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( iTempMonIndex[iLocSensor] );
}

/****************************************************************************/
/* GetTempUsageQst() - Returns usage indicator for temperature monitor      */
/****************************************************************************/

int GetTempUsageQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( (int)stTempMonConfigRsp[iLocSensor].byMonitorUsage );
}

/****************************************************************************/
/* GetTempReadingQst() - Returns reading from temperature monitor           */
/****************************************************************************/

float GetTempReadingQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetTempMonUpdate() )
      return( -1 );

   return( QST_TEMP_TO_FLOAT( stTempMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].lfCurrentReading ) );
}

/****************************************************************************/
/* GetTempHealthQst() - Returns health for temperature monitor              */
/****************************************************************************/

int GetTempHealthQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetTempMonUpdate() )
      return( -1 );

   // Give precedence to monitor status over health status

   if( stTempMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus )
      return( stTempMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus );
   else
      return( stTempMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].stMonitorStatus.uThresholdStatus );
}

/****************************************************************************/
/* GetTempHealthByteQst() - Returns health byte for temperature monitor     */
/****************************************************************************/

BOOL GetTempHealthByteQst( int iLocSensor, QST_MON_HEALTH_STATUS *pstStatus )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetTempMonUpdate() )
      return( FALSE );

   *((BYTE *)pstStatus) = *((BYTE *)&stTempMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].stMonitorStatus);
   return( TRUE );
}

/****************************************************************************/
/* GetTempThreshQst() - Returns health thresholds for temperature monitor   */
/****************************************************************************/

BOOL GetTempThreshQst( int iLocSensor, THRESH *pstThresh )
{
   if( (iLocSensor < 0) || (iLocSensor >= iTempMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   pstThresh->fNonCrit  = QST_TEMP_TO_FLOAT( stTempMonConfigRsp[iLocSensor].lfTempNonCritical    );
   pstThresh->fCrit     = QST_TEMP_TO_FLOAT( stTempMonConfigRsp[iLocSensor].lfTempCritical       );
   pstThresh->fNonRecov = QST_TEMP_TO_FLOAT( stTempMonConfigRsp[iLocSensor].lfTempNonRecoverable );

   return( TRUE );
}




/****************************************************************************/
/****************************************************************************/
/************************* Fan Speed Monitor Support ************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                              iFanMons = 0;
static int                              iFanMonIndex[QST_ABS_FAN_MONITORS];
static QST_GET_FAN_MON_CONFIG_RSP       stFanMonConfigRsp[QST_ABS_FAN_MONITORS];
static QST_GET_FAN_MON_UPDATE_RSP       stFanMonUpdateRsp;
static time_t                           tLastFanMonUpdate = 0;

/****************************************************************************/
/* GetFanMonConfig() - Get configuration for fan speed monitor              */
/****************************************************************************/

static BOOL GetFanMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD stGenericCmd;

   // Set Fan Speed Monitor Configuration command

   stGenericCmd.stHeader.byCommand       = QST_GET_FAN_MON_CONFIG;
   stGenericCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_MON_CONFIG_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stFanMonConfigRsp[iLocSensor], sizeof(QST_GET_FAN_MON_CONFIG_RSP) ) )
      return( FALSE );

   // Can't go any further if Subsystem rejected the command

   if( stFanMonConfigRsp[iLocSensor].byStatus )
   {
      SetQSTError( stFanMonConfigRsp[iLocSensor].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetFanMonUpdate() - Gets updated readings from fan monitors              */
/****************************************************************************/

static BOOL GetFanMonUpdate( void )
{
   EnterCritical();

   // Only update if readings are at least a second old

   if( tLastFanMonUpdate < time( NULL ) )
   {
      QST_GENERIC_CMD stGenericCmd;

      // Send the Fan Speed Monitor Update request

      stGenericCmd.stHeader.byCommand       = QST_GET_FAN_MON_UPDATE;
      stGenericCmd.stHeader.byEntity        = 0;
      stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_MON_UPDATE_RSP);

      if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stFanMonUpdateRsp, sizeof(QST_GET_FAN_MON_UPDATE_RSP) ) )
      {
         ExitCritical();
         return( FALSE );
      }

      // Can't go any further if Subsystem rejected the command

      if( stFanMonUpdateRsp.byStatus )
      {
         SetQSTError( stFanMonUpdateRsp.byStatus );
         ExitCritical();
         return( FALSE );
      }

      // Save the time of the update for later comparison

      tLastFanMonUpdate = time( NULL );
   }

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* GetFanCountQst() - Returns number of enabled fan speed monitors          */
/****************************************************************************/

int GetFanCountQst( void )
{
   return( iFanMons );
}

/****************************************************************************/
/* GetFanIndexQst() - Returns physical index for fan speed monitor          */
/****************************************************************************/

int GetFanIndexQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( iFanMonIndex[iLocSensor] );
}

/****************************************************************************/
/* GetFanUsageQst() - Returns usage indicator for fan speed monitor         */
/****************************************************************************/

int GetFanUsageQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( (int)stFanMonConfigRsp[iLocSensor].byMonitorUsage );
}

/****************************************************************************/
/* GetFanReadingQst() - Returns reading from fan speed monitor              */
/****************************************************************************/

float GetFanReadingQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetFanMonUpdate() )
      return( -1 );

   return( (float)stFanMonUpdateRsp.stMonitorUpdate[iFanMonIndex[iLocSensor]].uCurrentSpeed );
}

/****************************************************************************/
/* GetFanHealthQst() - Returns health for fan speed monitor                 */
/****************************************************************************/

int GetFanHealthQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetFanMonUpdate() )
      return( -1 );

   // Give precedence to monitor status over health status

   if( stFanMonUpdateRsp.stMonitorUpdate[iFanMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus )
      return( stFanMonUpdateRsp.stMonitorUpdate[iFanMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus );
   else
      return( stFanMonUpdateRsp.stMonitorUpdate[iTempMonIndex[iLocSensor]].stMonitorStatus.uThresholdStatus );
}

/****************************************************************************/
/* GetFanHealthByteQst() - Returns health byte for fan speed monitor        */
/****************************************************************************/

BOOL GetFanHealthByteQst( int iLocSensor, QST_MON_HEALTH_STATUS *pstStatus )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetFanMonUpdate() )
      return( FALSE );

   *((BYTE *)pstStatus) = *((BYTE *)&stFanMonUpdateRsp.stMonitorUpdate[iFanMonIndex[iLocSensor]].stMonitorStatus);
   return( TRUE );
}

/****************************************************************************/
/* GetFanThreshQst () - Returns health thresholds for fan speed monitor     */
/****************************************************************************/

BOOL GetFanThreshQst( int iLocSensor, THRESH *pstThresh )
{
   if( (iLocSensor < 0) || (iLocSensor >= iFanMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   pstThresh->fNonCrit  = (float)stFanMonConfigRsp[iLocSensor].uSpeedNonCritical;
   pstThresh->fCrit     = (float)stFanMonConfigRsp[iLocSensor].uSpeedCritical;
   pstThresh->fNonRecov = (float)stFanMonConfigRsp[iLocSensor].uSpeedNonRecoverable;

   return( TRUE );
}




/****************************************************************************/
/****************************************************************************/
/************************** Voltage Monitor Support *************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                              iVoltMons = 0;
static int                              iVoltMonIndex[QST_ABS_VOLT_MONITORS];
static QST_GET_VOLT_MON_CONFIG_RSP      stVoltMonConfigRsp[QST_ABS_VOLT_MONITORS];
static QST_GET_VOLT_MON_UPDATE_RSP      stVoltMonUpdateRsp;
static time_t                           tLastVoltMonUpdate = 0;

/****************************************************************************/
/* GetVoltMonConfig() - Get configuration for voltage monitor               */
/****************************************************************************/

static BOOL GetVoltMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD stGenericCmd;

   stGenericCmd.stHeader.byCommand       = QST_GET_VOLT_MON_CONFIG;
   stGenericCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_VOLT_MON_CONFIG_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stVoltMonConfigRsp[iLocSensor], sizeof(QST_GET_VOLT_MON_CONFIG_RSP) ) )
      return( FALSE );

   if( stVoltMonConfigRsp[iLocSensor].byStatus )
   {
      SetQSTError( stVoltMonConfigRsp[iLocSensor].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetVoltMonUpdate() - Gets updated readings from voltage monitors         */
/****************************************************************************/

static BOOL GetVoltMonUpdate( void )
{
   EnterCritical();

   // Only update if readings are at least a second old

   if( tLastVoltMonUpdate < time( NULL ) )
   {
      QST_GENERIC_CMD stGenericCmd;

      // Send the Temperature Monitor Update request

      stGenericCmd.stHeader.byCommand       = QST_GET_VOLT_MON_UPDATE;
      stGenericCmd.stHeader.byEntity        = 0;
      stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_VOLT_MON_UPDATE_RSP);

      if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stVoltMonUpdateRsp, sizeof(QST_GET_VOLT_MON_UPDATE_RSP) ) )
      {
         ExitCritical();
         return( FALSE );
      }

      // Can't go any further if Subsystem rejected the command

      if( stVoltMonUpdateRsp.byStatus )
      {
         SetQSTError( stVoltMonUpdateRsp.byStatus );
         ExitCritical();
         return( FALSE );
      }

      // Save the time of the update for later comparison

      tLastVoltMonUpdate = time( NULL );
   }

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* GetVoltCountQst() - Returns number of enabled voltage monitors           */
/****************************************************************************/

int GetVoltCountQst( void )
{
   return( iVoltMons );
}

/****************************************************************************/
/* GetVoltIndexQst() - Returns physical index for voltage monitor           */
/****************************************************************************/

int GetVoltIndexQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( iVoltMonIndex[iLocSensor] );
}

/****************************************************************************/
/* GetVoltUsageQst() - Returns usage indicator for voltage monitor          */
/****************************************************************************/

int GetVoltUsageQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( (int)stVoltMonConfigRsp[iLocSensor].byMonitorUsage );
}

/****************************************************************************/
/* GetVoltReadingQst() - Returns reading from voltage monitor               */
/****************************************************************************/

float GetVoltReadingQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetVoltMonUpdate() )
      return( -1 );

   return( QST_VOLT_TO_FLOAT( stVoltMonUpdateRsp.stMonitorUpdate[iVoltMonIndex[iLocSensor]].iCurrentVoltage ) );
}

/****************************************************************************/
/* GetVoltHealthQst() - Returns health for voltage monitor                  */
/****************************************************************************/

int GetVoltHealthQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetVoltMonUpdate() )
      return( -1 );

   // Give precedence to monitor status over health status

   if( stVoltMonUpdateRsp.stMonitorUpdate[iVoltMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus )
      return( stVoltMonUpdateRsp.stMonitorUpdate[iVoltMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus );
   else
      return( stVoltMonUpdateRsp.stMonitorUpdate[iVoltMonIndex[iLocSensor]].stMonitorStatus.uThresholdStatus );
}

/****************************************************************************/
/* GetVoltHealthByteQst() - Returns health byte for voltage monitor         */
/****************************************************************************/

BOOL GetVoltHealthByteQst( int iLocSensor, QST_MON_HEALTH_STATUS *pstStatus )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetVoltMonUpdate() )
      return( FALSE );

   *((BYTE *)pstStatus) = *((BYTE *)&stVoltMonUpdateRsp.stMonitorUpdate[iVoltMonIndex[iLocSensor]].stMonitorStatus);
   return( TRUE );
}

/****************************************************************************/
/* GetVoltThreshQst() - Returns health thresholds for voltage monitor       */
/****************************************************************************/

BOOL GetVoltThreshQst( int iLocSensor, THRESH *pstThreshLow, THRESH *pstThreshHigh )
{
   if( (iLocSensor < 0) || (iLocSensor >= iVoltMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   pstThreshLow->fNonCrit   = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iUnderVoltageNonCritical    );
   pstThreshLow->fCrit      = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iUnderVoltageCritical       );
   pstThreshLow->fNonRecov  = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iUnderVoltageNonRecoverable );

   pstThreshHigh->fNonCrit  = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iOverVoltageNonCritical     );
   pstThreshHigh->fCrit     = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iOverVoltageCritical        );
   pstThreshHigh->fNonRecov = QST_VOLT_TO_FLOAT( stVoltMonConfigRsp[iLocSensor].iOverVoltageNonRecoverable  );

   return( TRUE );
}




/****************************************************************************/
/****************************************************************************/
/************************** Current Monitor Support *************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                              iCurrMons = 0;
static int                              iCurrMonIndex[QST_ABS_CURR_MONITORS];
static QST_GET_CURR_MON_CONFIG_RSP      stCurrMonConfigRsp[QST_ABS_CURR_MONITORS];
static QST_GET_CURR_MON_UPDATE_RSP      stCurrMonUpdateRsp;
static time_t                           tLastCurrMonUpdate = 0;

/****************************************************************************/
/* GetCurrMonConfig() - Get configuration for current monitor               */
/****************************************************************************/

static BOOL GetCurrMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD stGenericCmd;

   stGenericCmd.stHeader.byCommand       = QST_GET_CURR_MON_CONFIG;
   stGenericCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_CURR_MON_CONFIG_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stCurrMonConfigRsp[iLocSensor], sizeof(QST_GET_CURR_MON_CONFIG_RSP) ) )
      return( FALSE );

   if( stCurrMonConfigRsp[iLocSensor].byStatus )
   {
      SetQSTError( stCurrMonConfigRsp[iLocSensor].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetCurrMonUpdate() - Gets updated readings from current monitors         */
/****************************************************************************/

static BOOL GetCurrMonUpdate( void )
{
   EnterCritical();

   // Only update if readings are at least a second old

   if( tLastCurrMonUpdate < time( NULL ) )
   {
      QST_GENERIC_CMD stGenericCmd;

      // Send the Temperature Monitor Update request

      stGenericCmd.stHeader.byCommand       = QST_GET_CURR_MON_UPDATE;
      stGenericCmd.stHeader.byEntity        = 0;
      stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_CURR_MON_UPDATE_RSP);

      if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stCurrMonUpdateRsp, sizeof(QST_GET_CURR_MON_UPDATE_RSP) ) )
      {
         ExitCritical();
         return( FALSE );
      }

      // Can't go any further if Subsystem rejected the command

      if( stCurrMonUpdateRsp.byStatus )
      {
         SetQSTError( stCurrMonUpdateRsp.byStatus );
         ExitCritical();
         return( FALSE );
      }

      // Save the time of the update for later comparison

      tLastCurrMonUpdate = time( NULL );
   }

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* GetCurrCountQst() - Returns number of enabled current monitors           */
/****************************************************************************/

int GetCurrCountQst( void )
{
   return( iCurrMons );
}

/****************************************************************************/
/* GetCurrIndexQst() - Returns physical index for current monitor           */
/****************************************************************************/

int GetCurrIndexQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( iCurrMonIndex[iLocSensor] );
}

/****************************************************************************/
/* GetCurrUsageQst() - Returns usage indicator for current monitor          */
/****************************************************************************/

int GetCurrUsageQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( (int)stCurrMonConfigRsp[iLocSensor].byMonitorUsage );
}

/****************************************************************************/
/* GetCurrReadingQst() - Returns reading from current monitor               */
/****************************************************************************/

float GetCurrReadingQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetCurrMonUpdate() )
      return( -1 );

   return( QST_CURR_TO_FLOAT( stCurrMonUpdateRsp.stMonitorUpdate[iCurrMonIndex[iLocSensor]].iCurrentCurrent ) );
}

/****************************************************************************/
/* GetCurrHealthQst() - Returns health for current monitor                  */
/****************************************************************************/

int GetCurrHealthQst( int iLocSensor )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetCurrMonUpdate() )
      return( -1 );

   // Give precedence to monitor status over health status

   if( stCurrMonUpdateRsp.stMonitorUpdate[iCurrMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus )
      return( stCurrMonUpdateRsp.stMonitorUpdate[iCurrMonIndex[iLocSensor]].stMonitorStatus.uMonitorStatus );
   else
      return( stCurrMonUpdateRsp.stMonitorUpdate[iCurrMonIndex[iLocSensor]].stMonitorStatus.uThresholdStatus );
}

/****************************************************************************/
/* GetCurrHealthByteQst() - Returns health byte for current monitor         */
/****************************************************************************/

BOOL GetCurrHealthByteQst( int iLocSensor, QST_MON_HEALTH_STATUS *pstStatus )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetCurrMonUpdate() )
      return( FALSE );

   *((BYTE *)pstStatus) = *((BYTE *)&stCurrMonUpdateRsp.stMonitorUpdate[iCurrMonIndex[iLocSensor]].stMonitorStatus);
   return( TRUE );
}

/****************************************************************************/
/* GetCurrThreshQst() - Returns health thresholds for current monitor       */
/****************************************************************************/

BOOL GetCurrThreshQst( int iLocSensor, THRESH *pstThreshLow, THRESH *pstThreshHigh )
{
   if( (iLocSensor < 0) || (iLocSensor >= iCurrMons) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   pstThreshLow->fNonCrit   = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iUnderCurrentNonCritical    );
   pstThreshLow->fCrit      = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iUnderCurrentCritical       );
   pstThreshLow->fNonRecov  = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iUnderCurrentNonRecoverable );

   pstThreshHigh->fNonCrit  = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iOverCurrentNonCritical     );
   pstThreshHigh->fCrit     = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iOverCurrentCritical        );
   pstThreshHigh->fNonRecov = QST_CURR_TO_FLOAT( stCurrMonConfigRsp[iLocSensor].iOverCurrentNonRecoverable  );

   return( TRUE );
}




/****************************************************************************/
/****************************************************************************/
/*********************** Fan Speed Controller Support ***********************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static int                              iFanCtrls = 0;
static int                              iFanCtrlIndex[QST_ABS_FAN_CONTROLLERS];
static QST_GET_FAN_CTRL_CONFIG_RSP      stFanCtrlConfigRsp[QST_ABS_FAN_CONTROLLERS];
static QST_GET_FAN_CTRL_UPDATE_RSP      stFanCtrlUpdateRsp;
static time_t                           tLastFanCtrlUpdate = 0;
static BOOL                             bFanCtrlUpdatable = TRUE;

/****************************************************************************/
/* GetFanCtrlConfig() - Gets configuration for a fan speed controller       */
/****************************************************************************/

static BOOL GetFanCtrlConfig( int iRemCtrl, int iLocCtrl )
{
   QST_GENERIC_CMD stGenericCmd;

   stGenericCmd.stHeader.byCommand       = QST_GET_FAN_CTRL_CONFIG;
   stGenericCmd.stHeader.byEntity        = (UINT8)iRemCtrl;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_CTRL_CONFIG_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stFanCtrlConfigRsp[iLocCtrl], sizeof(QST_GET_FAN_CTRL_CONFIG_RSP) ) )
      return( FALSE );

   if( stFanCtrlConfigRsp[iLocCtrl].byStatus )
   {
      SetQSTError( stFanCtrlConfigRsp[iLocCtrl].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetFanCtrlUpdate() - Gets updated readings from fan speed controllers    */
/****************************************************************************/

static BOOL GetFanCtrlUpdate( void )
{
   EnterCritical();

   // Only update if readings are at least a second old

   if( tLastFanCtrlUpdate < time( NULL ) )
   {
      QST_GENERIC_CMD stGenericCmd;

      // Send the Fan Speed Monitor Update request

      stGenericCmd.stHeader.byCommand       = QST_GET_FAN_CTRL_UPDATE;
      stGenericCmd.stHeader.byEntity        = 0;
      stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_CTRL_UPDATE_RSP);

      if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stFanCtrlUpdateRsp, sizeof(QST_GET_FAN_CTRL_UPDATE_RSP) ) )
      {
         ExitCritical();
         return( FALSE );
      }

      // Can't go any further if Subsystem rejected the command

      if( stFanCtrlUpdateRsp.byStatus )
      {
         SetQSTError( stFanCtrlUpdateRsp.byStatus );
         ExitCritical();
         return( FALSE );
      }

      // Save the time of the update for later comparison

      tLastFanCtrlUpdate = time( NULL );
   }

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* GetDutyCountQst() - Returns number of enabled fan speed controllers      */
/****************************************************************************/

int GetDutyCountQst( void )
{
   return( iFanCtrls );
}

/****************************************************************************/
/* GetDutyIndexQst() - Returns physical index for fan speed controller      */
/****************************************************************************/

int GetDutyIndexQst( int iLocCtrl )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( iFanCtrlIndex[iLocCtrl] );
}

/****************************************************************************/
/* GetDutyUsageQst() - Returns usage indicator for fan speed controller     */
/****************************************************************************/

int GetDutyUsageQst( int iLocCtrl )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   return( (int)stFanCtrlConfigRsp[iLocCtrl].byControllerUsage );
}

/****************************************************************************/
/* GetDutySettingQst() - Returns duty cycle for fan speed controller        */
/****************************************************************************/

float GetDutySettingQst( int iLocCtrl )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetFanCtrlUpdate() )
      return( -1 );

   return( QST_DUTY_TO_FLOAT( stFanCtrlUpdateRsp.stControllerUpdate[iFanCtrlIndex[iLocCtrl]].uCurrentDutyCycle ) );
}

/****************************************************************************/
/* GetDutyHealthQst() - Returns health for fan speed controller             */
/****************************************************************************/

int GetDutyHealthQst( int iLocCtrl )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( -1 );
   }

   if( !GetFanCtrlUpdate() )
      return( -1 );

   return( stFanCtrlUpdateRsp.stControllerUpdate[iFanCtrlIndex[iLocCtrl]].stControllerStatus.uControllerStatus );
}

/****************************************************************************/
/* GetDutyHealthByteQst() - Returns health byte for fan speed controller    */
/****************************************************************************/

int GetDutyHealthByteQst( int iLocCtrl, QST_FAN_CTRL_STATUS *pstStatus )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetFanCtrlUpdate() )
      return( FALSE );

   *((BYTE *)pstStatus) = *((BYTE *)&stFanCtrlUpdateRsp.stControllerUpdate[iFanCtrlIndex[iLocCtrl]].stControllerStatus);
   return( TRUE );
}

/****************************************************************************/
/* GetDutyManualQst() - Returns indication of whether or not a fan speed    */
/* controller is being manually controlled.                                 */
/****************************************************************************/

BOOL GetDutyManualQst( int iLocCtrl, BOOL *pbSWControl )
{
   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !GetFanCtrlUpdate() )
      return( FALSE );

   *pbSWControl = stFanCtrlUpdateRsp.stControllerUpdate[iFanCtrlIndex[iLocCtrl]].stControllerStatus.bOverrideSoftware;
   return( TRUE );
}

/****************************************************************************/
/* SetDutyManualQst() - Sets duty cycle of the fan speed controller         */
/****************************************************************************/

BOOL SetDutyManualQst( int iLocCtrl, float fDutyCycle )
{
   QST_SET_FAN_CTRL_DUTY_CMD   stDutyCmd;
   QST_GENERIC_RSP             stGenericRsp;

   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   // Abort if updates aren't allowed

   if( !bFanCtrlUpdatable )
   {
      SetError( ERROR_QST_CAPABILITY_LOCKED );
      return( FALSE );
   }

   // Send update to QST Subsystem

   EnterCritical();

   stDutyCmd.stHeader.byCommand       = QST_SET_FAN_CTRL_DUTY;
   stDutyCmd.stHeader.byEntity        = (UINT8)iFanCtrlIndex[iLocCtrl];
   stDutyCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_SET_FAN_CTRL_DUTY_CMD);
   stDutyCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);
   stDutyCmd.uDutyCycle               = QST_DUTY_FROM_FLOAT(fDutyCycle);

   if( !QstCommand2( &stDutyCmd, sizeof(QST_SET_FAN_CTRL_DUTY_CMD), &stGenericRsp, sizeof(QST_GENERIC_RSP) ) )
   {
      ExitCritical();
      return( FALSE );
   }

   if( stGenericRsp.byStatus )
   {
      SetQSTError( stGenericRsp.byStatus );
      ExitCritical();
      return( FALSE );
   }

   tLastFanCtrlUpdate = time( NULL ) - 1;   // force Update refresh

   ExitCritical();
   return( TRUE );
}

/****************************************************************************/
/* SetDutyAutoQst() - Puts fan speed controller back into auto mode         */
/****************************************************************************/

BOOL SetDutyAutoQst( int iLocCtrl )
{
   QST_GENERIC_CMD stGenericCmd;
   QST_GENERIC_RSP stGenericRsp;

   if( (iLocCtrl < 0) || (iLocCtrl >= iFanCtrls) )
   {
      SetError( ERROR_QST_INVALID_PARAMETER );
      return( FALSE );
   }

   // Abort if updates aren't allowed

   if( !bFanCtrlUpdatable )
   {
      SetError( ERROR_QST_CAPABILITY_LOCKED );
      return( FALSE );
   }

   // Send update to QST Subsystem

   EnterCritical();

   stGenericCmd.stHeader.byCommand       = QST_SET_FAN_CTRL_AUTO;
   stGenericCmd.stHeader.byEntity        = (UINT8)iFanCtrlIndex[iLocCtrl];
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stGenericRsp, sizeof(QST_GENERIC_RSP) ) )
   {
      ExitCritical();
      return( FALSE );
   }

   if( stGenericRsp.byStatus )
   {
      SetError( stGenericRsp.byStatus );
      ExitCritical();
      return( FALSE );
   }

   tLastFanCtrlUpdate = time( NULL ) - 1;   // force Update refresh

   ExitCritical();
   return( TRUE );
}




/****************************************************************************/
/****************************************************************************/
/************************** Initialization/Cleanup **************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* EnumerateMonCtrl() - Enumerates the available temperature sensors, fan   */
/* speed sensors and fan speed controllers, through query of active Monitor */
/* and Controller processes operating within the QST Subsystem.             */
/****************************************************************************/

static BOOL EnumerateMonCtrl( void )
{
   int                                   iBit;
   QST_GENERIC_CMD                       stGenericCmd;
   QST_GET_SUBSYSTEM_STATUS_RSP          stStatusRsp;
   QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP  stProfileRsp;

   // Get the QST Subsystem's Status

   stGenericCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_STATUS;
   stGenericCmd.stHeader.byEntity        = 0;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_STATUS_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stStatusRsp, sizeof(QST_GET_SUBSYSTEM_STATUS_RSP) ) )
      return( FALSE );

   if( stStatusRsp.byStatus )
   {
      SetQSTError( stStatusRsp.byStatus );
      return( FALSE );
   }

   // No point in going any further if the subsystem isn't configured

   if( !stStatusRsp.stSubsystemStatus.bSubsystemConfigured )
   {
      SetError( ERROR_QST_NOT_CONFIGURED );
      return( FALSE );
   }

   // Check the Lock Mask to see whether can support control operations

   if( stStatusRsp.stLockMask.bLockManualFanControl )
      bFanCtrlUpdatable = FALSE;

   // Get the QST Subsystem's configuration profile

   stGenericCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_CONFIG_PROFILE;
   stGenericCmd.stHeader.byEntity        = 0;
   stGenericCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stGenericCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP);

   if( !QstCommand2( &stGenericCmd, sizeof(QST_GENERIC_CMD), &stProfileRsp, sizeof(QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP) ) )
      return( FALSE );

   if( stProfileRsp.byStatus )
   {
      SetQSTError( stProfileRsp.byStatus );
      return( FALSE );
   }

   // Ascertain temperature sensor count and configuration

   for( iBit = iTempMons = 0; iBit < QST_ABS_TEMP_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfileRsp.dwTempMonsConfigured, iBit ) )
      {
         if( !GetTempMonConfig( iBit, iTempMons ) )
            return( FALSE );

         iTempMonIndex[iTempMons++] = iBit;
      }
   }

   // Ascertain fan speed sensor count and configuration

   for( iBit = iFanMons = 0; iBit < QST_ABS_FAN_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfileRsp.dwFanMonsConfigured, iBit ) )
      {
         if( !GetFanMonConfig( iBit, iFanMons ) )
            return( FALSE );

         iFanMonIndex[iFanMons++] = iBit;
      }
   }

   // Ascertain voltage sensor count and configuration

   for( iBit = iVoltMons = 0; iBit < QST_ABS_VOLT_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfileRsp.dwVoltMonsConfigured, iBit ) )
      {
         if( !GetVoltMonConfig( iBit, iVoltMons ) )
            return( FALSE );

         iVoltMonIndex[iVoltMons++] = iBit;
      }
   }

   // Ascertain current sensor count and configuration

   for( iBit = iCurrMons = 0; iBit < QST_ABS_CURR_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfileRsp.dwCurrMonsConfigured, iBit ) )
      {
         if( !GetCurrMonConfig( iBit, iCurrMons ) )
            return( FALSE );

         iCurrMonIndex[iVoltMons++] = iBit;
      }
   }

   // Ascertain fan speed controller count and configuration

   for( iBit = iFanCtrls = 0; iBit < QST_ABS_FAN_CONTROLLERS; iBit++ )
   {
      if( BIT_SET( stProfileRsp.dwFanCtrlsConfigured, iBit ) )
      {
         if( !GetFanCtrlConfig( iBit, iFanCtrls ) )
            return( FALSE );

         iFanCtrlIndex[iFanCtrls++] = iBit;
      }
   }

   return( TRUE );
}

/****************************************************************************/
/* InitializeQst() - Initializes support for obtaining readings and         */
/* settings from the QST Subsystem.                                         */
/****************************************************************************/

BOOL InitializeQst( void )
{

#ifdef BE_THREAD_SAFE

   hCritSect = CreateCritSect( CRIT_SECT_TYPE, FALSE );

   if( hCritSect )
   {

#endif

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

      if( QstInitialize() )
      {

#endif

         if( EnumerateMonCtrl() )
         {
            BOOL bSuccess = TRUE;

            if( iTempMons )
               bSuccess = GetTempMonUpdate();

            if( bSuccess && iFanMons )
               bSuccess = GetFanMonUpdate();

            if( bSuccess && iVoltMons )
               bSuccess = GetVoltMonUpdate();

            if( bSuccess && iCurrMons )
               bSuccess = GetCurrMonUpdate();

            if( bSuccess && iFanCtrls )
               bSuccess = GetFanCtrlUpdate();

            if( bSuccess )
               return( TRUE );
         }

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

         QstCleanup();
      }

#endif

#ifdef BE_THREAD_SAFE

      CloseCritSect( hCritSect );
   }

#endif

   return( FALSE );
}

/****************************************************************************/
/* CleanupQst() - Cleans up the resources supporting communications with    */
/* the QST Subsystem.                                                       */
/****************************************************************************/

void CleanupQst( void )
{
#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)
   QstCleanup();
#endif

#ifdef BE_THREAD_SAFE
   CloseCritSect( hCritSect );
#endif

   return;
}

