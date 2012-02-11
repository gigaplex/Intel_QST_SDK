/****************************************************************************/
/*                                                                          */
/*  Module:         AccessQst.c                                             */
/*                                                                          */
/*  Description:    One of a set of modules that  implement  the  Intel(R)  */
/*                  Quiet System Technology (QST) Instrumentation Library.  */
/*                  This  library exposes a set of functions that simplify  */
/*                  support for the  enumeration  and  monitoring  of  the  */
/*                  various  sensors  and  fan  speed controllers that are  */
/*                  supported  by  QST.   This   module   implements   the  */
/*                  functions  that  are  used to enumerate and update the  */
/*                  readings,  health  and  thresholds  for  the   various  */
/*                  temperature,  voltage,  current  and fan speed sensors  */
/*                  and enumerate and update the settings  and  health  of  */
/*                  the fan speed controllers.                              */
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

#include "QstDll.h"
#include "QstComm.h"

/****************************************************************************/
/* SetQSTError() - Places the appropriate error code for the QST error      */
/* into the appropriate error variable for the environment.                 */
/****************************************************************************/

#ifdef __WIN32__

static const DWORD dwWin32Map[] =
{
   ERROR_SUCCESS,               // QST_CMD_SUCCESSFUL
   ERROR_NOT_SUPPORTED,         // QST_CMD_REJECTED_UNSUPPORTED
   ERROR_ACCESS_DENIED,         // QST_CMD_REJECTED_LOCKED
   ERROR_INVALID_PARAMETER,     // QST_CMD_REJECTED_PARAMETER
   ERROR_REVISION_MISMATCH,     // QST_CMD_REJECTED_VERSION
   ERROR_GEN_FAILURE,           // QST_CMD_FAILED_COMM_ERROR
   ERROR_BAD_UNIT,              // QST_CMD_FAILED_SENSOR_ERROR
   ERROR_NOT_ENOUGH_MEMORY,     // QST_CMD_FAILED_NO_MEMORY
   ERROR_NO_SYSTEM_RESOURCES,   // QST_CMD_FAILED_NO_RESOURCES
   ERROR_INVALID_FUNCTION,      // QST_CMD_REJECTED_INVALID
   ERROR_BAD_COMMAND,           // QST_CMD_REJECTED_CMD_SIZE
   ERROR_BAD_LENGTH             // QST_CMD_REJECTED_RSP_SIZE
};

#else

static const int iErrnoMap[] =
{
   0,                           // QST_CMD_SUCCESSFUL
   ENOEXEC,                     // QST_CMD_REJECTED_UNSUPPORTED
   EACCES,                      // QST_CMD_REJECTED_LOCKED
   EINVAL,                      // QST_CMD_REJECTED_PARAMETER
   EDOM,                        // QST_CMD_REJECTED_VERSION
   EIO,                         // QST_CMD_FAILED_COMM_ERROR
   EFAULT,                      // QST_CMD_FAILED_SENSOR_ERROR
   ENOMEM,                      // QST_CMD_FAILED_NO_MEMORY
   ENOSPC,                      // QST_CMD_FAILED_NO_RESOURCES
   EPERM,                       // QST_CMD_REJECTED_INVALID
   ERANGE,                      // QST_CMD_REJECTED_CMD_SIZE
   ERANGE                       // QST_CMD_REJECTED_RSP_SIZE
};

#endif

static void SetQSTError( UINT8 byStatus )
{

#ifdef __WIN32__
   SetLastError( (byStatus <= QST_CMD_REJECTED_RSP_SIZE)? dwWin32Map[byStatus] : ERROR_NONE_MAPPED );
#else
   errno = (byStatus <= QST_CMD_REJECTED_RSP_SIZE)? iErrnoMap[byStatus] : ENOENT;
#endif

}

/****************************************************************************/
/* GetTempMonConfig() - Get configuration for temperature monitor           */
/****************************************************************************/

