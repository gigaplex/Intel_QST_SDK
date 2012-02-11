/****************************************************************************/
/*                                                                          */
/*  Module:         QstCommD.c                                              */
/*                                                                          */
/*  Description:    Supports Windows applications  communicating  directly  */
/*                  with  the  Intel(R) Quiet system Technology (QST) Sub-  */
/*                  system. This implementation communicates directly with  */
/*                  the HECI  Device  Driver;  it  does  not  require  the  */
/*                  services  of the QstComm DLL (in fact, it contains the  */
/*                  same code used in this DLL).                            */
/*                                                                          */
/*  Notes:      1.  This module is not part of  the production module set;  */
/*                  it is provided for debugging purposes only.             */
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
#pragma warning( disable : 4201 4701 )
#endif

#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <setupapi.h>
#include <winioctl.h>
#include <initguid.h>

#include "QstCmd.h"
#include "QstCmdLeg.h"
#include "LegTranslationFuncs.h"
#include "QstComm.h"

/****************************************************************************/
/* COnfiguration Variables (all delays and timeouts are in milliseconds)    */
/****************************************************************************/

#undef  USE_GLOBAL_MUTEX                                // undef this to use thread-safe local mutex,
                                                        // define this to use process-safe global mutex

#define ATTACH_RETRIES              5                   // attach attempts before giving up
#define ATTACH_DELAY                1000                // delay between attach attempts

#define RETRY_COUNT                 10                  // communication attempts before giving up
#define RETRY_DELAY                 1000                // delay between retries

#define IOCTL_TIMEOUT               1000                // timeout for DeviceIOControl completion
#define WRITE_TIMEOUT               1000                // timeout for WriteFile() completion
#define READ_TIMEOUT                1000                // timeout for ReadFile() completion

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

// GUIDs

DEFINE_GUID( HECI_INTERFACE_GUID, 0xE2D1FF34, 0x3458, 0x49A9, 0x88, 0xDA, 0x8E, 0x69, 0x15, 0xCE, 0x9B, 0xE5 );
DEFINE_GUID( QST_SUBSYSTEM_GUID, 0x6B5205B9, 0x8185, 0x4519, 0xB8, 0x89, 0xD9, 0x87, 0x24, 0xB5, 0x86, 0x07 );

// IOCTLs

#define IOCTL_HECI_GET_VERSION      0x8000E000
#define IOCTL_HECI_CONNECT_CLIENT   0x8000E004

// Client Properties

#pragma pack(1)

typedef struct _HECI_CLIENT_PROPERTIES
{
   UINT32                           minRxBufferSize;
   UINT8                            clientSpecific[1];

} HECI_CLIENT_PROPERTIES;

#pragma pack()

/****************************************************************************/
/* Process-Specific Variables                                               */
/****************************************************************************/

static HANDLE                       hMutex;             // Handle for Thread-Safe/Process-Safe Mutex
static HANDLE                       hDriver;            // Handle for Driver Connection
static HECI_CLIENT_PROPERTIES       stProperties;       // Buffer for QST Subsystem Connection Properties
static void                         *pvBuffer;          // Pointer to allocated Receive Buffer
static OVERLAPPED                   stRcvOverlapped;    // OVERLAPPED object for async receive operations
static BOOL                         bAttached;          // Indicates if attached to HECI driver
static TCHAR                        tszDevice[256];     // Buffer for pathname of HECI driver
static BOOL                         bReceivePosted;     // Indicates is receive buffer posted to driver

/****************************************************************************/
/* DoIOCTL() - Performs Device I/O Control Operation to the HECI Driver.    */
/****************************************************************************/

