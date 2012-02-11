/****************************************************************************/
/*                                                                          */
/*  Module:         QstInst.c                                               */
/*                                                                          */
/*  Description:    One of a set of modules that  implement  the  Intel(R)  */
/*                  Quiet System Technology (QST) Instrumentation Library.  */
/*                  This  library exposes a set of functions that simplify  */
/*                  support for the  enumeration  and  monitoring  of  the  */
/*                  various  sensors  and  fan  speed controllers that are  */
/*                  supported by QST. This module  implements  the  public  */
/*                  (API) functions of the Library.                         */
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
#include <time.h>
#include <errno.h>

#if defined(_WIN32) || defined(__WIN32__)
#pragma warning(disable: 4701)
#endif

#include "QstDll.h"
#include "QstInst.h"

/****************************************************************************/
/* QstGetSensorCount() - Returns a count of the number of sensors of the    */
/* specified type that are being managed by the QST Subsystem               */
/****************************************************************************/

BOOL APIENTRY QstGetSensorCount
(
   IN   QST_SENSOR_TYPE             eType,
   OUT  int                         *piCount
){
   // Handle obvious parameters issues

   if( !piCount )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      if( piCount )
         *piCount = 0;     // be nice to folks ignoring return code

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      *piCount = 0;     // be nice to folks ignoring return code
      return( FALSE );
   }

   // Process request

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      *piCount = pQstSeg->iTempMons;
      break;

   case VOLTAGE_SENSOR:

      *piCount = pQstSeg->iVoltMons;
       break;

   case CURRENT_SENSOR:

      *piCount = pQstSeg->iCurrMons;
       break;

   case FAN_SPEED_SENSOR:

      *piCount = pQstSeg->iFanMons;
      break;

   default:

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   return( TRUE );
}

/****************************************************************************/
/* QstGetSensorConfiguration() - Returns configuration for the specified    */
/* sensor                                                                   */
/****************************************************************************/

