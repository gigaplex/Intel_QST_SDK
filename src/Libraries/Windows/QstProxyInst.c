/****************************************************************************/
/*                                                                          */
/*  Module:         QstProxyInst.c                                          */
/*                                                                          */
/*  Description:    Primary  module  for  a  DLL  that  provides   Windows  */
/*                  applications  with instrumented access the sensors and  */
/*                  controllers  managed  by  the  Intel(R)  Quiet  System  */
/*                  Technology (QST) Subsystem via a Proxy Service.         */
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
#include <ctype.h>
#include <errno.h>

#pragma warning(disable: 4201 4701)

#include <windows.h>
#include <process.h>

#include "QstInst.h"
#include "QstProxyInst.h"

/****************************************************************************/
/****************************************************************************/
/****************** Proxy Service Data Access/Communication *****************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static BOOL                 bAttached;

static BOOL                 bPseudoService;
static HWND                 hPseudoService;

static SC_HANDLE            hServiceManager;
static SC_HANDLE            hService;

static HANDLE               hSegment;
static HANDLE               hMutex;
static HANDLE               hSemaphore;

static QST_PROXY_INST_SEG   *pQstInstSeg;

static TCHAR                tszSemaphoreName[65];

/****************************************************************************/
/* OpenQstService() - Opens connection to QST Proxy Service                 */
/****************************************************************************/

static BOOL OpenQstService( void )
{
   hServiceManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

   if( hServiceManager )
   {
      hService = OpenService( hServiceManager, QST_SERVICE_NAME, SERVICE_ALL_ACCESS );

      if( hService )
         return( TRUE );

      CloseServiceHandle( hServiceManager );
   }

   return( FALSE );
}

/****************************************************************************/
/* CloseQSTService() - Closes connection to QST Proxy Service               */
/****************************************************************************/

static void CloseQstService( void )
{
   CloseServiceHandle( hService );
   CloseServiceHandle( hServiceManager );
}

/****************************************************************************/
/* OpenQSTPseudoService() - Opens connection to QST Proxy Service that is   */
/* running as a Psuedo-Service                                              */
/****************************************************************************/

static BOOL OpenQstPseudoService( void )
{
   hPseudoService = FindWindow( QST_SERVICE_WINDOW, QST_SERVICE_NAME );
   return( (hPseudoService)? TRUE : FALSE );
}

/****************************************************************************/
/* CloseQSTPseudoService() - Closes connection to QST Proxy Service that is */
/* running as a Psuedo-Service                                              */
/****************************************************************************/

static void CloseQstPseudoService( void )
{
   CloseHandle( hPseudoService );
}

/****************************************************************************/
/* Initialize() - Initializes communications between a particular process   */
/* and the QST Proxy Service. This includes establishing a notification     */
/* path between the process and the QST Proxy Service, establishing access  */
/* to the global memory block used to share command and QST data between    */
/* them and establishing access to the mutex that will be used to provide   */
/* us with exclusive access to this global memory block.                    */
/****************************************************************************/

static BOOL Initialize( void )
{
   SECURITY_DESCRIPTOR sd;

   // Open a connection to the QST Proxy Service

   if( OpenQstPseudoService() )
      bPseudoService = TRUE;
   else if( OpenQstService() )
      bPseudoService = FALSE;
   else
      return( FALSE );

   // Setup security for allocated objects

   InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
   SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

   // Allocate a semaphore for response signalling

   _stprintf( tszSemaphoreName, __TEXT("Global\\QSTSem%X"), (DWORD)time( NULL ) );
   hSemaphore = CreateSemaphore( NULL, 0, 1, tszSemaphoreName );

   if( hSemaphore )
   {
      SetKernelObjectSecurity( hSemaphore, DACL_SECURITY_INFORMATION, &sd );

      // Obtain access to the QST Proxy Service's communications buffer

      hSegment = OpenFileMapping( FILE_MAP_WRITE, TRUE, QST_COMM_SEG_NAME );

      if( hSegment )
      {
         // Map communications buffer into our process space

         pQstInstSeg = (QST_PROXY_INST_SEG *)MapViewOfFile( hSegment, FILE_MAP_WRITE, 0, 0, 0 );

         if( pQstInstSeg )
         {
            // Obtain access to the QST Proxy Service's commuications mutex

            hMutex = OpenMutex( MUTEX_ALL_ACCESS, TRUE, QST_COMM_MUTEX );

            if( hMutex )
            {
               // We're good to go!

               return( bAttached = TRUE );
            }

            UnmapViewOfFile( pQstInstSeg );
         }

         CloseHandle( hSegment );
      }

      CloseHandle( hSemaphore );
   }

   if( bPseudoService )
      CloseQstPseudoService();
   else
      CloseQstService();

   return( bAttached = FALSE );
}

