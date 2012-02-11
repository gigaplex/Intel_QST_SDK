/****************************************************************************/
/*                                                                          */
/*  Module:         QstProxyComm.c                                          */
/*                                                                          */
/*  Author:         N. Scott Pearson                                        */
/*                                                                          */
/*  Date Written:   August 10, 2005                                         */
/*                                                                          */
/*  Description:    Primary module for a DLL  that  provides  support  for  */
/*                  Windows  applications to communicate with the Intel(R)  */
/*                  Quiet System Technology (QST) Subsystem  via  a  Proxy  */
/*                  Service.                                                */
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

#include "QstCmd.h"
#include "QstCmdLeg.h"
#include "LegTranslationFuncs.h"
#include "QstComm.h"
#include "QstProxyComm.h"

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

static QST_PROXY_COMM_SEG   *pQstCommSeg;

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

         pQstCommSeg = (QST_PROXY_COMM_SEG *)MapViewOfFile( hSegment, FILE_MAP_WRITE, 0, 0, 0 );

         if( pQstCommSeg )
         {
            // Obtain access to the QST Proxy Service's commuications mutex

            hMutex = OpenMutex( MUTEX_ALL_ACCESS, TRUE, QST_COMM_MUTEX );

            if( hMutex )
            {
               // We're good to go!

               return( bAttached = TRUE );
            }

            UnmapViewOfFile( pQstCommSeg );
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

   UnmapViewOfFile( pQstCommSeg );

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
/* CheckRspSuccess () - Checks the status of the command in the response    */
/* buffer.  Returns TRUE for success and FALSE for all other values.  This  */
/* function must take into account the version of the firmware it is        */
/* communicating with.                                                      */
/****************************************************************************/

static BOOL CheckRspSuccess( void *pvRspBuf )
{
   // Just use a hack for now that checks the status. We know that all status
   // information is the first byte of the response packet.

   return( ( *((UINT8*)pvRspBuf) == QST_CMD_SUCCESSFUL )? TRUE : FALSE );
}

/****************************************************************************/
/* CommonCmdHandler() - Common code used to pass commands and obtain any    */
/* responses from the QST subsystem. This code MUST be compatible with any  */
/* revision of the QST firmware.                                            */
/****************************************************************************/

BOOL CommonCmdHandler (
   IN    void     *pvCmdBuf,
   IN    size_t   tCmdSize,
   OUT   void     *pvRspBuf,
   IN    size_t   tRspSize
   )
{
   BOOL bSuccess;

   // Make sure we are attached to the Proxy Service

   if( !CheckAttached() )
      return( FALSE );

   // Obtain exclusive access to the communications segment

   switch( WaitForSingleObject( hMutex, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      break;

   default:

      return( FALSE );
   }

   // Place the command into the segment

   strcpy( pQstCommSeg->tszRspSem, tszSemaphoreName );
   memcpy( pQstCommSeg->byCmdRsp, pvCmdBuf, tCmdSize );

   pQstCommSeg->wLength = (UINT16)tCmdSize;

   // Signal the QST Proxy Service to process the message

   if( bPseudoService )
   {
      PostMessage( hPseudoService, WM_PSEUDO_SERVICE_COMMAND, 0, EXEC_QST_COMMAND );
   }
   else
   {
      SERVICE_STATUS stStatus;

      if( !ControlService( hService, EXEC_QST_COMMAND, &stStatus ) )
      {
         ReleaseMutex( hMutex );
         return( FALSE );
      }
   }

   // Wait for the QST Proxy Service to signal completion

   switch( WaitForSingleObject( hSemaphore, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      if( (pQstCommSeg->wLength != tRspSize) && CheckRspSuccess( pQstCommSeg->byCmdRsp ) )
      {
         // Data received is larger/smaller than expected

         SetLastError( ERROR_BAD_LENGTH );
         bSuccess = FALSE;
      }
      else
      {
         // Data received is right length or QST rejected command; place info available in caller's buffer

         memcpy( pvRspBuf, pQstCommSeg->byCmdRsp, __min(pQstCommSeg->wLength, tRspSize) );
         bSuccess = TRUE;
      }

      break;

   default:

      bSuccess = FALSE;
      break;
   }

   ReleaseMutex( hMutex );
   return( bSuccess );
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
/* QSTCommand() - Sends command to the QST Subsystem and awaits response    */
/****************************************************************************/

BOOL APIENTRY QstCommand(

   IN  void                         *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                       tCmdSize,           // Size of command packet
   OUT void                         *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                       tRspSize            // Expected size of response packet
){
   P_QST_LEG_SST_PASS_THROUGH_CMD   pstQstCmd = (P_QST_LEG_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                                        // For structured access to command packet

   //
   // Initialize Subsystem Information structure
   //
   if (!GetSubsystemInformation ())
   {
      SetLastError( ERROR_BAD_DEVICE );
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   // Validate command buffer size

   if (tCmdSize < sizeof(QST_LEG_CMD_HEADER))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      return FALSE;
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LEG_LAST_CMD_CODE )
   {
      SetLastError( ERROR_BAD_COMMAND );
      return( FALSE );
   }

   // Verify Command/Response Lengths

   if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_LEG_CMD_HEADER)))
       || (tRspSize != pstQstCmd->stHeader.wResponseLength) )
   {
      SetLastError( ERROR_BAD_FORMAT );
      return( FALSE );
   }

   // If SST Pass-through, verify Command/Response Lengths for SST Command

   if(    (pstQstCmd->stHeader.byCommand == QST_LEG_SST_PASS_THROUGH)
       && (    (pstQstCmd->stHeader.wCommandLength  != (pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
            || (pstQstCmd->stHeader.wResponseLength != (pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) ) )
   {
      SetLastError( ERROR_BAD_FORMAT );
      return( FALSE );
   }

   //
   // Determine if sending to a QST 2.x ME firmware...
   //
   if (TranslationToNewRequired ())
   {
      //
      // Target is QST 1.x ME firmware so translate command to legacy command set.
      //
      switch (TranslateToNewCommand (pvCmdBuf, tCmdSize, pvRspBuf, tRspSize))
      {
      case TRANSLATE_CMD_SUCCESS:
         return TRUE;

      case TRANSLATE_CMD_INVALID_PARAMETER:
         SetLastError (ERROR_INVALID_PARAMETER);
         return FALSE;

      case TRANSLATE_CMD_BAD_COMMAND:
         SetLastError (ERROR_BAD_COMMAND);
         return FALSE;

      case TRANSLATE_CMD_NOT_ENOUGH_MEMORY:
         SetLastError (ERROR_NOT_ENOUGH_MEMORY);
         return FALSE;

      case TRANSLATE_CMD_FAILED_WITH_ERROR_SET:
         return FALSE;

      default:
         return FALSE;
      }
   }

   //
   // Call common command handler (Calls SetLastError before exit if failed)
   //
   return CommonCmdHandler (pvCmdBuf, tCmdSize, pvRspBuf, tRspSize);
}


/****************************************************************************/
/* QSTCommand2() - Sends command to the QST Subsystem and awaits response    */
/****************************************************************************/

BOOL APIENTRY QstCommand2(
   IN  void       *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t     tCmdSize,           // Size of command packet
   OUT void       *pvRspBuf,          // Address of buffer for response packet
   IN  size_t     tRspSize            // Expected size of response packet
   )
{
   P_QST_SST_PASS_THROUGH_CMD   pstQstCmd = (P_QST_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                      // For structured access to command packet

   //
   // Initialize Subsystem Information structure
   //
   if (!GetSubsystemInformation ())
   {
      SetLastError( ERROR_BAD_DEVICE );
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
      SetLastError( ERROR_INVALID_PARAMETER );
      return( FALSE );
   }

   // Validate command buffer size

   if (tCmdSize < sizeof(QST_CMD_HEADER))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      return FALSE;
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LAST_CMD_CODE )
   {
      SetLastError( ERROR_BAD_COMMAND );
      return( FALSE );
   }

   // Verify Command/Response Lengths

   if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_CMD_HEADER)))
       || (tRspSize != pstQstCmd->stHeader.wResponseLength) )
   {
      SetLastError( ERROR_BAD_FORMAT );
      return( FALSE );
   }

   // If SST Pass-through, verify Command/Response Lengths for SST Command

   if(    (pstQstCmd->stHeader.byCommand == QST_SST_PASS_THROUGH)
       && (    (pstQstCmd->stHeader.wCommandLength  != (pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
            || (pstQstCmd->stHeader.wResponseLength != (pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) ) )
   {
      SetLastError( ERROR_BAD_FORMAT );
      return( FALSE );
   }

   //
   // Determine if sending to a QST 1.x ME firmware...
   //
   if (TranslationToLegacyRequired ())
   {
      //
      // Target is QST 1.x ME firmware so translate command to legacy command set.
      //
      switch (TranslateToLegacyCommand (pvCmdBuf, tCmdSize, pvRspBuf, tRspSize))
      {
      case TRANSLATE_CMD_SUCCESS:
         return TRUE;

      case TRANSLATE_CMD_INVALID_PARAMETER:
         SetLastError (ERROR_INVALID_PARAMETER);
         return FALSE;

      case TRANSLATE_CMD_BAD_COMMAND:
         SetLastError (ERROR_BAD_COMMAND);
         return FALSE;

      case TRANSLATE_CMD_NOT_ENOUGH_MEMORY:
         SetLastError (ERROR_NOT_ENOUGH_MEMORY);
         return FALSE;

      case TRANSLATE_CMD_FAILED_WITH_ERROR_SET:
         return FALSE;

      default:
         return FALSE;
      }
   }

   //
   // Call common command handler (Calls SetLastError before exit if failed)
   //
   return CommonCmdHandler (pvCmdBuf, tCmdSize, pvRspBuf, tRspSize);
}