static BOOL GetTempMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD               stCmd;
   P_QST_GET_TEMP_MON_CONFIG_RSP pstRsp    = &pQstSeg->stTempMonConfigRsp[iLocSensor];
   P_QST_THRESH                  pstThresh = &pQstSeg->stTempMonThresh[iLocSensor];

   // Send the Temperature Monitor Configuration request

   stCmd.stHeader.byCommand       = QST_GET_TEMP_MON_CONFIG;
   stCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_TEMP_MON_CONFIG_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), pstRsp, sizeof(QST_GET_TEMP_MON_CONFIG_RSP) ) )
      return( FALSE );

   // Can't go any further if Subsystem rejected the command

   if( pstRsp->byStatus )
   {
      SetQSTError( pstRsp->byStatus );
      return( FALSE );
   }

   // Successful, Save off thresholds in converted form

   pstThresh->fNonCritical    = QST_TEMP_TO_FLOAT( pstRsp->lfTempNonCritical    );
   pstThresh->fCritical       = QST_TEMP_TO_FLOAT( pstRsp->lfTempCritical       );
   pstThresh->fNonRecoverable = QST_TEMP_TO_FLOAT( pstRsp->lfTempNonRecoverable );

   return( TRUE );
}

/****************************************************************************/
/* GetTempMonUpdateQst() - Gets updated readings and health status for the  */
/* temperature monitors, provided the polling interval has expired          */
/****************************************************************************/

BOOL GetTempMonUpdateQst( void )
{
   MILLITIME stCurrTime;

   // Only update if readings are at least a second old

   CurrMTime( &stCurrTime );

   if( PastMTime( &pQstSeg->stTempMonUpdateTime, &stCurrTime ) )
   {
      QST_GENERIC_CMD stCmd;

      // Send the Temperature Monitor Update request

      stCmd.stHeader.byCommand       = QST_GET_TEMP_MON_UPDATE;
      stCmd.stHeader.byEntity        = 0;
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GET_TEMP_MON_UPDATE_RSP);

      if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stTempMonUpdateRsp, sizeof(QST_GET_TEMP_MON_UPDATE_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( pQstSeg->stTempMonUpdateRsp.byStatus )
      {
         SetQSTError( pQstSeg->stTempMonUpdateRsp.byStatus );
         return( FALSE );
      }

      // Save the time of the next update

      CopyMTime( &pQstSeg->stTempMonUpdateTime, &stCurrTime );
      AddMTime( &pQstSeg->stTempMonUpdateTime, 0, pQstSeg->dwPollingInterval );
   }

   return( TRUE );
}

/****************************************************************************/
/* SetTempThreshQst() - Sets health thresholds for temperature monitor      */
/****************************************************************************/

BOOL SetTempThreshQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iTempMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_TEMP_MON_THRESHOLDS_CMD  stCmd;
      QST_GENERIC_RSP                  stRsp;
      P_QST_GET_TEMP_MON_CONFIG_RSP    pstRsp    = &pQstSeg->stTempMonConfigRsp[iLocSensor];
      P_QST_THRESH                     pstThresh = &pQstSeg->stTempMonThresh[iLocSensor];

      stCmd.stHeader.byCommand       = QST_SET_TEMP_MON_THRESHOLDS;
      stCmd.stHeader.byEntity        = (UINT8)pQstSeg->iTempMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_SET_TEMP_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);

      // Convert to QST Subsystem's

      stCmd.lfTempNonCritical        = QST_TEMP_FROM_FLOAT( fNonCritical    + 0.005 );
      stCmd.lfTempCritical           = QST_TEMP_FROM_FLOAT( fCritical       + 0.005 );
      stCmd.lfTempNonRecoverable     = QST_TEMP_FROM_FLOAT( fNonRecoverable + 0.005 );

      if( !QstCommand2( &stCmd, sizeof(QST_SET_TEMP_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThresh->fNonCritical      = fNonCritical;
      pstThresh->fCritical         = fCritical;
      pstThresh->fNonRecoverable   = fNonRecoverable;

      // In both forms...

      pstRsp->lfTempNonCritical    = stCmd.lfTempNonCritical;
      pstRsp->lfTempCritical       = stCmd.lfTempCritical;
      pstRsp->lfTempNonRecoverable = stCmd.lfTempNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeTempMonThreshChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* GetFanMonConfig() - Get configuration for fan speed monitor              */
/****************************************************************************/

static BOOL GetFanMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD               stCmd;
   P_QST_GET_FAN_MON_CONFIG_RSP  pstRsp    = &pQstSeg->stFanMonConfigRsp[iLocSensor];
   P_QST_THRESH                  pstThresh = &pQstSeg->stFanMonThresh[iLocSensor];

   // Set Fan Speed Monitor Configuration command

   stCmd.stHeader.byCommand       = QST_GET_FAN_MON_CONFIG;
   stCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_MON_CONFIG_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), pstRsp, sizeof(QST_GET_FAN_MON_CONFIG_RSP) ) )
      return( FALSE );

   // Can't go any further if Subsystem rejected the command

   if( pstRsp->byStatus )
   {
      SetQSTError( pstRsp->byStatus );
      return( FALSE );
   }

   // Successful, Save off thresholds in converted form

   pstThresh->fNonCritical    = (float)pstRsp->uSpeedNonCritical;
   pstThresh->fCritical       = (float)pstRsp->uSpeedCritical;
   pstThresh->fNonRecoverable = (float)pstRsp->uSpeedNonRecoverable;

   return( TRUE );
}

