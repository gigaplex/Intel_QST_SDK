
#define SINGLE_THREADED             // Define this to force module to do QST
                                    // HECI transactions one at a time...

/****************************************************************************/
/*                                                                          */
/*  Module:         QstComm.c                                               */
/*                                                                          */
/*  Description:    Provides support for Linux applications to communicate  */
/*                  with  the  Intel(R)  Quiet System Technology (QST) F/W  */
/*                  Subsystem running on the Management Engine (ME).        */
/*                                                                          */
/*  Notes:      1.  This  module  is  designed  such that it can be linked  */
/*                  directly  into  an  application  (along  with   module  */
/*                  heci.c) or it can be used as the main module  for  the  */
/*                  QstComm Shared Object (SO) File.                        */
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

#ifndef __linux__
#error This source module intended for use in Linux environments only
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/time.h>

#define INITGUID                        // Declare GUIDs locally

#include "QstCmd.h"
#include "QstCmdLeg.h"
#include "LegTranslationFuncs.h"
#include "QstComm.h"
#include "CritSect.h"
#include "heci.h"

/****************************************************************************/
/* Configuration                                                            */
/****************************************************************************/

#define ATTACH_RETRIES  5               // Attach attempts before giving up
#define ATTACH_DELAY    1000            // Delay between attach attempts

#define RETRY_COUNT     10              // Communication attempts before giving up
#define RETRY_DELAY     1000            // Delay between retries

#ifdef  SINGLE_THREADED
/****************************************************************************/
/* Definitions/Variables for single-threading                               */
/****************************************************************************/

#define CRITSECT_TYPE   0xAF5C010       // Critical Section type
static  HCRITSECT       hCritSect;      // Critical Section handle

#endif

/****************************************************************************/
/* Process-Specific Variables                                               */
/****************************************************************************/

DEFINE_GUID( QST_SUBSYSTEM_GUID, 0x6B5205B9, 0x8185, 0x4519, 0xB8, 0x89, 0xD9, 0x87, 0x24, 0xB5, 0x86, 0x07 );
                                        // GUID for QST Subsystem

static BOOL             bAttached;      // Indicates if attached to HECI driver
static int              iMaxReceive;    // Maximum receive/send length
static int              iInitErrno;     // Saved errno from initialization

/****************************************************************************/
/* Delay() - Implements an 'n' millisecond delay.                           */
/****************************************************************************/

static void Delay( int iMilliseconds )
{
   if( iMilliseconds )
   {
      struct timespec stTime;

      stTime.tv_sec   = (time_t)(iMilliseconds / 1000);
      stTime.tv_nsec  = 1000000L * (iMilliseconds % 1000);    // 1,000,000 ns = 1 ms

      while( (nanosleep( &stTime, &stTime ) == -1) && (errno == EINTR) );
   }
}

/****************************************************************************/
/* AttachDriver() - Attaches HECI driver.                                   */
/****************************************************************************/

static BOOL AttachDriver( void )
{
   int iRetries, iMax;

   for( iRetries = 0, bAttached = FALSE; iRetries < ATTACH_RETRIES; iRetries++ )
   {
      iMaxReceive = HeciConnect( &QST_SUBSYSTEM_GUID );

      if( iMaxReceive > 0 )
      {
         bAttached = TRUE;
         break;
      }

      Delay( ATTACH_DELAY );
   }

   return( bAttached );
}

/****************************************************************************/
/* DetachDriver() - Detaches HECI driver                                    */
/****************************************************************************/

static void DetachDriver( void )
{
   if( bAttached )
   {
      HeciDisconnect();
      bAttached = FALSE;
   }
}

/****************************************************************************/
/* CommonCmdHandler() - Common code used to pass commands and obtain any    */
/* responses from the QST subsystem.  This code MUST be compatible with any */
/* revision of the QST firmware.                                            */
/****************************************************************************/

