/****************************************************************************/
/*                                                                          */
/*  Module:         QstInst.c                                               */
/*                                                                          */
/*  Description:    Implements functions allowing the QST  Instrumentation  */
/*                  (QstInst) DLL to be used with the simplicity of static  */
/*                  DLL  loading  yet  actually loads the DLL dymanically.  */
/*                  The QST Instrumentation DLL provides applications with  */
/*                  support for obtaining information  about  the  various  */
/*                  Sensors  and Fan Speed Controllers being maintained by  */
/*                  the QST Subsystem.                                      */
/*                                                                          */
/*  Notes:      1.  This is helper module, designed to support the dynamic  */
/*                  linking  of  programs  with the QstInst DLL. It is NOT  */
/*                  needed for programs that statically link to the DLL.    */
/*                                                                          */
/*              2.  In order to use this module from your application, you  */
/*                  need to do the following:                               */
/*                                                                          */
/*                  a.  #define DYNAMIC_DLL_LOADING  before  you  #include  */
/*                      header file QstInst.h.                              */
/*                                                                          */
/*                  b.  Invoke function QstInstInitialize(), in  order  to  */
/*                      get the DLL loaded into your application's address  */
/*                      space.                                              */
/*                                                                          */
/*                  c.  Invoke the various functions suported by the DLL.   */
/*                                                                          */
/*                  d.  When done,  call function QstInstCleanup() to have  */
/*                      the DLL unloaded.                                   */
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

#include <windows.h>
#include "QstInst.h"

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static BOOL                                    bQstInstDLL = FALSE;
static HMODULE                                 hQstInstDLL = NULL;

static DWORD                                   dwLoadError = ERROR_INVALID_DLL;

static PFN_QST_GET_SENSOR_COUNT                pfQstGetSensorCount;
static PFN_QST_GET_SENSOR_CONFIGURATION        pfQstGetSensorConfiguration;
static PFN_QST_GET_SENSOR_THRESHOLDS_HIGH      pfQstGetSensorThresholdsHigh;
static PFN_QST_GET_SENSOR_THRESHOLDS_LOW       pfQstGetSensorThresholdsLow;
static PFN_QST_GET_SENSOR_HEALTH               pfQstGetSensorHealth;
static PFN_QST_GET_SENSOR_READING              pfQstGetSensorReading;
static PFN_QST_SET_SENSOR_THRESHOLDS_HIGH      pfQstSetSensorThresholdsHigh;
static PFN_QST_SENSOR_THRESHOLDS_HIGH_CHANGED  pfQstSensorThresholdsHighChanged;
static PFN_QST_SET_SENSOR_THRESHOLDS_LOW       pfQstSetSensorThresholdsLow;
static PFN_QST_SENSOR_THRESHOLDS_LOW_CHANGED   pfQstSensorThresholdsLowChanged;
static PFN_QST_GET_CONTROLLER_COUNT            pfQstGetControllerCount;
static PFN_QST_GET_CONTROLLER_CONFIGURATION    pfQstGetControllerConfiguration;
static PFN_QST_GET_CONTROLLER_STATE            pfQstGetControllerState;
static PFN_QST_GET_CONTROLLER_DUTY_CYCLE       pfQstGetControllerDutyCycle;
static PFN_QST_GET_POLLING_INTERVAL            pfQstGetPollingInterval;
static PFN_QST_SET_POLLING_INTERVAL            pfQstSetPollingInterval;
static PFN_QST_POLLING_INTERVAL_CHANGED        pfQstPollingIntervalChanged;

/****************************************************************************/
/* QstInstInitialize() - Initializes support for using the QstInst DLL      */
/****************************************************************************/