static int DoIOCTL(

   IN  DWORD                        dwIOCTL,            // I/O Control Code
   IN  void                         *pvCmdBuff,         // Command Data Buffer
   IN  int                          iCmdBuffLen,        // Command Data Buffer Length
   OUT void                         *pvRspBuff,         // Response Data Buffer
   OUT int                          iRspBuffLen         // Response Data Buffer Length
){
   DWORD                            dwCount = 0;        // Buffer for transfer count
   DWORD                            dwStatus;           // Status Buffer
   DWORD                            dwError;            // Status Buffer
   BOOL                             bDone;              // DeviceIoControl completion status
   BOOL                             bCompleted = FALSE; // Indicates if operation completed
   OVERLAPPED                       stOverlapped;       // Async I/O Descriptor

   stOverlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

   if( stOverlapped.hEvent )
   {
      stOverlapped.Offset     = 0;
      stOverlapped.OffsetHigh = 0;

      bDone = DeviceIoControl( hDriver, dwIOCTL, pvCmdBuff, iCmdBuffLen, pvRspBuff, iRspBuffLen, NULL, &stOverlapped );
      dwError = GetLastError();

      if( !bDone && (dwError != ERROR_IO_PENDING) )
      {
         // Operation failed...

         dwStatus = dwError;
         dwCount = 0;
      }
      else
      {
         // Operation completed or at least started...

         switch( WaitForSingleObject( stOverlapped.hEvent, IOCTL_TIMEOUT ) )
         {
         case WAIT_TIMEOUT:     // Completion never signalled

            dwStatus = ERROR_TIMEOUT;
            dwCount = 0;
            break;

         case WAIT_OBJECT_0:    // Completion signalled
         case WAIT_ABANDONED:   // Mutex abandoned???

            if( !GetOverlappedResult( hDriver, &stOverlapped, &dwCount, TRUE ) )
            {
               dwStatus = GetLastError();
               dwCount = 0;
               break;
            }

            bCompleted = TRUE;
            break;

         case WAIT_FAILED:      // Function Failed
         default:

            dwStatus = GetLastError();
            dwCount = 0;
            break;
         }
      }

      CloseHandle( stOverlapped.hEvent );

      if( !bCompleted )
         SetLastError( dwStatus );
   }

   return( (int)dwCount );
}

/****************************************************************************/
/* DoSend() - Performs send of a message through the HECI driver.           */
/****************************************************************************/

static int DoSend(

   IN  void                         *pvBuffer,          // Message buffer
   IN  int                          iBufferLen          // Buffer length
){
   DWORD                            dwCount = 0;        // Buffer for read count
   DWORD                            dwStatus;           // Status Buffer
   DWORD                            dwError;            // Status Buffer
   BOOL                             bDone;              // WriteFile completion status
   BOOL                             bCompleted = FALSE; // Indicates if operation completed
   OVERLAPPED                       stOverlapped;       // Async I/O Descriptor

   stOverlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

   if( stOverlapped.hEvent )
   {
      stOverlapped.Offset     = 0;
      stOverlapped.OffsetHigh = 0;

      bDone = WriteFile( hDriver, pvBuffer, iBufferLen, NULL, &stOverlapped );
      dwError = GetLastError();

      if( !bDone && (dwError != ERROR_IO_PENDING) )
      {
         // Operation failed...

         dwStatus = dwError;
         dwCount = 0;
      }
      else
      {
         // Operation completed synchronously or started asynchronously...

         switch( WaitForSingleObject( stOverlapped.hEvent, WRITE_TIMEOUT ) )
         {
         case WAIT_TIMEOUT:     // Completion never signalled

            dwStatus = ERROR_TIMEOUT;
            dwCount = 0;
            break;

         case WAIT_OBJECT_0:    // Completion signalled
         case WAIT_ABANDONED:   // Mutex abandoned???

            if( !GetOverlappedResult( hDriver, &stOverlapped, &dwCount, TRUE ) )
            {
               dwStatus = GetLastError();
               dwCount = 0;
               break;
            }

            bCompleted = TRUE;
            break;

         case WAIT_FAILED:      // Function Failed
         default:

            dwStatus = GetLastError();
            dwCount = 0;
            break;
         }
      }

      CloseHandle( stOverlapped.hEvent );

      if( !bCompleted )
         SetLastError( dwStatus );
   }

   return( (int)dwCount );
}

/****************************************************************************/
/* PostReceive() - Posts receive buffer for response coming back from QST   */
/****************************************************************************/

static BOOL PostReceive(

   IN  void                         *pvBuffer,          // Response buffer
   IN  int                          iBufferLen          // Buffer length
){
   DWORD                            dwStatus;           // Status Buffer

   stRcvOverlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

   if( stRcvOverlapped.hEvent )
   {
      stRcvOverlapped.Offset     = 0;
      stRcvOverlapped.OffsetHigh = 0;

      // Signal success if operation completed synchronously or started asynchronously

      if( ReadFile( hDriver, pvBuffer, iBufferLen, NULL, &stRcvOverlapped ) || (GetLastError() == ERROR_IO_PENDING) )
         bReceivePosted = TRUE;
      else
      {
         // Failed; Preserve ccode across handle close

         dwStatus = GetLastError();
         CloseHandle( stRcvOverlapped.hEvent );
         SetLastError( dwStatus );

         bReceivePosted = FALSE;
      }
   }

   return( bReceivePosted );
}