BOOL CommonCmdHandler(

   IN  void                         *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                       tCmdSize,           // Size of command packet
   OUT void                         *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                       tRspSize            // Expected size of response packet
){
   DWORD                            tReceived;          // Response packet size
   int                              iRetries;           // Retry counter
   int                              iErrnoSave;         // For saving errno value
   BOOL                             bSucceeded = FALSE; // Success indicator

   // If we had problem during module initialization, we can't continue

   if( iInitErrno )
   {
      errno = iInitErrno;
      return( FALSE );
   }

#ifdef SINGLE_THREADED

   // Get exclusive access to driver (no overlapped operations)

   if( !EnterCritSect( hCritSect ) )
      return( FALSE );

#endif

   // Attach to HECI driver if we haven't already

   if( bAttached || AttachDriver() )
   {
      // Attached; now have access to info needed to finish verifying command/response size...

      if( (tCmdSize > iMaxReceive) || (tRspSize > iMaxReceive) )
      {
         // wants to send/receive more than can be supported...

         iErrnoSave = ERANGE;
      }
      else
      {
         // Sizes can be handled; we can attempt the transaction
         // Support retries during attempt...

         for( iRetries = 0; iRetries < RETRY_COUNT; iRetries++ )
         {
            // Send Command packet

            if( HeciSend( pvCmdBuf, tCmdSize ) )
            {
               // Successful!
               // If no response is desired, we're done!

               if( tRspSize == 0 )
               {
                  bSucceeded = TRUE;
                  break;
               }

               // Receive and process Response Packet

               tReceived = HeciReceive( pvRspBuf, tRspSize );

               if( tReceived > 0 )
               {
                  // We have a response so we're done, only need to verify length

                  // Since QST, if it's rejecting/failing command, always returns
                  // just a single byte (the status byte), we can say there's a
                  // problem if the length doesn't match what's requested and the
                  // status byte says succeeded. Otherwise we're cool...

                  if( (tReceived != tRspSize) && (*((UINT8*)pvRspBuf) == QST_CMD_SUCCESSFUL) )
                     iErrnoSave = ENOSPC;
                  else
                     bSucceeded = TRUE;

                  break;
               }
            }

            // Only get here if send/receive failed
            // Want to remember ccode from first attempt, not after retry...

            if( iRetries == 0 )
               iErrnoSave = errno;

            // Recreate attachment to driver (in case lost due to PM transition, HECI reset, etc.)
            // Implement our retry delay in the middle of this to give driver a chance to recover
            // (and others a chance to use driver)

            DetachDriver();

#ifdef SINGLE_THREADED
            LeaveCritSect( hCritSect );
#endif

            Delay( RETRY_DELAY );

#ifdef SINGLE_THREADED
            EnterCritSect( hCritSect );
#endif

            if( !AttachDriver() )
               break;      // Can't reattach, time to give up
         }
      }
   }

#ifdef SINGLE_THREADED
   LeaveCritSect( hCritSect );
#endif

   // Set errno to reflect any errors detected

   if( !bSucceeded )
      errno = iErrnoSave;

   return( bSucceeded );
}

/****************************************************************************/
/* QstCommand() - Sends command to the QST Subsystem and awaits response.   */
/* Function returns TRUE/FALSE success indicator. Use errno to obtain       */
/* details about failures.                                                  */
/*                                                                          */
/* If errors occur during the write/read operations, which will happen if   */
/* the system goes through a low-power state transition and could happen if */
/* the ME suffers a failure (reset), we try reforming the QST Subsystem     */
/* connection. If successful, we then restart the transaction. We will try  */
/* this RETRY_COUNT times before giving up.                                 */
/****************************************************************************/