BOOL APIENTRY QstGetSensorConfiguration
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   OUT  QST_FUNCTION                *peFunction,
   OUT  BOOL                        *pbRelative,
   OUT  float                       *pfNominal
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !peFunction || !pbRelative || !pfNominal || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( iIndex >= pQstSeg->iTempMons )
      {

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      *peFunction = (QST_FUNCTION)pQstSeg->stTempMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = (BOOL)pQstSeg->stTempMonConfigRsp[iIndex].bRelativeReadings;
      *pfNominal  = QST_TEMP_TO_FLOAT( pQstSeg->stTempMonConfigRsp[iIndex].lfTempNominal );

      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstSeg->iVoltMons )
      {

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      *peFunction = (QST_FUNCTION)pQstSeg->stVoltMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = QST_VOLT_TO_FLOAT( pQstSeg->stVoltMonConfigRsp[iIndex].iVoltageNominal );

      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstSeg->iCurrMons )
      {

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      *peFunction = (QST_FUNCTION)pQstSeg->stCurrMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = QST_CURR_TO_FLOAT( pQstSeg->stCurrMonConfigRsp[iIndex].iCurrentNominal );

      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstSeg->iFanMons )
      {

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      *peFunction = (QST_FUNCTION)pQstSeg->stFanMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = (float)pQstSeg->stFanMonConfigRsp[iIndex].uSpeedNominal;

      bSuccess = TRUE;
      break;

   default:

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      break;
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorThresholdsHigh() - Returns the high health thresholds for    */
/* the specified sensor.                                                    */
/****************************************************************************/

BOOL APIENTRY QstGetSensorThresholdsHigh
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   OUT  float                       *pfNonCritical,
   OUT  float                       *pfCritical,
   OUT  float                       *pfNonRecoverable
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pfNonCritical || !pfCritical || !pfNonRecoverable || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case TEMPERATURE_SENSOR:

         if( iIndex >= pQstSeg->iTempMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stTempMonThresh[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stTempMonThresh[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stTempMonThresh[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case VOLTAGE_SENSOR:

         if( iIndex >= pQstSeg->iVoltMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stVoltMonThreshHigh[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stVoltMonThreshHigh[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stVoltMonThreshHigh[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case CURRENT_SENSOR:

         if( iIndex >= pQstSeg->iCurrMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stCurrMonThreshHigh[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stCurrMonThreshHigh[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stCurrMonThreshHigh[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case FAN_SPEED_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorThresholdsLow() - Returns the low health thresholds for the  */
/* specified sensor.                                                        */
/****************************************************************************/

BOOL APIENTRY QstGetSensorThresholdsLow(

   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   OUT  float                       *pfNonCritical,
   OUT  float                       *pfCritical,
   OUT  float                       *pfNonRecoverable
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pfNonCritical || !pfCritical || !pfNonRecoverable || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case VOLTAGE_SENSOR:

         if( iIndex >= pQstSeg->iVoltMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stVoltMonThreshLow[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stVoltMonThreshLow[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stVoltMonThreshLow[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case CURRENT_SENSOR:

         if( iIndex >= pQstSeg->iCurrMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stCurrMonThreshLow[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stCurrMonThreshLow[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stCurrMonThreshLow[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case FAN_SPEED_SENSOR:

         if( iIndex >= pQstSeg->iFanMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pfNonCritical    = pQstSeg->stFanMonThresh[iIndex].fNonCritical;
         *pfCritical       = pQstSeg->stFanMonThresh[iIndex].fCritical;
         *pfNonRecoverable = pQstSeg->stFanMonThresh[iIndex].fNonRecoverable;

         bSuccess = TRUE;
         break;

      case TEMPERATURE_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorHealth() - Returns the health status for the specified       */
/* sensor                                                                   */
/****************************************************************************/

BOOL APIENTRY QstGetSensorHealth
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   OUT  QST_HEALTH                  *peHealth
){
   BOOL                             bSuccess = FALSE;
   QST_MON_HEALTH_STATUS            *pstStatus;

   // Handle obvious parameters issues

   if( !peHealth || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case TEMPERATURE_SENSOR:

         if( (iIndex >= pQstSeg->iTempMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetTempMonUpdateQst() )
         {
            pstStatus = &pQstSeg->stTempMonUpdateRsp.stMonitorUpdate[pQstSeg->iTempMonIndex[iIndex]].stMonitorStatus;
            bSuccess = TRUE;
         }

         break;

      case VOLTAGE_SENSOR:

         if( iIndex >= pQstSeg->iVoltMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetVoltMonUpdateQst() )
         {
            pstStatus = &pQstSeg->stVoltMonUpdateRsp.stMonitorUpdate[pQstSeg->iVoltMonIndex[iIndex]].stMonitorStatus;
            bSuccess = TRUE;
         }

         break;

      case CURRENT_SENSOR:

         if( iIndex >= pQstSeg->iCurrMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetCurrMonUpdateQst() )
         {
            pstStatus = &pQstSeg->stCurrMonUpdateRsp.stMonitorUpdate[pQstSeg->iCurrMonIndex[iIndex]].stMonitorStatus;
            bSuccess = TRUE;
         }

         break;

      case FAN_SPEED_SENSOR:

         if( iIndex >= pQstSeg->iFanMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetFanMonUpdateQst() )
         {
            pstStatus = &pQstSeg->stFanMonUpdateRsp.stMonitorUpdate[pQstSeg->iFanMonIndex[iIndex]].stMonitorStatus;
            bSuccess = TRUE;
         }

         break;

      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      if( bSuccess )
      {
         if( pstStatus->uMonitorStatus )
            *peHealth = pstStatus->uMonitorStatus;
         else
            *peHealth = pstStatus->uThresholdStatus;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorReading() - Returns current reading from specified sensor    */
/****************************************************************************/

BOOL APIENTRY QstGetSensorReading
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   OUT  float                       *pfReading
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pfReading || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case TEMPERATURE_SENSOR:

         if( (iIndex >= pQstSeg->iTempMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetTempMonUpdateQst() )
         {
            *pfReading = QST_TEMP_TO_FLOAT(pQstSeg->stTempMonUpdateRsp.stMonitorUpdate[pQstSeg->iTempMonIndex[iIndex]].lfCurrentReading);
            bSuccess = TRUE;
         }
         break;

      case VOLTAGE_SENSOR:

         if( iIndex >= pQstSeg->iVoltMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetVoltMonUpdateQst() )
         {
            *pfReading = QST_VOLT_TO_FLOAT(pQstSeg->stVoltMonUpdateRsp.stMonitorUpdate[pQstSeg->iVoltMonIndex[iIndex]].iCurrentVoltage);
            bSuccess = TRUE;
         }

         break;

      case CURRENT_SENSOR:

         if( iIndex >= pQstSeg->iCurrMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetCurrMonUpdateQst() )
         {
            *pfReading = QST_VOLT_TO_FLOAT(pQstSeg->stCurrMonUpdateRsp.stMonitorUpdate[pQstSeg->iCurrMonIndex[iIndex]].iCurrentCurrent);
            bSuccess = TRUE;
         }

         break;

      case FAN_SPEED_SENSOR:

         if( iIndex >= pQstSeg->iFanMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if( GetFanMonUpdateQst() )
         {
            *pfReading = (float)pQstSeg->stFanMonUpdateRsp.stMonitorUpdate[pQstSeg->iFanMonIndex[iIndex]].uCurrentSpeed;
            bSuccess = TRUE;
         }

         break;

      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstSetSensorThresholdsHigh() - Sets the high health thresholds for the   */
/* specified sensor                                                         */
/****************************************************************************/

BOOL APIENTRY QstSetSensorThresholdsHigh
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   IN   float                       fNonCritical,
   IN   float                       fCritical,
   IN   float                       fNonRecoverable
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( iIndex < 0 )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case TEMPERATURE_SENSOR:

         if( (iIndex >= pQstSeg->iTempMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stTempMonThresh[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stTempMonThresh[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stTempMonThresh[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetTempThreshQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case VOLTAGE_SENSOR:

         if( iIndex >= pQstSeg->iVoltMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stVoltMonThreshHigh[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stVoltMonThreshHigh[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stVoltMonThreshHigh[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetVoltThreshHighQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case CURRENT_SENSOR:

         if( iIndex >= pQstSeg->iCurrMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stCurrMonThreshHigh[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stCurrMonThreshHigh[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stCurrMonThreshHigh[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetCurrThreshHighQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case FAN_SPEED_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstSetSensorThresholdsLow() - Sets the low health thresholds for the     */
/* specified sensor                                                         */
/****************************************************************************/

BOOL APIENTRY QstSetSensorThresholdsLow
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   IN   float                       fNonCritical,
   IN   float                       fCritical,
   IN   float                       fNonRecoverable
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( iIndex < 0 )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case VOLTAGE_SENSOR:

         if( (iIndex >= pQstSeg->iVoltMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stVoltMonThreshLow[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stVoltMonThreshLow[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stVoltMonThreshLow[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetVoltThreshLowQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case CURRENT_SENSOR:

         if( (iIndex >= pQstSeg->iCurrMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stCurrMonThreshLow[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stCurrMonThreshLow[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stCurrMonThreshLow[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetCurrThreshLowQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case FAN_SPEED_SENSOR:

         if( iIndex >= pQstSeg->iFanMons )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         if(    (pQstSeg->stFanMonThresh[iIndex].fNonCritical    == fNonCritical   )
             && (pQstSeg->stFanMonThresh[iIndex].fCritical       == fCritical      )
             && (pQstSeg->stFanMonThresh[iIndex].fNonRecoverable == fNonRecoverable) )
            bSuccess = TRUE;
         else
            bSuccess = SetFanThreshQst( iIndex, fNonCritical, fCritical, fNonRecoverable );

         break;

      case TEMPERATURE_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstSensorThresholdsHighChanged() - Returns an indication of whether or   */
/* not the specified sensor's high health thresholds have been changed      */
/* since the specified time.                                                */
/****************************************************************************/

BOOL APIENTRY QstSensorThresholdsHighChanged
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   IN   time_t                      tLastUpdate,
   OUT  BOOL                        *pbUpdated
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pbUpdated || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case TEMPERATURE_SENSOR:

         if( (iIndex >= pQstSeg->iTempMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeTempMonThreshChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case VOLTAGE_SENSOR:

         if( (iIndex >= pQstSeg->iVoltMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeVoltMonThreshHighChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case CURRENT_SENSOR:

         if( (iIndex >= pQstSeg->iCurrMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeCurrMonThreshHighChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case FAN_SPEED_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstSensorThresholdsLowChanged() - Returns an indication of whether or    */
/* not the specified sensor's low health thresholds have been changed since */
/* the specified time.                                                      */
/****************************************************************************/

BOOL APIENTRY QstSensorThresholdsLowChanged
(
   IN   QST_SENSOR_TYPE             eType,
   IN   int                         iIndex,
   IN   time_t                      tLastUpdate,
   OUT  BOOL                        *pbUpdated
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pbUpdated || (iIndex < 0) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      switch( eType )
      {
      case VOLTAGE_SENSOR:

         if( (iIndex >= pQstSeg->iVoltMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeVoltMonThreshLowChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case CURRENT_SENSOR:

         if( (iIndex >= pQstSeg->iCurrMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeCurrMonThreshLowChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case FAN_SPEED_SENSOR:

         if( (iIndex >= pQstSeg->iFanMons) )
         {

#ifdef __WIN32__
            SetLastError( ERROR_INVALID_PARAMETER );
#else
            errno = EINVAL;
#endif

            break;
         }

         *pbUpdated = (tLastUpdate <= pQstSeg->tTimeFanMonThreshChanged[iIndex]);
         bSuccess = TRUE;
         break;

      case TEMPERATURE_SENSOR:
      default:

#ifdef __WIN32__
         SetLastError( ERROR_INVALID_PARAMETER );
#else
         errno = EINVAL;
#endif

         break;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetControllerCount() - Returns the number of Fan Controllers          */
/****************************************************************************/

BOOL APIENTRY QstGetControllerCount
(
   OUT int                          *piCount
){
   // Handle obvious parameters issues

   if( !piCount )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   *piCount = pQstSeg->iFanCtrls;
   return( TRUE );
}


/****************************************************************************/
/* QstGetControllerConfiguration() - Returns the configuration of (the      */
/* usage indicator for) the specified Fan Controller                        */
/****************************************************************************/

BOOL APIENTRY QstGetControllerConfiguration
(
   IN   int                         iIndex,
   OUT  QST_FUNCTION                *peFunction
){
   // Handle obvious parameters issues

   if( !peFunction || (iIndex < 0) || (iIndex >= pQstSeg->iFanCtrls) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   *peFunction = (QST_FUNCTION)pQstSeg->stFanCtrlConfigRsp[iIndex].byControllerUsage;
   return( TRUE );
}

/****************************************************************************/
/* QstGetControllerState() - Returns the current state for the specified    */
/* Fan Controller                                                           */
/****************************************************************************/

BOOL APIENTRY QstGetControllerState
(
   IN   int                         iIndex,
   OUT  QST_HEALTH                  *peHealth,
   OUT  QST_CONTROL_STATE           *peControl
){
   BOOL                             bSuccess = FALSE;
   QST_FAN_CTRL_STATUS              *pstStatus;

   // Handle obvious parameters issues

   if( !peHealth || !peControl || (iIndex < 0) || (iIndex >= pQstSeg->iFanCtrls) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      if( GetFanCtrlUpdateQst() )
      {
         pstStatus = &pQstSeg->stFanCtrlUpdateRsp.stControllerUpdate[pQstSeg->iFanCtrlIndex[iIndex]].stControllerStatus;
         *peHealth = pstStatus->uControllerStatus;

         if( pstStatus->bOverrideSoftware )
            *peControl = CONTROL_OVERRIDE_SOFTWARE;
         else if( pstStatus->bOverrideFanController )
            *peControl = CONTROL_OVERRIDE_CONTROLLER_ERROR;
         else if( pstStatus->bOverrideTemperatureSensor)
            *peControl = CONTROL_OVERRIDE_SENSOR_ERROR;
         else
            *peControl = CONTROL_NORMAL;

         bSuccess = TRUE;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetControllerDutyCycle() - Returns the current duty cycle setting     */
/* for the specified Fan Controller                                         */
/****************************************************************************/

BOOL APIENTRY QstGetControllerDutyCycle
(
   IN   int                         iIndex,
   OUT  float                       *pfDuty
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pfDuty || (iIndex < 0) || (iIndex >= pQstSeg->iFanCtrls) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      if( GetFanCtrlUpdateQst() )
      {
         *pfDuty = QST_DUTY_TO_FLOAT( pQstSeg->stFanCtrlUpdateRsp.stControllerUpdate[pQstSeg->iFanCtrlIndex[iIndex]].uCurrentDutyCycle );
         bSuccess = TRUE;
      }

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetPollingInterval() - Returns the current Polling Interval           */
/****************************************************************************/

BOOL APIENTRY QstGetPollingInterval
(
   OUT  DWORD                       *pdwInterval
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pdwInterval )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      *pdwInterval = pQstSeg->dwPollingInterval;
      bSuccess = TRUE;

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstSetPollingInterval() - Sets the Polling Interval to the specified     */
/* value                                                                    */
/****************************************************************************/

BOOL APIENTRY QstSetPollingInterval
(
   IN   DWORD                       dwInterval
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( (dwInterval < 250 ) || (dwInterval > 10000) )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      if( pQstSeg->dwPollingInterval == dwInterval )
         bSuccess = TRUE;
      else
         bSuccess = UpdatePollingInterval( dwInterval );

      EndCriticalSection();
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstPollingIntervalChanged() - Returns an indication of whether or not    */
/* the Polling Interval has changed since the specified time                */
/****************************************************************************/

BOOL APIENTRY QstPollingIntervalChanged
(
   IN   time_t                      tLastUpdate,
   OUT  BOOL                        *pbUpdated
){
   BOOL                             bSuccess = FALSE;

   // Handle obvious parameters issues

   if( !pbUpdated )
   {

#ifdef __WIN32__
      SetLastError( ERROR_INVALID_PARAMETER );
#else
      errno = EINVAL;
#endif

      return( FALSE );
   }

   // Handle errors during library initialization

   if( !pQstSeg )
   {

#ifdef __WIN32__
      SetLastError( dwInitError );
#else
      errno = iInitErrno;
#endif

      return( FALSE );
   }

   // Process request

   if( BeginCriticalSection() )
   {
      *pbUpdated = (tLastUpdate <= pQstSeg->tTimePollingIntervalChanged);
      bSuccess = TRUE;

      EndCriticalSection();
   }

   return( bSuccess );
}