/****************************************************************************/
/* GetFanMonUpdateQst() - Gets updated readings and health status for the   */
/* fan speed monitors, provided the polling interval has expired            */
/****************************************************************************/

BOOL GetFanMonUpdateQst( void )
{
   MILLITIME stCurrTime;

   // Only update if readings are at least a second old

   CurrMTime( &stCurrTime );

   if( PastMTime( &pQstSeg->stFanMonUpdateTime, &stCurrTime ) )
   {
      QST_GENERIC_CMD stCmd;

      // Send the Fan Speed Monitor Update request

      stCmd.stHeader.byCommand       = QST_GET_FAN_MON_UPDATE;
      stCmd.stHeader.byEntity        = 0;
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_MON_UPDATE_RSP);

      if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stFanMonUpdateRsp, sizeof(QST_GET_FAN_MON_UPDATE_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( pQstSeg->stFanMonUpdateRsp.byStatus )
      {
         SetQSTError( pQstSeg->stFanMonUpdateRsp.byStatus );
         return( FALSE );
      }

      // Save the time of the next update

      CopyMTime( &pQstSeg->stFanMonUpdateTime, &stCurrTime );
      AddMTime( &pQstSeg->stFanMonUpdateTime, 0, pQstSeg->dwPollingInterval );
   }

   return( TRUE );
}

/****************************************************************************/
/* SetFanThreshQst() - Sets health thresholds for fan speed monitor         */
/****************************************************************************/

BOOL SetFanThreshQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iFanMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_FAN_MON_THRESHOLDS_CMD stCmd;
      QST_GENERIC_RSP                stRsp;
      P_QST_GET_FAN_MON_CONFIG_RSP   pstRsp    = &pQstSeg->stFanMonConfigRsp[iLocSensor];
      P_QST_THRESH                   pstThresh = &pQstSeg->stFanMonThresh[iLocSensor];

      stCmd.stHeader.byCommand       = QST_SET_FAN_MON_THRESHOLDS;
      stCmd.stHeader.byEntity        = (UINT8)pQstSeg->iFanMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_SET_FAN_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GENERIC_RSP);

      stCmd.uSpeedNonCritical        = (UINT16)fNonCritical;
      stCmd.uSpeedCritical           = (UINT16)fCritical;
      stCmd.uSpeedNonRecoverable     = (UINT16)fNonRecoverable;

      if( !QstCommand2( &stCmd, sizeof(QST_SET_FAN_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThresh->fNonCritical      = fNonCritical;
      pstThresh->fCritical         = fCritical;
      pstThresh->fNonRecoverable   = fNonRecoverable;

      // In both forms...

      pstRsp->uSpeedNonCritical    = stCmd.uSpeedNonCritical;
      pstRsp->uSpeedCritical       = stCmd.uSpeedCritical;
      pstRsp->uSpeedNonRecoverable = stCmd.uSpeedNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeFanMonThreshChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* GetVoltMonConfig() - Get configuration for voltage monitor               */
/****************************************************************************/

static BOOL GetVoltMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD               stCmd;
   P_QST_GET_VOLT_MON_CONFIG_RSP pstRsp        = &pQstSeg->stVoltMonConfigRsp[iLocSensor];
   P_QST_THRESH                  pstThreshLow  = &pQstSeg->stVoltMonThreshLow[iLocSensor];
   P_QST_THRESH                  pstThreshHigh = &pQstSeg->stVoltMonThreshHigh[iLocSensor];

   stCmd.stHeader.byCommand       = QST_GET_VOLT_MON_CONFIG;
   stCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_VOLT_MON_CONFIG_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), pstRsp, sizeof(QST_GET_VOLT_MON_CONFIG_RSP) ) )
      return( FALSE );

   if( pstRsp->byStatus )
   {
      SetQSTError( pstRsp->byStatus );
      return( FALSE );
   }

   // Successful, Save off thresholds in converted form

   pstThreshLow->fNonCritical     = QST_VOLT_TO_FLOAT( pstRsp->iUnderVoltageNonCritical    );
   pstThreshLow->fCritical        = QST_VOLT_TO_FLOAT( pstRsp->iUnderVoltageCritical       );
   pstThreshLow->fNonRecoverable  = QST_VOLT_TO_FLOAT( pstRsp->iUnderVoltageNonRecoverable );

   pstThreshHigh->fNonCritical    = QST_VOLT_TO_FLOAT( pstRsp->iOverVoltageNonCritical     );
   pstThreshHigh->fCritical       = QST_VOLT_TO_FLOAT( pstRsp->iOverVoltageCritical        );
   pstThreshHigh->fNonRecoverable = QST_VOLT_TO_FLOAT( pstRsp->iOverVoltageNonRecoverable  );

   return( TRUE );
}