BOOL QstCommand(

   IN  void                         *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                       tCmdSize,           // Size of command packet
   OUT void                         *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                       tRspSize            // Expected size of response packet
){
   P_QST_LEG_SST_PASS_THROUGH_CMD   pstQstCmd = (P_QST_LEG_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                                         // For structured access to command packet
   BOOL                             bSucceeded = FALSE;  // Success indicator

   // Initialize Subsystem Information structure

   if (!GetSubsystemInformation())
   {
      errno = ENODEV;
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
      errno = EFAULT;
      return( FALSE );
   }

   // Validate command buffer size

   if( tCmdSize < sizeof(QST_LEG_CMD_HEADER) )
   {
      errno = EPERM;
      return FALSE;
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LEG_LAST_CMD_CODE )
   {
      errno = EPERM;
      return( FALSE );
   }

   // Verify Command/Response Lengths

   if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_LEG_CMD_HEADER)))
       || (tRspSize != (pstQstCmd->stHeader.wResponseLength)) )
   {
      errno = ERANGE;
      return( FALSE );
   }

   // Verify Command/Response Lengths for SST Command if one's embedded in the packet

   if( pstQstCmd->stHeader.byCommand == QST_LEG_SST_PASS_THROUGH )
   {
      if(    (pstQstCmd->stHeader.wCommandLength  != (pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
          || (pstQstCmd->stHeader.wResponseLength != (pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) )
      {
         errno = ERANGE;
         return( FALSE );
      }
   }

   // Determine if sending to a QST 2.x ME firmware...

   if( TranslationToNewRequired() )
   {
      // Target is QST 2.x ME firmware so translate command to new command set.

      switch( TranslateToNewCommand( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) )
      {
      case TRANSLATE_CMD_SUCCESS:

         return( TRUE );

      case TRANSLATE_CMD_INVALID_PARAMETER:
      case TRANSLATE_CMD_BAD_COMMAND:

         errno = EINVAL;
         return( FALSE );

      case TRANSLATE_CMD_NOT_ENOUGH_MEMORY:

         errno = ENOMEM;
         return( FALSE );

      case TRANSLATE_CMD_FAILED_WITH_ERROR_SET:
      default:

         return( FALSE );
      }
   }

   // Call common command handler (sets errno before exit if failed)

   return( CommonCmdHandler( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
}

/****************************************************************************/
/* QstCommand2() - Sends command to the QST Subsystem and awaits response.  */
/* Function returns TRUE/FALSE success indicator. Use errno to obtain       */
/* details about failures.                                                  */
/*                                                                          */
/* If errors occur during the write/read operations, which will happen if   */
/* the system goes through a low-power state transition and could happen if */
/* the ME suffers a failure (reset), we try reforming the QST Subsystem     */
/* connection. If successful, we then restart the transaction. We will try  */
/* this RETRY_COUNT times before giving up.                                 */
/****************************************************************************/

BOOL QstCommand2(

   IN  void                        *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                      tCmdSize,           // Size of command packet
   OUT void                        *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                      tRspSize            // Expected size of response packet
){
   P_QST_SST_PASS_THROUGH_CMD       pstQstCmd = (P_QST_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                                        // For structured access to command packet
   BOOL                             bSucceeded = FALSE; // Success indicator

   // Initialize Subsystem Information structure

   if (!GetSubsystemInformation())
   {
      errno = ENODEV;
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
      errno = EFAULT;
      return( FALSE );
   }

   // Validate command buffer size

   if( tCmdSize < sizeof(QST_CMD_HEADER) )
   {
      errno = EPERM;
      return( FALSE );
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LAST_CMD_CODE )
   {
      errno = EPERM;
      return( FALSE );
   }

   // Verify Command/Response Lengths

   if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_CMD_HEADER)))
       || (tRspSize != (pstQstCmd->stHeader.wResponseLength)) )
   {
      errno = ERANGE;
      return( FALSE );
   }

   // Verify Command/Response Lengths for SST Command if one's embedded in the packet

   if( pstQstCmd->stHeader.byCommand == QST_SST_PASS_THROUGH )
   {
      if(    (pstQstCmd->stHeader.wCommandLength  != (pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
          || (pstQstCmd->stHeader.wResponseLength != (pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) )
      {
         errno = ERANGE;
         return( FALSE );
      }
   }

   // Determine if sending to a QST 1.x ME firmware...

   if( TranslationToLegacyRequired() )
   {
      // Target is QST 1.x ME firmware so translate command to legacy command set.

      switch( TranslateToLegacyCommand( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) )
      {
      case TRANSLATE_CMD_SUCCESS:

         return( TRUE );

      case TRANSLATE_CMD_INVALID_PARAMETER:
      case TRANSLATE_CMD_BAD_COMMAND:

         errno = EINVAL;
         return( FALSE );

      case TRANSLATE_CMD_NOT_ENOUGH_MEMORY:

         errno = ENOMEM;
         return( FALSE );

      case TRANSLATE_CMD_FAILED_WITH_ERROR_SET:
      default:

         return( FALSE );
      }
   }

   // Call common command handler (sets errno before exit if failed)

   return( CommonCmdHandler( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
}

/****************************************************************************/
/* InitializeModule() - Initializes module. Runs when module loaded...      */
/****************************************************************************/

static void InitializeModule( void )
{
   // Initialize variables

   bAttached    = FALSE;
   iMaxReceive  = 0;
   iInitErrno   = 0;

   // Initialize HECI support module

   if( !HeciInitialize() )
   {
      iInitErrno = errno; // Save errno for reporting later
      return;
   }

#ifdef SINGLE_THREADED

   hCritSect = CreateCritSect( CRITSECT_TYPE, FALSE );

   if( !hCritSect )
   {
      iInitErrno = errno; // Save errno for reporting later
      return;
   }

#endif // def SINGLE_THREADED

}

/****************************************************************************/
/* CleanupModule() - Cleans up after module. Runs when module unloaded...   */
/****************************************************************************/

static void CleanupModule( void )
{
   DetachDriver();
   HeciCleanup();

#ifdef SINGLE_THREADED
   CloseCritSect( hCritSect );
#endif

}

/****************************************************************************/
/* Bind our module-level constructor/destructor functions for load-/unload- */
/* time execution                                                           */
/****************************************************************************/

#ifdef USE_CTORS
    static void (*const init_array [])( void ) __attribute__ ((section (".ctors")))      = { InitializeModule, };
    static void (*const fini_array [])( void ) __attribute__ ((section (".dtors")))      = { CleanupModule,    };
#else
    static void (*const init_array [])( void ) __attribute__ ((section (".init_array"))) = { InitializeModule, };
    static void (*const fini_array [])( void ) __attribute__ ((section (".fini_array"))) = { CleanupModule,    };
#endif