/****************************************************************************/
/* CompleteReceive() - Completes receive of response coming back from QST   */
/****************************************************************************/

static int CompleteReceive(

   BOOL                             bCare               // Care about result
){
   DWORD                            dwCount = 0;        // Buffer for read count
   DWORD                            dwStatus;           // Status Buffer
   BOOL                             bCompleted = FALSE; // Indicates if operation completed

   switch( WaitForSingleObject( stRcvOverlapped.hEvent, READ_TIMEOUT ) )
   {
   case WAIT_TIMEOUT:     // Completion never signalled

      dwStatus = ERROR_TIMEOUT;
      dwCount = 0;
      break;

   case WAIT_OBJECT_0:    // Completion signalled
   case WAIT_ABANDONED:   // Mutex abandoned???

      if( !GetOverlappedResult( hDriver, &stRcvOverlapped, &dwCount, TRUE ) )
      {
         dwStatus = GetLastError();
         dwCount = 0;
         break;
      }

      bCompleted = TRUE;
      break;

   case WAIT_FAILED:      // Function Failed
   default:

      dwStatus = GetLastError();
      dwCount = 0;
      break;
   }

   CloseHandle( stRcvOverlapped.hEvent );
   bReceivePosted = FALSE;

   if( bCare && !bCompleted )
      SetLastError( dwStatus );

   return( (int)dwCount );
}

/****************************************************************************/
/* CancelReceive() - Cancels receive previously posted                      */
/****************************************************************************/

static void CancelReceive(

   void
){
   if( bReceivePosted )
   {
      CancelIo( hDriver );
      CompleteReceive( FALSE );
   }
}

/****************************************************************************/
/* AsyncDelay() - Implements a delay of some number of milliseconds.        */
/****************************************************************************/

