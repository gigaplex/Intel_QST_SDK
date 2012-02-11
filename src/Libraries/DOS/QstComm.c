/****************************************************************************/
/*                                                                          */
/*  Module:         QstComm.c                                               */
/*                                                                          */
/*  Description:    Provides the functions necessary to  communicate  with  */
/*                  the Quiet System Technology (QST) Subsystem within      */
/*                  the DOS environment.                                    */
/*                                                                          */
/*  Notes:      1.  Utilizes the services of module heci.c to  handle  the  */
/*                  Host-to-Embedded  Controller  (HECI) H/W interface and  */
/*                  to  send  and  receive  packets  to  and from the QST   */
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
#include <conio.h>
#include <errno.h>

#include "typedef.h"
#include "heci.h"
#include "QstCmd.h"
#include "QstCmdLeg.h"
#include "LegTranslationFuncs.h"

#if defined(_MSC_VER) && (_MSC_VER <= 800)
#include "Mem32.h"
#endif

/****************************************************************************/
/* Hardware Definitions                                                     */
/****************************************************************************/

#define PCI_INTEL_VENDOR_ID             0x8086

#define PCI_HECI_MBAR_REG               0x10

#define PCI_INDEX_REG                   0x0CF8
#define PCI_DATA_REG                    0x0CFC

#define MAX_MESSAGE_SIZE                sizeof(QST_SET_SUBSYSTEM_CONFIG_CMD)

#define HOST_PROC_ADDRESS               0   // HECI Id for Host CPU
#define QST_PROC_ADDRESS                3   // HECI Id for QST Subsystem

/****************************************************************************/
/* Module Variables                                                         */
/****************************************************************************/

static void                             *pvBuffer = NULL;
static U32                              uHeciBAR;

/****************************************************************************/
/* ReadPCI() - Performs a register read from PCI address space. The support */
/* functions used are dependent upon the target environment...              */
/****************************************************************************/

static U32 ReadPCI( U32 uAddress )
{

#ifdef _MSC_VER
#if (_MSC_VER <= 800)

   // MS 16-bit compiler (i.e. 1.52c)

   out32( PCI_INDEX_REG, uAddress );
   return( in32( PCI_DATA_REG ) );

#else

   // ME 32-bit compiler (i.e. MSVC 6.0+)

   _outpd( PCI_INDEX_REG, uAddress );
   return( _inpd( PCI_DATA_REG ) );

#endif
#else

   // Open Watcom 32-bit compiler

   outpd( PCI_INDEX_REG, uAddress );
   return( inpd( PCI_DATA_REG ) );

#endif

}

/****************************************************************************/
/* FindHECIDevice() - Attempts to locate the HECI device. It scans the PCI  */
/* address space for a device that matches one of the HECI Device ID Masks. */
/****************************************************************************/

static const U32 HECIPCIDevIds[] =
{
   // Broadwater
// 0x2974,      // G946 (no ME; no QST possible)
   0x2984,      // 965 Pre-Production
   0x2994,      // Q965/Q963
   0x29A4,      // P965/G965/
   // BearLake
   0x29B4,      // Q35
   0x29C4,      // G33/P35/G35
   0x29D4,      // Q33
// 0x29E4,      // X38 (no ME; no QST possible)
   0x29F4,      // BL Pre-Production
   // EagleLake
   0x2E04,      // EL Pre-Production
   0x2E14,      // Q45/Q43
   0x2E24,      // G45/G43/P45
   0x2E34,      // G41
   0x2E44,      // B43
   0x2E94,      // B43 SoftCreek Upgrade
   // Ibexpeak
   0x3B64       // All SKUs
};

static const U32 NumHECIPCIDevIds = sizeof(HECIPCIDevIds) / sizeof(U32);


static U32 FindHECIDevice( void )
{
   U32 uAddr, uBus, uDev, uFunc, uDevId, uSearchId;

   for( uBus = 0; uBus < 256; uBus++ )
   {
      for( uDev = 0; uDev < 32; uDev++ )
      {
         for( uFunc = 0; uFunc < 8; uFunc++ )
         {
            for ( uDevId = 0; uDevId < NumHECIPCIDevIds; uDevId++ )
            {
               uSearchId = PCI_INTEL_VENDOR_ID | (HECIPCIDevIds[uDevId] << 16);

               uAddr = 0x80000000 | (uBus << 16) | (uDev << 11) | (uFunc << 8 );

               if( ReadPCI( uAddr ) == uSearchId )
                  return( uAddr );
            }
         }
      }
   }

   return( 0 );
}