/****************************************************************************/
/* Cleanup() - Frees up the resources used to facilitate communications.    */
/****************************************************************************/

static void Cleanup( void )
{
   if( bPseudoService )
      CloseQstPseudoService();
   else
      CloseQstService();

   UnmapViewOfFile( pQstInstSeg );

   CloseHandle( hMutex );
   CloseHandle( hSegment );
   CloseHandle( hSemaphore );

   bAttached = FALSE;
}

/****************************************************************************/
/* CheckAttached() - Checks if we have an attachment with the Service and,  */
/* if we don't, attempts to create one. We try doing this for up to 60      */
/* seconds before giving up, in case we are being used from a Service and   */
/* are in an initialization race condition with the QST Proxy Service.      */
/****************************************************************************/

static BOOL CheckAttached( void )
{
   if( !bAttached )
   {
      BOOL iIndex;

      for( iIndex = 0; iIndex < 60; iIndex++ )
      {
         if( Initialize() )
            break;

         Sleep( 1000 );
      }
   }

   if( !bAttached )
      SetLastError( ERROR_SERVICE_NOT_ACTIVE );

   return( bAttached );
}

/****************************************************************************/
/* BeginCritical() - Ensures we have access to QST Proxy Service and gets   */
/* us exclusive access to the communication segment.                        */
/****************************************************************************/