BOOL QstInstInitialize( void )
{
   // Load the Instrumentation Layer DLL

   hQstInstDLL = LoadLibrary( QST_INST_DLL );

   if( hQstInstDLL )
   {
      // Build pointers to the Instrumentation Layer DLL's functions

      pfQstGetSensorCount              = (PFN_QST_GET_SENSOR_COUNT)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_COUNT) );
      pfQstGetSensorConfiguration      = (PFN_QST_GET_SENSOR_CONFIGURATION)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_CONFIGURATION) );
      pfQstGetSensorThresholdsHigh     = (PFN_QST_GET_SENSOR_THRESHOLDS_HIGH)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_THRESHOLDS_HIGH) );
      pfQstGetSensorThresholdsLow      = (PFN_QST_GET_SENSOR_THRESHOLDS_LOW)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_THRESHOLDS_LOW) );
      pfQstGetSensorHealth             = (PFN_QST_GET_SENSOR_HEALTH)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_HEALTH) );
      pfQstGetSensorReading            = (PFN_QST_GET_SENSOR_READING)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_SENSOR_READING) );
      pfQstSetSensorThresholdsHigh     = (PFN_QST_SET_SENSOR_THRESHOLDS_HIGH)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_SET_SENSOR_THRESHOLDS_HIGH) );
      pfQstSensorThresholdsHighChanged = (PFN_QST_SENSOR_THRESHOLDS_HIGH_CHANGED)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_SENSOR_THRESHOLDS_HIGH_CHANGED) );
      pfQstSetSensorThresholdsLow      = (PFN_QST_SET_SENSOR_THRESHOLDS_LOW)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_SET_SENSOR_THRESHOLDS_LOW) );
      pfQstSensorThresholdsLowChanged  = (PFN_QST_SENSOR_THRESHOLDS_LOW_CHANGED)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_SENSOR_THRESHOLDS_LOW_CHANGED) );
      pfQstGetControllerCount          = (PFN_QST_GET_CONTROLLER_COUNT)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_CONTROLLER_COUNT) );
      pfQstGetControllerConfiguration  = (PFN_QST_GET_CONTROLLER_CONFIGURATION)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_CONTROLLER_CONFIGURATION) );
      pfQstGetControllerState          = (PFN_QST_GET_CONTROLLER_STATE)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_CONTROLLER_STATE) );
      pfQstGetControllerDutyCycle      = (PFN_QST_GET_CONTROLLER_DUTY_CYCLE)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_CONTROLLER_DUTY_CYCLE) );
      pfQstGetPollingInterval          = (PFN_QST_GET_POLLING_INTERVAL)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_GET_POLLING_INTERVAL) );
      pfQstSetPollingInterval          = (PFN_QST_SET_POLLING_INTERVAL)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_SET_POLLING_INTERVAL) );
      pfQstPollingIntervalChanged      = (PFN_QST_POLLING_INTERVAL_CHANGED)GetProcAddress( hQstInstDLL, MAKEINTRESOURCE(QST_ORD_POLLING_INTERVAL_CHANGED) );

      // Verify success of pointer build

      if(    pfQstGetSensorCount
          && pfQstGetSensorConfiguration
          && pfQstGetSensorThresholdsHigh
          && pfQstGetSensorThresholdsLow
          && pfQstGetSensorHealth
          && pfQstGetSensorReading
          && pfQstSetSensorThresholdsHigh
          && pfQstSensorThresholdsHighChanged
          && pfQstSetSensorThresholdsLow
          && pfQstSensorThresholdsLowChanged
          && pfQstGetControllerCount
          && pfQstGetControllerConfiguration
          && pfQstGetControllerState
          && pfQstGetControllerDutyCycle
          && pfQstGetPollingInterval
          && pfQstSetPollingInterval
          && pfQstPollingIntervalChanged
        )
         return( bQstInstDLL = TRUE );

      // failed, so start unwinding things

      dwLoadError = GetLastError();

      FreeLibrary( hQstInstDLL );
      hQstInstDLL = NULL;
   }
   else
      dwLoadError = GetLastError();

   return( bQstInstDLL = FALSE );
}

/****************************************************************************/
/* QstInstCleanup() - Cleans up the resources supporting use of the         */
/* Instrumentation Layer.                                                   */
/****************************************************************************/

void QstInstCleanup( void )
{
   if( bQstInstDLL )
   {
      FreeLibrary( hQstInstDLL );

      bQstInstDLL = FALSE;
      hQstInstDLL = NULL;
   }
}

/****************************************************************************/
/* Function Wrappers                                                        */
/****************************************************************************/