/****************************************************************************/
/* GetVoltMonUpdateQst() - Gets updated readings and health status for the  */
/* voltage monitors, provided the polling interval has expired              */
/****************************************************************************/

BOOL GetVoltMonUpdateQst( void )
{
   MILLITIME stCurrTime;

   // Only update if readings are at least a second old

   CurrMTime( &stCurrTime );

   if( PastMTime( &pQstSeg->stVoltMonUpdateTime, &stCurrTime ) )
   {
      QST_GENERIC_CMD stCmd;

      // Send the Temperature Monitor Update request

      stCmd.stHeader.byCommand       = QST_GET_VOLT_MON_UPDATE;
      stCmd.stHeader.byEntity        = 0;
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GET_VOLT_MON_UPDATE_RSP);

      if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stVoltMonUpdateRsp, sizeof(QST_GET_VOLT_MON_UPDATE_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( pQstSeg->stVoltMonUpdateRsp.byStatus )
      {
         SetQSTError( pQstSeg->stVoltMonUpdateRsp.byStatus );
         return( FALSE );
      }

      // Save the time of the next update

      CopyMTime( &pQstSeg->stVoltMonUpdateTime, &stCurrTime );
      AddMTime( &pQstSeg->stVoltMonUpdateTime, 0, pQstSeg->dwPollingInterval );
   }

   return( TRUE );
}

/****************************************************************************/
/* SetVoltThreshLowQst() - Sets low health thresholds for voltage monitor   */
/****************************************************************************/