/****************************************************************************/
/* QstInitialize() - Initializes support for communicating with the QST     */
/* subsystem. Function returns TRUE/FALSE success indicator. If FALSE is    */
/* returned, check errno for the reason. Possible error code are:           */
/*   ENXIO  - Cannot locate HECI device in PCI address space.               */
/*   EIO    - Errors initializing HECI device.                              */
/*   ENOMEM - Unable to alloocate receive buffer.                           */
/****************************************************************************/

BOOL QstInitialize( void )
{
   U32 uHeciAddr, uMsgSize = MAX_MESSAGE_SIZE;

   // Locate the HECI device in the PCI address space

   uHeciAddr = FindHECIDevice();

   if( !uHeciAddr )
   {
       errno = ENXIO;
       return( FALSE );
   }

   // Get base address for HECI device's registers

   uHeciBAR = ReadPCI( uHeciAddr | PCI_HECI_MBAR_REG ) & 0xFFFFFF00;

   // Initialize the HECI Interface

   if( !HECIInitialize( uHeciBAR ) )
   {
      errno = EIO;
      return( FALSE );
   }

   // Create a receive buffer

   pvBuffer = malloc( MAX_MESSAGE_SIZE );

   if( !pvBuffer )
   {
      errno = ENOMEM;
      return( FALSE );
   }

   // Clear any unattended messages left in the input stream

   while( HECIReceive( NON_BLOCKING, uHeciBAR, (U32 *)pvBuffer, &uMsgSize, HOST_PROC_ADDRESS, QST_PROC_ADDRESS ) );

   // We're ready!

   return( TRUE );
}

/****************************************************************************/
/* QstCleanup() - Cleans up the resources supporting communications with    */
/* the QST Subsystem.                                                       */
/****************************************************************************/