static BOOL AsyncDelay(

   DWORD                            dwMilliseconds      // length of delay desired
){

#ifdef USE_GLOBAL_MUTEX

   ReleaseMutex( hMutex );
   Sleep( dwMilliseconds );

   switch( WaitForSingleObject( hMutex, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      return( TRUE );

   default:

      return( FALSE );
   }

#else

   Sleep( dwMilliseconds );
   return( TRUE );

#endif

}

/****************************************************************************/
/* LookupDriver() - Obtains the pathname for the HECI device.The function   */
/* returns a TRUE/FALSE success indicator. Extended error information is    */
/* available via GetLastError().                                            */
/****************************************************************************/

static BOOL LookupDriver(

   void
){
   LONG                             iIndex;             // Enumeration Identifier
   HDEVINFO                         hDeviceInfo;        // Handle for Device Info Set

   DWORD                            dwDetailSize;       // Size of Device Interface Detail Block
   SP_DEVICE_INTERFACE_DATA         stInterfaceData;    // Device Interface Data
   PSP_DEVICE_INTERFACE_DETAIL_DATA pstDeviceDetail;    // Device Interface Detail Data

   BOOL                             bFound = FALSE;     // Indicates whether we found name

   // Find all devices that have our interface

   hDeviceInfo = SetupDiGetClassDevs( (LPGUID)&HECI_INTERFACE_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

   if( hDeviceInfo != INVALID_HANDLE_VALUE )
   {
      stInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
      pstDeviceDetail = NULL;

      for( iIndex = 0; SetupDiEnumDeviceInterfaces( hDeviceInfo, NULL, (LPGUID)&HECI_INTERFACE_GUID, iIndex, &stInterfaceData ); iIndex++ )
      {
         if( !SetupDiGetDeviceInterfaceDetail( hDeviceInfo, &stInterfaceData, NULL, 0, &dwDetailSize, NULL ) )
         {
            if( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
               continue;
         }

         pstDeviceDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc( LPTR, dwDetailSize );

         if( pstDeviceDetail )
         {
            pstDeviceDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            if( !SetupDiGetDeviceInterfaceDetail( hDeviceInfo, &stInterfaceData, pstDeviceDetail, dwDetailSize, NULL, NULL ) )
            {
               LocalFree( pstDeviceDetail );
               pstDeviceDetail = NULL;
            }
         }

         break;
      }

      if( pstDeviceDetail )
      {
         _tcscpy( tszDevice, pstDeviceDetail->DevicePath );
         bFound = TRUE;

         LocalFree( pstDeviceDetail );
      }

      SetupDiDestroyDeviceInfoList( hDeviceInfo );
   }

   return( bFound );
}

/****************************************************************************/
/* AttachDriver() - Obtains resources necessary for communicating with the  */
/* QST Subsystem. This includes Driver Connection and Receive buffer. The   */
/* function returns a TRUE/FALSE success indicator. Extended error          */
/* information is available via GetLastError().                             */
/****************************************************************************/

static BOOL AttachDriver(

   void
){
   DWORD                            dwStatus;           // Win32 fuction call status
   int                              iRetries;           // Retry counter
   DWORD                            dwPropertiesSize;   // Size of Connection Properties

   for( iRetries = 0; iRetries < ATTACH_RETRIES; iRetries++ )
   {
      // Create driver connection

      hDriver = CreateFile( tszDevice, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL );

      if( hDriver == INVALID_HANDLE_VALUE )
         dwStatus = GetLastError();
      else
      {
         // Request connect to the QST Subsystem

         dwPropertiesSize = DoIOCTL( IOCTL_HECI_CONNECT_CLIENT, (LPVOID)&QST_SUBSYSTEM_GUID,
                                     16, &stProperties, sizeof(HECI_CLIENT_PROPERTIES) );

         if( dwPropertiesSize != sizeof(HECI_CLIENT_PROPERTIES) )
            dwStatus = GetLastError();
         else
         {
            // Obtain a receive buffer of the required size

            pvBuffer = LocalAlloc( LPTR, stProperties.minRxBufferSize );

            if( pvBuffer )
               return( bAttached = TRUE );    // We're attached, buffered and ready to rock!

            dwStatus = GetLastError();
         }

         CloseHandle( hDriver );
      }

      if( !AsyncDelay( ATTACH_DELAY ) )
         break;
   }

   SetLastError( dwStatus );
   return( bAttached = FALSE );
}

/****************************************************************************/
/* DetachDriver() - Deletes QST Subsystem communications resources.         */
/****************************************************************************/

static void DetachDriver(

   void
){
   if( bAttached )
   {
      CloseHandle( hDriver );
      LocalFree( pvBuffer );
      bAttached = FALSE;
   }
}

/****************************************************************************/
/* CheckRspSuccess () - Checks the status of the command in the response    */
/* buffer.  Returns TRUE for success and FALSE for all other values.  This  */
/* function must take into account the version of the firmware it is        */
/* communicating with.                                                      */
/****************************************************************************/

static BOOL CheckRspSuccess (
   IN    void     *pvRspBuf
   )
{
   //
   // Just use a hack for now to check for status.  We know that all status
   // information is just the first byte in the response packet.
   //
   if (*((UINT8*) pvRspBuf) == QST_CMD_SUCCESSFUL)
   {
      return TRUE;
   }

   return FALSE;
}

/****************************************************************************/
/* CommonCmdHandler() - Common code used to pass commands and obtain any    */
/* responses from the QST subsystem.  This code MUST be compatible with any */
/* revision of the QST firmware.                                            */
/****************************************************************************/

BOOL CommonCmdHandler (
   IN  void                         *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                       tCmdSize,           // Size of command packet
   OUT void                         *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                       tRspSize            // Expected size of response packet
   )
{
   BOOL           bSucceeded = FALSE;     // Success indicator
   DWORD          dwCount;                // Count returned by WriteFile()/ReadFile()
   DWORD          dwStatus;               // Buffer for status codes
   int            iRetries;               // Retry counter

   // Obtain (thread) exclusive access to resources

   switch( WaitForSingleObject( hMutex, INFINITE ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      break;

   default:

      return( FALSE );
   }

   // Attach to HECI driver if we aren't already

   if( !bAttached && !AttachDriver() )
   {
      // Couldn't attach the driver...

      dwStatus = GetLastError();
   }
   else
   {
      // Attached; now have info to verify max command/response size...

      if( (tCmdSize > stProperties.minRxBufferSize) || (tRspSize > stProperties.minRxBufferSize) )
         dwStatus = ERROR_BAD_LENGTH;

      // Length ok, attempt communication...

      else
      {
         // Support retries during communication...

         for( iRetries = 0; iRetries < RETRY_COUNT; iRetries++ )
         {
            bReceivePosted = FALSE;

            // Post Receive Buffer if a response expected

            if( (tRspSize == 0) || PostReceive( pvBuffer, stProperties.minRxBufferSize ) )
            {
               // Send Command Packet

               if( DoSend( pvCmdBuf, tCmdSize ) )
               {
                  // If no response is desired, we're done!

                  if( tRspSize == 0 )
                  {
                     bSucceeded = TRUE;
                     break;
                  }

                  // Receive and process Response Packet

                  dwCount = CompleteReceive( TRUE );

                  if( dwCount )
                  {
                     // Have a response; verify it

                     if( (dwCount != tRspSize) && (CheckRspSuccess(pvBuffer)) )
                     {
                        // Data received is larger/smaller than expected

                        dwStatus = ERROR_BAD_LENGTH;
                     }
                     else
                     {
                        // Data received is right length (or QST rejected command)
                        // Place info available into caller's buffer

                        memcpy( pvRspBuf, pvBuffer, __min(dwCount, tRspSize) );
                        bSucceeded = TRUE;
                     }

                     // We're done!

                     break;
                  }
               }

               // Remember ccode from first failed operation...

               if( iRetries == 0 )
                  dwStatus = GetLastError();

               // Won't need the buffer we posted...

               if( tRspSize != 0 )
                  CancelReceive();
            }

            // A retry is necessary; recreate attachment to driver in case lost due to PM transition, etc.

            DetachDriver();

            if( !AttachDriver() )
               break;                           // Can't reattach, time to give up

            if( !AsyncDelay( RETRY_DELAY ) )    // Delay retry to give driver a break
               break;
         }
      }
   }

   ReleaseMutex( hMutex );

   if( !bSucceeded )
      SetLastError( dwStatus );       // Indicate why we're failing

   return (bSucceeded);
}

/****************************************************************************/
/* QstInitialize() - Initializes support for communicating with the QST     */
/* Subsystem                                                                */
/****************************************************************************/

BOOL QstInitialize( void )
{
   // Ascertain path for HECI driver

   if( !LookupDriver() )
      return( FALSE );

   // Obtain mutex to ensure process-safe/thread-safe communications

#ifdef USE_GLOBAL_MUTEX
   hMutex = CreateMutex( NULL, FALSE, __TEXT("Global\\QSTCommandMutex") );
#else
   hMutex = CreateMutex( NULL, FALSE, NULL );
#endif

   if( !hMutex )
      return( FALSE );

#ifdef USE_GLOBAL_MUTEX

   // If creator of global Mutex, enable access by others

   if( GetLastError() != ERROR_ALREADY_EXISTS )
   {
      SECURITY_DESCRIPTOR sd;

      InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
      SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );
      SetKernelObjectSecurity( hMutex, DACL_SECURITY_INFORMATION, &sd );
   }

#endif

   // Indicate we require driver attachment

   bAttached = FALSE;
   return( TRUE );
}

/****************************************************************************/
/* QstCleanup() - Cleans up support for communicating with the QST          */
/* Subsystem                                                                */
/****************************************************************************/

void QstCleanup(

   void
){
   DetachDriver();
   CloseHandle( hMutex );
}

/****************************************************************************/
/* QstCommand() - Sends command to the QST Subsystem and awaits response.   */
/* Function returns TRUE/FALSE success indicator. Use GetLastError() to     */
/* obtain details about failures.                                           */
/*                                                                          */
/* If errors occur during the write/read operations, which will happen if   */
/* the system goes through a low-power state transition and could happen if */
/* the ME suffers a failure (reset), we try reforming the QST Subsystem     */
/* connection. If successful, we then restart the transaction. We will try  */
/* this RETRY_COUNT times before giving up.                                 */
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
      // Target is QST 2.x ME firmware so translate command to new command set.
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
/* QstCommand2() - Sends command to the QST Subsystem and awaits response.  */
/* Function returns TRUE/FALSE success indicator. Use GetLastError() to     */
/* obtain details about failures.                                           */
/*                                                                          */
/* If errors occur during the write/read operations, which will happen if   */
/* the system goes through a low-power state transition and could happen if */
/* the ME suffers a failure (reset), we try reforming the QST Subsystem     */
/* connection. If successful, we then restart the transaction. We will try  */
/* this RETRY_COUNT times before giving up.                                 */
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