BOOL SetVoltThreshLowQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iVoltMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_VOLT_MON_THRESHOLDS_CMD stCmd;
      QST_GENERIC_RSP                 stRsp;
      P_QST_GET_VOLT_MON_CONFIG_RSP   pstRsp       = &pQstSeg->stVoltMonConfigRsp[iLocSensor];
      P_QST_THRESH                    pstThreshLow = &pQstSeg->stVoltMonThreshLow[iLocSensor];

      stCmd.stHeader.byCommand          = QST_SET_VOLT_MON_THRESHOLDS;
      stCmd.stHeader.byEntity           = (UINT8)pQstSeg->iVoltMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength     = QST_CMD_DATA_SIZE(QST_SET_VOLT_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength    = sizeof(QST_GENERIC_RSP);

      stCmd.iUnderVoltageNonCritical    = QST_VOLT_FROM_FLOAT(fNonCritical    + 0.0005);
      stCmd.iUnderVoltageCritical       = QST_VOLT_FROM_FLOAT(fCritical       + 0.0005);
      stCmd.iUnderVoltageNonRecoverable = QST_VOLT_FROM_FLOAT(fNonRecoverable + 0.0005);

      stCmd.iOverVoltageNonCritical     = pstRsp->iOverVoltageNonCritical;
      stCmd.iOverVoltageCritical        = pstRsp->iOverVoltageCritical;
      stCmd.iOverVoltageNonRecoverable  = pstRsp->iOverVoltageNonRecoverable;

      if( !QstCommand2( &stCmd, sizeof(QST_SET_VOLT_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThreshLow->fNonCritical          = fNonCritical;
      pstThreshLow->fCritical             = fCritical;
      pstThreshLow->fNonRecoverable       = fNonRecoverable;

      // In both forms...

      pstRsp->iUnderVoltageNonCritical    = stCmd.iUnderVoltageNonCritical;
      pstRsp->iUnderVoltageCritical       = stCmd.iUnderVoltageCritical;
      pstRsp->iUnderVoltageNonRecoverable = stCmd.iUnderVoltageNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeVoltMonThreshLowChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* SetVoltThreshHighQst() - Sets high health thresholds for voltage monitor */
/****************************************************************************/

BOOL SetVoltThreshHighQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iVoltMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_VOLT_MON_THRESHOLDS_CMD stCmd;
      QST_GENERIC_RSP                 stRsp;
      P_QST_GET_VOLT_MON_CONFIG_RSP   pstRsp        = &pQstSeg->stVoltMonConfigRsp[iLocSensor];
      P_QST_THRESH                    pstThreshHigh = &pQstSeg->stVoltMonThreshHigh[iLocSensor];

      stCmd.stHeader.byCommand          = QST_SET_VOLT_MON_THRESHOLDS;
      stCmd.stHeader.byEntity           = (UINT8)pQstSeg->iVoltMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength     = QST_CMD_DATA_SIZE(QST_SET_VOLT_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength    = sizeof(QST_GENERIC_RSP);

      stCmd.iUnderVoltageNonCritical    = pstRsp->iUnderVoltageNonCritical;
      stCmd.iUnderVoltageCritical       = pstRsp->iUnderVoltageCritical;
      stCmd.iUnderVoltageNonRecoverable = pstRsp->iUnderVoltageNonRecoverable;

      stCmd.iOverVoltageNonCritical     = QST_VOLT_FROM_FLOAT(fNonCritical    + 0.0005);
      stCmd.iOverVoltageCritical        = QST_VOLT_FROM_FLOAT(fCritical       + 0.0005);
      stCmd.iOverVoltageNonRecoverable  = QST_VOLT_FROM_FLOAT(fNonRecoverable + 0.0005);

      if( !QstCommand2( &stCmd, sizeof(QST_SET_VOLT_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThreshHigh->fNonCritical        = fNonCritical;
      pstThreshHigh->fCritical           = fCritical;
      pstThreshHigh->fNonRecoverable     = fNonRecoverable;

      // In both forms...

      pstRsp->iOverVoltageNonCritical    = stCmd.iOverVoltageNonCritical;
      pstRsp->iOverVoltageCritical       = stCmd.iOverVoltageCritical;
      pstRsp->iOverVoltageNonRecoverable = stCmd.iOverVoltageNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeVoltMonThreshHighChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* GetCurrMonConfig() - Get configuration for voltage monitor               */
/****************************************************************************/

static BOOL GetCurrMonConfig( int iRemSensor, int iLocSensor )
{
   QST_GENERIC_CMD               stCmd;
   P_QST_GET_CURR_MON_CONFIG_RSP pstRsp        = &pQstSeg->stCurrMonConfigRsp[iLocSensor];
   P_QST_THRESH                  pstThreshLow  = &pQstSeg->stCurrMonThreshLow[iLocSensor];
   P_QST_THRESH                  pstThreshHigh = &pQstSeg->stCurrMonThreshHigh[iLocSensor];

   stCmd.stHeader.byCommand       = QST_GET_CURR_MON_CONFIG;
   stCmd.stHeader.byEntity        = (UINT8)iRemSensor;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_CURR_MON_CONFIG_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), pstRsp, sizeof(QST_GET_CURR_MON_CONFIG_RSP) ) )
      return( FALSE );

   if( pstRsp->byStatus )
   {
      SetQSTError( pstRsp->byStatus );
      return( FALSE );
   }

   // Successful, Save off thresholds in converted form

   pstThreshLow->fNonCritical     = QST_CURR_TO_FLOAT( pstRsp->iUnderCurrentNonCritical    );
   pstThreshLow->fCritical        = QST_CURR_TO_FLOAT( pstRsp->iUnderCurrentCritical       );
   pstThreshLow->fNonRecoverable  = QST_CURR_TO_FLOAT( pstRsp->iUnderCurrentNonRecoverable );

   pstThreshHigh->fNonCritical    = QST_CURR_TO_FLOAT( pstRsp->iOverCurrentNonCritical     );
   pstThreshHigh->fCritical       = QST_CURR_TO_FLOAT( pstRsp->iOverCurrentCritical        );
   pstThreshHigh->fNonRecoverable = QST_CURR_TO_FLOAT( pstRsp->iOverCurrentNonRecoverable  );

   return( TRUE );
}

/****************************************************************************/
/* GetCurrMonUpdateQst() - Gets updated readings and health status for the  */
/* voltage monitors, provided the polling interval has expired              */
/****************************************************************************/

BOOL GetCurrMonUpdateQst( void )
{
   MILLITIME stCurrTime;

   // Only update if readings are at least a second old

   CurrMTime( &stCurrTime );

   if( PastMTime( &pQstSeg->stCurrMonUpdateTime, &stCurrTime ) )
   {
      QST_GENERIC_CMD stCmd;

      // Send the Temperature Monitor Update request

      stCmd.stHeader.byCommand       = QST_GET_CURR_MON_UPDATE;
      stCmd.stHeader.byEntity        = 0;
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GET_CURR_MON_UPDATE_RSP);

      if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stCurrMonUpdateRsp, sizeof(QST_GET_CURR_MON_UPDATE_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( pQstSeg->stCurrMonUpdateRsp.byStatus )
      {
         SetQSTError( pQstSeg->stCurrMonUpdateRsp.byStatus );
         return( FALSE );
      }

      // Save the time of the next update

      CopyMTime( &pQstSeg->stCurrMonUpdateTime, &stCurrTime );
      AddMTime( &pQstSeg->stCurrMonUpdateTime, 0, pQstSeg->dwPollingInterval );
   }

   return( TRUE );
}

/****************************************************************************/
/* SetCurrThreshLowQst() - Sets low health thresholds for voltage monitor   */
/****************************************************************************/

BOOL SetCurrThreshLowQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iCurrMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_CURR_MON_THRESHOLDS_CMD stCmd;
      QST_GENERIC_RSP                 stRsp;
      P_QST_GET_CURR_MON_CONFIG_RSP   pstRsp       = &pQstSeg->stCurrMonConfigRsp[iLocSensor];
      P_QST_THRESH                    pstThreshLow = &pQstSeg->stCurrMonThreshLow[iLocSensor];

      stCmd.stHeader.byCommand          = QST_SET_CURR_MON_THRESHOLDS;
      stCmd.stHeader.byEntity           = (UINT8)pQstSeg->iCurrMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength     = QST_CMD_DATA_SIZE(QST_SET_CURR_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength    = sizeof(QST_GENERIC_RSP);

      stCmd.iUnderCurrentNonCritical    = QST_CURR_FROM_FLOAT(fNonCritical    + 0.0005);
      stCmd.iUnderCurrentCritical       = QST_CURR_FROM_FLOAT(fCritical       + 0.0005);
      stCmd.iUnderCurrentNonRecoverable = QST_CURR_FROM_FLOAT(fNonRecoverable + 0.0005);

      stCmd.iOverCurrentNonCritical     = pstRsp->iOverCurrentNonCritical;
      stCmd.iOverCurrentCritical        = pstRsp->iOverCurrentCritical;
      stCmd.iOverCurrentNonRecoverable  = pstRsp->iOverCurrentNonRecoverable;

      if( !QstCommand2( &stCmd, sizeof(QST_SET_CURR_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThreshLow->fNonCritical          = fNonCritical;
      pstThreshLow->fCritical             = fCritical;
      pstThreshLow->fNonRecoverable       = fNonRecoverable;

      // In both forms...

      pstRsp->iUnderCurrentNonCritical    = stCmd.iUnderCurrentNonCritical;
      pstRsp->iUnderCurrentCritical       = stCmd.iUnderCurrentCritical;
      pstRsp->iUnderCurrentNonRecoverable = stCmd.iUnderCurrentNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeCurrMonThreshLowChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* SetCurrThreshHighQst() - Sets high health thresholds for voltage monitor */
/****************************************************************************/

BOOL SetCurrThreshHighQst( int iLocSensor, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( (iLocSensor < 0) || (iLocSensor >= pQstSeg->iCurrMons) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }
   else
   {
      QST_SET_CURR_MON_THRESHOLDS_CMD stCmd;
      QST_GENERIC_RSP                 stRsp;
      P_QST_GET_CURR_MON_CONFIG_RSP   pstRsp        = &pQstSeg->stCurrMonConfigRsp[iLocSensor];
      P_QST_THRESH                    pstThreshHigh = &pQstSeg->stCurrMonThreshHigh[iLocSensor];

      stCmd.stHeader.byCommand          = QST_SET_CURR_MON_THRESHOLDS;
      stCmd.stHeader.byEntity           = (UINT8)pQstSeg->iCurrMonIndex[iLocSensor];
      stCmd.stHeader.wCommandLength     = QST_CMD_DATA_SIZE(QST_SET_CURR_MON_THRESHOLDS_CMD);
      stCmd.stHeader.wResponseLength    = sizeof(QST_GENERIC_RSP);

      stCmd.iUnderCurrentNonCritical    = pstRsp->iUnderCurrentNonCritical;
      stCmd.iUnderCurrentCritical       = pstRsp->iUnderCurrentCritical;
      stCmd.iUnderCurrentNonRecoverable = pstRsp->iUnderCurrentNonRecoverable;

      stCmd.iOverCurrentNonCritical     = QST_CURR_FROM_FLOAT(fNonCritical    + 0.0005);
      stCmd.iOverCurrentCritical        = QST_CURR_FROM_FLOAT(fCritical       + 0.0005);
      stCmd.iOverCurrentNonRecoverable  = QST_CURR_FROM_FLOAT(fNonRecoverable + 0.0005);

      if( !QstCommand2( &stCmd, sizeof(QST_SET_CURR_MON_THRESHOLDS_CMD), &stRsp, sizeof(QST_GENERIC_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( stRsp.byStatus )
      {
         SetQSTError( stRsp.byStatus );
         return( FALSE );
      }

      // Successful, can now save updated thresholds in our local data

      pstThreshHigh->fNonCritical        = fNonCritical;
      pstThreshHigh->fCritical           = fCritical;
      pstThreshHigh->fNonRecoverable     = fNonRecoverable;

      // In both forms...

      pstRsp->iOverCurrentNonCritical    = stCmd.iOverCurrentNonCritical;
      pstRsp->iOverCurrentCritical       = stCmd.iOverCurrentCritical;
      pstRsp->iOverCurrentNonRecoverable = stCmd.iOverCurrentNonRecoverable;

      // Save current time as time of threshold change

      time( &pQstSeg->tTimeCurrMonThreshHighChanged[iLocSensor] );
      return( TRUE );
   }
}

/****************************************************************************/
/* GetFanCtrlConfig() - Gets configuration for a fan speed controller       */
/****************************************************************************/

static BOOL GetFanCtrlConfig( int iRemCtrl, int iLocCtrl )
{
   QST_GENERIC_CMD stCmd;

   stCmd.stHeader.byCommand       = QST_GET_FAN_CTRL_CONFIG;
   stCmd.stHeader.byEntity        = (UINT8)iRemCtrl;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_CTRL_CONFIG_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stFanCtrlConfigRsp[iLocCtrl], sizeof(QST_GET_FAN_CTRL_CONFIG_RSP) ) )
      return( FALSE );

   if( pQstSeg->stFanCtrlConfigRsp[iLocCtrl].byStatus )
   {
      SetQSTError( pQstSeg->stFanCtrlConfigRsp[iLocCtrl].byStatus );
      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* GetFanCtrlUpdateQst() - Gets updated duty settings and health status     */
/* for the Fan Controllers, provided the polling interval has expired       */
/****************************************************************************/

BOOL GetFanCtrlUpdateQst( void )
{
   MILLITIME stCurrTime;

   // Only update if readings are at least a second old

   CurrMTime( &stCurrTime );

   if( PastMTime( &pQstSeg->stFanCtrlUpdateTime, &stCurrTime ) )
   {
      QST_GENERIC_CMD stCmd;

      // Send the Fan Speed Monitor Update request

      stCmd.stHeader.byCommand       = QST_GET_FAN_CTRL_UPDATE;
      stCmd.stHeader.byEntity        = 0;
      stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
      stCmd.stHeader.wResponseLength = sizeof(QST_GET_FAN_CTRL_UPDATE_RSP);

      if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &pQstSeg->stFanCtrlUpdateRsp, sizeof(QST_GET_FAN_CTRL_UPDATE_RSP) ) )
         return( FALSE );

      // Can't go any further if Subsystem rejected the command

      if( pQstSeg->stFanCtrlUpdateRsp.byStatus )
      {
         SetQSTError( pQstSeg->stFanCtrlUpdateRsp.byStatus );
         return( FALSE );
      }

      // Save the time of the next update

      CopyMTime( &pQstSeg->stFanCtrlUpdateTime, &stCurrTime );
      AddMTime( &pQstSeg->stFanCtrlUpdateTime, 0, pQstSeg->dwPollingInterval );
   }

   return( TRUE );
}

/****************************************************************************/
/* EnumerateMonCtrl() - Enumerates the available temperature sensors, fan   */
/* speed sensors and fan speed controllers, through query of active Monitor */
/* and Controller processes operating within the QST Subsystem.             */
/****************************************************************************/

static BOOL EnumerateMonCtrl( void )
{
   int                                   iBit;
   QST_GENERIC_CMD                       stCmd;
   QST_GET_SUBSYSTEM_STATUS_RSP          stStatRsp;
   QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP  stProfRsp;

   // Get the QST Subsystem's Status

   stCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_STATUS;
   stCmd.stHeader.byEntity        = 0;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_STATUS_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &stStatRsp, sizeof(QST_GET_SUBSYSTEM_STATUS_RSP) ) )
      return( FALSE );

   if( stStatRsp.byStatus )
   {
      SetQSTError( stStatRsp.byStatus );
      return( FALSE );
   }

   // No point in going any further if the subsystem isn't configured

   if( !stStatRsp.stSubsystemStatus.bSubsystemConfigured )
   {

#ifdef __WIN32__
      SetLastError( ERROR_BAD_CONFIGURATION );
#else
      errno = ENODEV;
#endif

      return( FALSE );
   }

   // Get the QST Subsystem's configuration profile

   stCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_CONFIG_PROFILE;
   stCmd.stHeader.byEntity        = 0;
   stCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
   stCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP);

   if( !QstCommand2( &stCmd, sizeof(QST_GENERIC_CMD), &stProfRsp, sizeof(QST_GET_SUBSYSTEM_CONFIG_PROFILE_RSP) ) )
      return( FALSE );

   if( stProfRsp.byStatus )
   {
      SetQSTError( stProfRsp.byStatus );
      return( FALSE );
   }

   // Ascertain temperature sensor count and configuration

   for( iBit = pQstSeg->iTempMons = 0; iBit < QST_ABS_TEMP_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfRsp.dwTempMonsConfigured, iBit ) )
      {
         if( !GetTempMonConfig( iBit, pQstSeg->iTempMons ) )
            return( FALSE );

         pQstSeg->iTempMonIndex[pQstSeg->iTempMons++] = iBit;
      }
   }

   if( !GetTempMonUpdateQst() )
      return( FALSE );

   // Ascertain fan speed sensor count and configuration

   for( iBit = pQstSeg->iFanMons = 0; iBit < QST_ABS_FAN_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfRsp.dwFanMonsConfigured, iBit ) )
      {
         if( !GetFanMonConfig( iBit, pQstSeg->iFanMons ) )
            return( FALSE );

         pQstSeg->iFanMonIndex[pQstSeg->iFanMons++] = iBit;
      }
   }

   if( !GetFanMonUpdateQst() )
      return( FALSE );

   // Ascertain voltage sensor count and configuration

   for( iBit = pQstSeg->iVoltMons = 0; iBit < QST_ABS_VOLT_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfRsp.dwVoltMonsConfigured, iBit ) )
      {
         if( !GetVoltMonConfig( iBit, pQstSeg->iVoltMons ) )
            return( FALSE );

         pQstSeg->iVoltMonIndex[pQstSeg->iVoltMons++] = iBit;
      }
   }

   if( !GetVoltMonUpdateQst() )
      return( FALSE );

   // Ascertain current sensor count and configuration

   for( iBit = pQstSeg->iCurrMons = 0; iBit < QST_ABS_CURR_MONITORS; iBit++ )
   {
      if( BIT_SET( stProfRsp.dwCurrMonsConfigured, iBit ) )
      {
         if( !GetCurrMonConfig( iBit, pQstSeg->iCurrMons ) )
            return( FALSE );

         pQstSeg->iCurrMonIndex[pQstSeg->iCurrMons++] = iBit;
      }
   }

   if( !GetCurrMonUpdateQst() )
      return( FALSE );

   // Ascertain fan speed controller count and configuration

   for( iBit = pQstSeg->iFanCtrls = 0; iBit < QST_ABS_FAN_CONTROLLERS; iBit++ )
   {
      if( BIT_SET( stProfRsp.dwFanCtrlsConfigured, iBit ) )
      {
         if( !GetFanCtrlConfig( iBit, pQstSeg->iFanCtrls ) )
            return( FALSE );

         pQstSeg->iFanCtrlIndex[pQstSeg->iFanCtrls++] = iBit;
      }
   }

   if( !GetFanCtrlUpdateQst() )
      return( FALSE );

   return( TRUE );
}

/****************************************************************************/
/* InitializeQst() - Initializes support for obtaining readings and         */
/* settings from the QST Subsystem.                                         */
/****************************************************************************/

BOOL InitializeQst( BOOL bInitQstSeg )
{
#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

   if( QstInitialize() )
   {

#endif

      if( !bInitQstSeg )
         return( TRUE );

      if( EnumerateMonCtrl() )
         return( TRUE );

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

      QstCleanup();
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

   return;
}