void QstCleanup( void )
{
   U32 uMsgSize = MAX_HECI_BUFFER_SIZE;

   if( pvBuffer )
   {
      // Clear any unattended messages left in the input stream

      while( HECIReceive( NON_BLOCKING, uHeciBAR, (U32 *)pvBuffer, &uMsgSize, HOST_PROC_ADDRESS, QST_PROC_ADDRESS ) );

      // Delete receive buffer

      free( pvBuffer );
      pvBuffer = NULL;
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
   )
{
   U32                              uMsgSize;           // Count returned by WriteFile()/ReadFile()
   P_QST_GENERIC_RSP                pstQstRsp = (P_QST_GENERIC_RSP)pvBuffer;
                                                        // For structured access to response buffer
                                                        // Note this is our raw response buffer, not
                                                        // that provided by the caller...

   // Send the command to the QstSubsystem

   if( !HECISend( (U32 *)pvCmdBuf, uHeciBAR, (U32)tCmdSize, HOST_PROC_ADDRESS, QST_PROC_ADDRESS ) )
   {
      errno = EIO;
      return( FALSE );
   }

   // Process for response if one expected

   if( tRspSize )
   {
      // Receive the response from the QST Subsystem

      uMsgSize = MAX_MESSAGE_SIZE;

      if( !HECIReceive( BLOCKING, uHeciBAR, (U32 *)pvBuffer, &uMsgSize, HOST_PROC_ADDRESS, QST_PROC_ADDRESS ) )
      {
         errno = EIO;
         return( FALSE );
      }

      // Check the response size. Must match if command wasn't rejected by QST

      if( (uMsgSize != tRspSize) && (pstQstRsp->byStatus == QST_CMD_SUCCESSFUL) )
      {
         errno = ENOSPC;
         return( FALSE );
      }

      // Data received is right length; place in caller's buffer

      memcpy( pvRspBuf, pvBuffer, tRspSize );
   }

   return( TRUE );
}

/****************************************************************************/
/* QstCommand() - Sends command to the QST Subsystem and awaits response.   */
/* Function returns TRUE/FALSE success indicator. If FALSE is returned,     */
/* check errno for the reason. Possible error code are:                     */
/*   ENXIO  - Interface is not initialized.                                 */
/*   EINVAL - Command/response size is too large.                           */
/*   EPERM  - Command code is not supported.                                */
/*   ERANGE - Command/Response data length or embedded SST Packet           */
/*            read/write data length is inconsistent/invalid.               */
/*   EIO    - Command/Response packet transmission failed.                  */
/*   ENOSPC - Response packet is not requested size.                        */
/****************************************************************************/

BOOL QstCommand(

   IN  void                         *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                       tCmdSize,           // Size of command packet
   OUT void                         *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                       tRspSize            // Expected size of response packet
){
   P_QST_LEG_SST_PASS_THROUGH_CMD   pstQstCmd = (P_QST_LEG_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                                        // For structured access to command packet

   // Initialize Subsystem Information structure

   if (!GetSubsystemInformation())
   {
      errno = ENODEV;
      return( FALSE );
   }

   // Fail call if not properly initialized

   if( !pvBuffer )
   {
      errno = ENXIO;
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
       errno = EFAULT;
       return( FALSE );
   }

   // Verify Command/Response packet sizes are not larger than allowed

   if( (tCmdSize > MAX_MESSAGE_SIZE) || (tRspSize > MAX_MESSAGE_SIZE) )
   {
      errno = EINVAL;
      return( FALSE );
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LEG_LAST_CMD_CODE )
   {
      errno = EPERM;
      return( FALSE );
   }

   // Verify Command/Response Lengths

   else if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_LEG_CMD_HEADER)))
            || (tRspSize != pstQstCmd->stHeader.wResponseLength) )
   {
      errno = ERANGE;
      return( FALSE );
   }

   // Verify Command/Response Lengths for SST Command if one's embedded in the packet

   else if(    (pstQstCmd->stHeader.byCommand == QST_LEG_SST_PASS_THROUGH)
            && (    (pstQstCmd->stHeader.wCommandLength  != (WORD)(pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
                 || (pstQstCmd->stHeader.wResponseLength != (WORD)(pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) ) )
   {
      errno = ERANGE;
      return( FALSE );
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

   // Call common command handler (Calls SetLastError before exit if failed)

   return( CommonCmdHandler( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
}

/****************************************************************************/
/* QstCommand2() - Sends command to the QST Subsystem and awaits response.  */
/* Function returns TRUE/FALSE success indicator. If FALSE is returned,     */
/* check errno for the reason. Possible error code are:                     */
/*   ENXIO  - Interface is not initialized.                                 */
/*   EINVAL - Command/response size is too large.                           */
/*   EPERM  - Command code is not supported.                                */
/*   ERANGE - Command/Response data length or embedded SST Packet           */
/*            read/write data length is inconsistent/invalid.               */
/*   EIO    - Command/Response packet transmission failed.                  */
/*   ENOSPC - Response packet is not requested size.                        */
/****************************************************************************/

BOOL QstCommand2(

   IN  void                        *pvCmdBuf,          // Address of buffer contaiing command packet
   IN  size_t                      tCmdSize,           // Size of command packet
   OUT void                        *pvRspBuf,          // Address of buffer for response packet
   IN  size_t                      tRspSize            // Expected size of response packet
){
   P_QST_SST_PASS_THROUGH_CMD      pstQstCmd = (P_QST_SST_PASS_THROUGH_CMD)pvCmdBuf;
                                                       // For structured access to command packet

   // Initialize Subsystem Information structure

   if (!GetSubsystemInformation())
   {
      errno = ENODEV;
      return( FALSE );
   }

   // Fail call if not properly initialized

   if( !pvBuffer )
   {
      errno = ENXIO;
      return( FALSE );
   }

   // Verify buffer validity

   if( !pvCmdBuf || !pvRspBuf )
   {
       errno = EFAULT;
       return( FALSE );
   }

   // Verify Command/Response packet sizes are not larger than allowed

   if( (tCmdSize > MAX_MESSAGE_SIZE) || (tRspSize > MAX_MESSAGE_SIZE) )
   {
      errno = EINVAL;
      return( FALSE );
   }

   // Verify Command Code

   if( pstQstCmd->stHeader.byCommand > QST_LAST_CMD_CODE )
   {
      errno = EPERM;
      return( FALSE );
   }

   // Verify Command/Response Lengths

   else if(    (tCmdSize != (pstQstCmd->stHeader.wCommandLength + sizeof(QST_CMD_HEADER)))
            || (tRspSize != pstQstCmd->stHeader.wResponseLength) )
   {
      errno = ERANGE;
      return( FALSE );
   }

   // Verify Command/Response Lengths for SST Command if one's embedded in the packet

   else if(    (pstQstCmd->stHeader.byCommand == QST_SST_PASS_THROUGH)
            && (    (pstQstCmd->stHeader.wCommandLength  != (WORD)(pstQstCmd->stSSTPacket.stSSTHeader.byWriteLength + sizeof(SST_CMD_HEADER)))
                 || (pstQstCmd->stHeader.wResponseLength != (WORD)(pstQstCmd->stSSTPacket.stSSTHeader.byReadLength + 1)) ) )
   {
      errno = ERANGE;
      return( FALSE );
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

   // Call common command handler (Calls SetLastError before exit if failed)

   return( CommonCmdHandler( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
}