static BOOL BeginCritical( void )
{
   // Make sure we are attached to the Proxy Service

   if( !CheckAttached() )
      return( FALSE );

   // Obtain exclusive access to the communications segment

   switch( WaitForSingleObject( hMutex, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      return( TRUE );

   default:

      return( FALSE );
   }
}

/****************************************************************************/
/* EndCritical() - Releases exclusive access to the communcations segment   */
/****************************************************************************/

static void EndCritical( void )
{
   ReleaseMutex( hMutex );
}

/****************************************************************************/
/* ProxyOperation() - Requests that the QST Proxy Service perform some      */
/* specific operation and waits for its completion to be signalled.         */
/****************************************************************************/

static BOOL ProxyOperation( int iOperation )
{
   BOOL bSuccess;

   // Place signalling information into the communications segment

   strcpy( pQstInstSeg->stCommSeg.tszRspSem, tszSemaphoreName );

   // Signal the QST Subsystem to process the message

   if( bPseudoService )
   {
      PostMessage( hPseudoService, WM_PSEUDO_SERVICE_COMMAND, 0, iOperation );
   }
   else
   {
      SERVICE_STATUS stStatus;

      if( !ControlService( hService, (DWORD)iOperation, &stStatus ) )
         return( FALSE );
   }

   // Wait for the QST Subsystem to signal completion

   switch( WaitForSingleObject( hSemaphore, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      if( pQstInstSeg->dwStatus )
      {
         SetLastError( pQstInstSeg->dwStatus );
         bSuccess = FALSE;
      }
      else
         bSuccess = TRUE;

      break;

   default:

      bSuccess = FALSE;
      break;
   }

   return( bSuccess );
}

/****************************************************************************/
/* SetThresholds() - Requests the setting of a particular threshold set     */
/****************************************************************************/

static BOOL SetThresholds( int iOperation, int iSensor, float fNonCritical,
                           float fCritical, float fNonRecoverable )
{
   pQstInstSeg->iSensor                    = iSensor;
   pQstInstSeg->stThresh.fNonCritical      = fNonCritical;
   pQstInstSeg->stThresh.fCritical         = fCritical;
   pQstInstSeg->stThresh.fNonRecoverable   = fNonRecoverable;

   return( ProxyOperation( iOperation ) );
}

/****************************************************************************/
/* SetPollingInterval() - Requests the set of the Polling Interval          */
/****************************************************************************/

static BOOL SetPollingInterval( DWORD dwPollingInterval )
{
   pQstInstSeg->dwNewPollingInterval = dwPollingInterval;

   return( ProxyOperation( SET_POLLING_INTERVAL ) );
}

/****************************************************************************/
/* DllMain() - Main entry point for the DLL. It handles process attach and  */
/* detach requests.                                                         */
/****************************************************************************/

BOOL APIENTRY DllMain( HANDLE hModule, DWORD dwReason, LPVOID pvReserved )
{
   switch( dwReason )
   {
   case DLL_PROCESS_ATTACH:

      (void)Initialize();   // this may fail; we don't care...
      break;

   case DLL_PROCESS_DETACH:

      Cleanup();
      break;
   }

   return( TRUE );

    UNREFERENCED_PARAMETER( hModule );
    UNREFERENCED_PARAMETER( pvReserved );
}





/****************************************************************************/
/****************************************************************************/
/******************************* DLL Functions ******************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* QstGetSensorCount() - Returns a count of the number of sensors of the    */
/* specified type that are being managed by the QST Subsystem               */
/****************************************************************************/

BOOL APIENTRY QstGetSensorCount(

   IN   QST_SENSOR_TYPE     eType,
   OUT  int                 *piCount
){
   BOOL                     bSuccess = TRUE;

   if( !piCount )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !CheckAttached() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      *piCount = pQstInstSeg->stDataSeg.iTempMons;
      break;

   case VOLTAGE_SENSOR:

      *piCount = pQstInstSeg->stDataSeg.iVoltMons;
       break;

   case CURRENT_SENSOR:

      *piCount = pQstInstSeg->stDataSeg.iCurrMons;
       break;

   case FAN_SPEED_SENSOR:

      *piCount = pQstInstSeg->stDataSeg.iFanMons;
      break;

   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      bSuccess = FALSE;
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorConfiguration() - Returns configuration for the specified    */
/* sensor                                                                   */
/****************************************************************************/

BOOL APIENTRY QstGetSensorConfiguration(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   OUT  QST_FUNCTION        *peFunction,
   OUT  BOOL                *pbRelative,
   OUT  float               *pfNominal
){
   BOOL                     bSuccess = FALSE;

   if( !peFunction || !pbRelative || !pfNominal || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !CheckAttached() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iTempMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *peFunction = (QST_FUNCTION)pQstInstSeg->stDataSeg.stTempMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = (BOOL)pQstInstSeg->stDataSeg.stTempMonConfigRsp[iIndex].bRelativeReadings;
      *pfNominal  = QST_TEMP_TO_FLOAT( pQstInstSeg->stDataSeg.stTempMonConfigRsp[iIndex].lfTempNominal );

      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *peFunction = (QST_FUNCTION)pQstInstSeg->stDataSeg.stVoltMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = QST_VOLT_TO_FLOAT( pQstInstSeg->stDataSeg.stVoltMonConfigRsp[iIndex].iVoltageNominal );

      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *peFunction = (QST_FUNCTION)pQstInstSeg->stDataSeg.stCurrMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = QST_CURR_TO_FLOAT( pQstInstSeg->stDataSeg.stCurrMonConfigRsp[iIndex].iCurrentNominal );

      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iFanMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *peFunction = (QST_FUNCTION)pQstInstSeg->stDataSeg.stFanMonConfigRsp[iIndex].byMonitorUsage;
      *pbRelative = FALSE;
      *pfNominal  = (float)pQstInstSeg->stDataSeg.stFanMonConfigRsp[iIndex].uSpeedNominal;

      bSuccess = TRUE;
      break;

   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorThresholdsHigh() - Returns the high health thresholds for    */
/* the specified sensor.                                                    */
/****************************************************************************/

BOOL APIENTRY QstGetSensorThresholdsHigh(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   OUT  float               *pfNonCritical,
   OUT  float               *pfCritical,
   OUT  float               *pfNonRecoverable
){
   BOOL                     bSuccess = FALSE;

   if( !pfNonCritical || !pfCritical || !pfNonRecoverable || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iTempMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorThresholdsLow() - Returns the low health thresholds for the  */
/* specified sensor.                                                        */
/****************************************************************************/

BOOL APIENTRY QstGetSensorThresholdsLow(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   OUT  float               *pfNonCritical,
   OUT  float               *pfCritical,
   OUT  float               *pfNonRecoverable
){
   BOOL                     bSuccess = FALSE;

   if( !pfNonCritical || !pfCritical || !pfNonRecoverable || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iFanMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfNonCritical    = pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fNonCritical;
      *pfCritical       = pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fCritical;
      *pfNonRecoverable = pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fNonRecoverable;

      bSuccess = TRUE;
      break;

   case TEMPERATURE_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorHealth() - Returns the health status for the specified       */
/* sensor                                                                   */
/****************************************************************************/

BOOL APIENTRY QstGetSensorHealth(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   OUT  QST_HEALTH          *peHealth
){
   BOOL                     bSuccess = FALSE;
   QST_MON_HEALTH_STATUS    *pstStatus;

   if( !peHealth || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iTempMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      pstStatus = &pQstInstSeg->stDataSeg.stTempMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iTempMonIndex[iIndex]].stMonitorStatus;
      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      pstStatus = &pQstInstSeg->stDataSeg.stVoltMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iVoltMonIndex[iIndex]].stMonitorStatus;
      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      pstStatus = &pQstInstSeg->stDataSeg.stCurrMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iCurrMonIndex[iIndex]].stMonitorStatus;
      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iFanMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      pstStatus = &pQstInstSeg->stDataSeg.stFanMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iFanMonIndex[iIndex]].stMonitorStatus;
      bSuccess = TRUE;
      break;

   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   if( bSuccess )
   {
      if( pstStatus->uMonitorStatus )
         *peHealth = pstStatus->uMonitorStatus;
      else
         *peHealth = pstStatus->uThresholdStatus;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstGetSensorReading() - Returns current reading from specified sensor    */
/****************************************************************************/

BOOL APIENTRY QstGetSensorReading(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   OUT  float               *pfReading
){
   BOOL                     bSuccess = FALSE;

   if( !pfReading || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iTempMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfReading = QST_TEMP_TO_FLOAT(pQstInstSeg->stDataSeg.stTempMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iTempMonIndex[iIndex]].lfCurrentReading);
      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfReading = QST_VOLT_TO_FLOAT(pQstInstSeg->stDataSeg.stVoltMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iVoltMonIndex[iIndex]].iCurrentVoltage);
      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfReading = QST_CURR_TO_FLOAT(pQstInstSeg->stDataSeg.stCurrMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iCurrMonIndex[iIndex]].iCurrentCurrent);
      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iFanMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pfReading = (float)pQstInstSeg->stDataSeg.stFanMonUpdateRsp.stMonitorUpdate[pQstInstSeg->stDataSeg.iFanMonIndex[iIndex]].uCurrentSpeed;
      bSuccess = TRUE;
      break;

   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstSetSensorThresholdsHigh() - Sets the high health thresholds for the   */
/* specified sensor                                                         */
/****************************************************************************/

BOOL APIENTRY QstSetSensorThresholdsHigh(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   IN   float               fNonCritical,
   IN   float               fCritical,
   IN   float               fNonRecoverable
){
   BOOL                     bSuccess = FALSE;

   if( iIndex < 0 )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iTempMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stTempMonThresh[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_TEMP_THRESH, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case VOLTAGE_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iVoltMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stVoltMonThreshHigh[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_VOLT_THRESH_HIGH, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case CURRENT_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iCurrMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stCurrMonThreshHigh[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_CURR_THRESH_HIGH, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case FAN_SPEED_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstSetSensorThresholdsLow() - Sets the low health thresholds for the     */
/* specified sensor                                                         */
/****************************************************************************/

BOOL APIENTRY QstSetSensorThresholdsLow(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   IN   float               fNonCritical,
   IN   float               fCritical,
   IN   float               fNonRecoverable
){
   BOOL                     bSuccess = FALSE;

   if( iIndex < 0 )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case VOLTAGE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iVoltMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stVoltMonThreshLow[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_VOLT_THRESH_LOW, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case CURRENT_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iCurrMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stCurrMonThreshLow[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_CURR_THRESH_LOW, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case FAN_SPEED_SENSOR:

      if( iIndex >= pQstInstSeg->stDataSeg.iFanMons )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      if(    (pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fNonCritical    == fNonCritical   )
          && (pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fCritical       == fCritical      )
          && (pQstInstSeg->stDataSeg.stFanMonThresh[iIndex].fNonRecoverable == fNonRecoverable) )
         bSuccess = TRUE;
      else
         bSuccess = SetThresholds( SET_FAN_THRESH, iIndex, fNonCritical, fCritical, fNonRecoverable );

      break;

   case TEMPERATURE_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstSensorThresholdsHighChanged() - Returns an indication of whether or   */
/* not the specified sensor's high health thresholds have been changed      */
/* since the specified time.                                                */
/****************************************************************************/

BOOL APIENTRY QstSensorThresholdsHighChanged(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   IN   time_t              tLastUpdate,
   OUT  BOOL                *pbUpdated
){
   BOOL                     bSuccess = FALSE;

   if( !pbUpdated || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case TEMPERATURE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iTempMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeTempMonThreshChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case VOLTAGE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iVoltMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeVoltMonThreshHighChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iCurrMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeCurrMonThreshHighChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstSensorThresholdsLowChanged() - Returns an indication of whether or    */
/* not the specified sensor's low health thresholds have been changed since */
/* the specified time.                                                      */
/****************************************************************************/

BOOL APIENTRY QstSensorThresholdsLowChanged(

   IN   QST_SENSOR_TYPE     eType,
   IN   int                 iIndex,
   IN   time_t              tLastUpdate,
   OUT  BOOL                *pbUpdated
){
   BOOL                     bSuccess = FALSE;

   if( !pbUpdated || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   switch( eType )
   {
   case VOLTAGE_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iVoltMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeVoltMonThreshLowChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case CURRENT_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iCurrMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeCurrMonThreshLowChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case FAN_SPEED_SENSOR:

      if( (iIndex >= pQstInstSeg->stDataSeg.iFanMons) )
      {
         SetLastError( ERROR_INVALID_PARAMETER );
         break;
      }

      *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimeFanMonThreshChanged[iIndex]);
      bSuccess = TRUE;
      break;

   case TEMPERATURE_SENSOR:
   default:

      SetLastError( ERROR_INVALID_PARAMETER );
      break;
   }

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstGetControllerCount() - Returns the number of Fan Controllers          */
/****************************************************************************/

BOOL APIENTRY QstGetControllerCount(

   OUT int                  *piCount
){
   if( !piCount )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !CheckAttached() )
      return( FALSE );

   *piCount = pQstInstSeg->stDataSeg.iFanCtrls;
   return( TRUE );
}


/****************************************************************************/
/* QstGetControllerConfiguration() - Returns the configuration of (the      */
/* usage indicator for) the specified Fan Controller                        */
/****************************************************************************/

BOOL APIENTRY QstGetControllerConfiguration(

   IN   int                 iIndex,
   OUT  QST_FUNCTION        *peFunction
){
   if( !peFunction || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !CheckAttached() )
      return( FALSE );

   if( iIndex >= pQstInstSeg->stDataSeg.iFanCtrls )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   *peFunction = (QST_FUNCTION)pQstInstSeg->stDataSeg.stFanCtrlConfigRsp[iIndex].byControllerUsage;
   return( TRUE );
}

/****************************************************************************/
/* QstGetControllerState() - Returns the current state for the specified    */
/* Fan Controller                                                           */
/****************************************************************************/

BOOL APIENTRY QstGetControllerState(

   IN   int                 iIndex,
   OUT  QST_HEALTH          *peHealth,
   OUT  QST_CONTROL_STATE   *peControl
){
   QST_FAN_CTRL_STATUS      *pstStatus;

   if( !peHealth || !peControl || (iIndex < 0) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   if( iIndex >= pQstInstSeg->stDataSeg.iFanCtrls )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   pstStatus = &pQstInstSeg->stDataSeg.stFanCtrlUpdateRsp.stControllerUpdate[pQstInstSeg->stDataSeg.iFanCtrlIndex[iIndex]].stControllerStatus;
   *peHealth = pstStatus->uControllerStatus;

   if( pstStatus->bOverrideSoftware )
      *peControl = CONTROL_OVERRIDE_SOFTWARE;
   else if( pstStatus->bOverrideFanController )
      *peControl = CONTROL_OVERRIDE_CONTROLLER_ERROR;
   else if( pstStatus->bOverrideTemperatureSensor)
      *peControl = CONTROL_OVERRIDE_SENSOR_ERROR;
   else
      *peControl = CONTROL_NORMAL;

   EndCritical();
   return( TRUE );
}

/****************************************************************************/
/* QstGetControllerDutyCycle() - Returns the current duty cycle setting     */
/* for the specified Fan Controller                                         */
/****************************************************************************/

BOOL APIENTRY QstGetControllerDutyCycle(

   IN   int                 iIndex,
   OUT  float               *pfDuty
){
   if( !pfDuty || (iIndex < 0) || (iIndex >= pQstInstSeg->stDataSeg.iFanCtrls) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   if( iIndex >= pQstInstSeg->stDataSeg.iFanCtrls )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   *pfDuty = QST_DUTY_TO_FLOAT( pQstInstSeg->stDataSeg.stFanCtrlUpdateRsp.stControllerUpdate[pQstInstSeg->stDataSeg.iFanCtrlIndex[iIndex]].uCurrentDutyCycle );

   EndCritical();
   return( TRUE );
}

/****************************************************************************/
/* QstGetPollingInterval() - Returns the current Polling Interval           */
/****************************************************************************/

BOOL APIENTRY QstGetPollingInterval
(
   OUT  DWORD               *pdwInterval
){
   if( !pdwInterval )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   *pdwInterval = pQstInstSeg->stDataSeg.dwPollingInterval;

   EndCritical();
   return( TRUE );
}

/****************************************************************************/
/* QstSetPollingInterval() - Sets the Polling Interval to the specified     */
/* value                                                                    */
/****************************************************************************/

BOOL APIENTRY QstSetPollingInterval
(
   IN   DWORD               dwInterval
){
   BOOL                     bSuccess = FALSE;

   if( (dwInterval < 250 ) || (dwInterval > 10000) )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   if( pQstInstSeg->stDataSeg.dwPollingInterval == dwInterval )
      bSuccess = TRUE;
   else
      bSuccess = SetPollingInterval( dwInterval );

   EndCritical();
   return( bSuccess );
}

/****************************************************************************/
/* QstPollingIntervalChanged() - Returns an indication of whether or not    */
/* the Polling Interval has changed since the specified time                */
/****************************************************************************/

BOOL APIENTRY QstPollingIntervalChanged
(
   IN   time_t              tLastUpdate,
   OUT  BOOL                *pbUpdated
){
   if( !pbUpdated )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   if( !BeginCritical() )
      return( FALSE );

   *pbUpdated = (tLastUpdate <= pQstInstSeg->stDataSeg.tTimePollingIntervalChanged);

   EndCritical();
   return( TRUE );
}