BOOL APIENTRY QstGetSensorCount( QST_SENSOR_TYPE eType, int *piCount )
{
   if( bQstInstDLL )
      return( pfQstGetSensorCount( eType, piCount ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetSensorConfiguration( QST_SENSOR_TYPE eType, int iIndex, QST_FUNCTION *peFunction, BOOL *pbRelative, float *pfNominal )
{
   if( bQstInstDLL )
      return( pfQstGetSensorConfiguration( eType, iIndex, peFunction, pbRelative, pfNominal ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetSensorThresholdsHigh( QST_SENSOR_TYPE eType, int iIndex, float *pfNonCritical, float *pfCritical, float *pfNonRecoverable )
{
   if( bQstInstDLL )
      return( pfQstGetSensorThresholdsHigh( eType, iIndex, pfNonCritical, pfCritical, pfNonRecoverable ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetSensorThresholdsLow( QST_SENSOR_TYPE eType, int iIndex, float *pfNonCritical, float *pfCritical, float *pfNonRecoverable )
{
   if( bQstInstDLL )
      return( pfQstGetSensorThresholdsLow( eType, iIndex, pfNonCritical, pfCritical, pfNonRecoverable ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetSensorHealth( QST_SENSOR_TYPE eType, int iIndex, QST_HEALTH *peHealth )
{
   if( bQstInstDLL )
      return( pfQstGetSensorHealth( eType, iIndex, peHealth ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetSensorReading( QST_SENSOR_TYPE eType, int iIndex, float *pfReading )
{
   if( bQstInstDLL )
      return( pfQstGetSensorReading( eType, iIndex, pfReading ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstSetSensorThresholdsHigh( QST_SENSOR_TYPE eType, int iIndex, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( bQstInstDLL )
      return( pfQstSetSensorThresholdsHigh( eType, iIndex, fNonCritical, fCritical, fNonRecoverable ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstSensorThresholdsHighChanged( QST_SENSOR_TYPE eType, int iIndex, time_t tLast, BOOL *pbUpdated )
{
   if( bQstInstDLL )
      return( pfQstSensorThresholdsHighChanged( eType, iIndex, tLast, pbUpdated ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstSetSensorThresholdsLow( QST_SENSOR_TYPE eType, int iIndex, float fNonCritical, float fCritical, float fNonRecoverable )
{
   if( bQstInstDLL )
      return( pfQstSetSensorThresholdsLow( eType, iIndex, fNonCritical, fCritical, fNonRecoverable ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstSensorThresholdsLowChanged( QST_SENSOR_TYPE eType, int iIndex, time_t tLast, BOOL *pbUpdated )
{
   if( bQstInstDLL )
      return( pfQstSensorThresholdsLowChanged( eType, iIndex, tLast, pbUpdated ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetControllerCount( int *piCount )
{
   if( bQstInstDLL )
      return( pfQstGetControllerCount( piCount ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetControllerConfiguration( int iIndex, QST_FUNCTION *peFunction )
{
   if( bQstInstDLL )
      return( pfQstGetControllerConfiguration( iIndex, peFunction ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetControllerState( int iIndex, QST_HEALTH *peHealth, QST_CONTROL_STATE *peState )
{
   if( bQstInstDLL )
      return( pfQstGetControllerState( iIndex, peHealth, peState ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetControllerDutyCycle( int iIndex, float *pfDuty )
{
   if( bQstInstDLL )
      return( pfQstGetControllerDutyCycle( iIndex, pfDuty ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstGetPollingInterval( DWORD *pdwInterval )
{
   if( bQstInstDLL )
      return( pfQstGetPollingInterval( pdwInterval ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstSetPollingInterval( DWORD dwInterval )
{
   if( bQstInstDLL )
      return( pfQstSetPollingInterval( dwInterval ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

BOOL APIENTRY QstPollingIntervalChanged( time_t tLast, BOOL *pbUpdated )
{
   if( bQstInstDLL )
      return( pfQstPollingIntervalChanged( tLast, pbUpdated ) );
   else
   {
      SetLastError( dwLoadError );
      return( FALSE );
   }
}

