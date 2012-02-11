/****************************************************************************/
/*                                                                          */
/*  Module Name:    BusTest.c                                               */
/*                                                                          */
/*  Description:    Implements sample program BusTest, which  demonstrates  */
/*                  how  to  utilize the services of Intel(R) Quiet System  */
/*                  Technology  (QST)  to  implement   support   for   the  */
/*                  enumeration  of  all devices connected to the SST/PECI  */
/*                  Bus.                                                    */
/*                                                                          */
/*  Notes:      1.  The program  uses  the  SST  Pass-Through  command  to  */
/*                  request  that  QST  perform the specified SST/PECI Bus  */
/*                  transactions on the program's behalf. The SST/PECI Bus  */
/*                  interface  is  not  directly  accessible  to  software  */
/*                  running on the host processor.                          */
/*                                                                          */
/*              2.  The program cycles through the possible bus addresses,  */
/*                  sending a PING command to each device. For those  PECI  */
/*                  devices  (processors)  that respond, their presence is  */
/*                  documented. For those SST Bus  devices  that  respond,  */
/*                  the  program  sends a GETDIB command to the device and  */
/*                  documents the contents of the Device Information Block  */
/*                  (DIB) returned.                                         */
/*                                                                          */
/*              3.  The  source  code  contains  conditional  support  for  */
/*                  building  executables  for  DOS, Windows and Linux. In  */
/*                  the case of Windows, conditional support  is  provided  */
/*                  for  static (loadtime) or dynamic (runtime) binding of  */
/*                  the QST Comm DLL. Currently, the Visual Studio project  */
/*                  for  this  program  is  set up for dynamic binding. To  */
/*                  utilize dynamic binding, which allows the  program  to  */
/*                  determine  if  and  when  the DLL is to be loaded into  */
/*                  memory, modify the  project  to  include  source  file  */
/*                  ..\Support\QstComm.c and symbol DYNAMIC_DLL_LOADING.    */
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

#include "QstComm.h"
#include "QstCmd.h"

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

static UINT8                        CommandBuffer[128];
static P_QST_SST_PASS_THROUGH_CMD   pSstCommand  = (P_QST_SST_PASS_THROUGH_CMD)CommandBuffer;

static UINT8                        ResponseBuffer[128];
static P_SST_GET_LONG_DIB_RSP       pSstGetDibRsp  = (P_SST_GET_LONG_DIB_RSP)ResponseBuffer;

/****************************************************************************/
/* QstError() - Returns a pointer to a string providing an explanation for  */
/* the QST exception code provided.                                         */
/****************************************************************************/

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
};

static const char * const QstError( UINT8 byQstCCode )
{
   return( (byQstCCode <= QST_CMD_REJECTED_RSP_SIZE)? pszQstError[byQstCCode] : "Unknown QST Error" );
}

/****************************************************************************/
/* main() - Mainline for program.                                           */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
{
   unsigned uSstAddr, uNumDev;

   puts( "\nIntel(R) Quiet System Technology SST/PECI Bus Scan Demo" );
   puts( "Copyright (C) 2007-2008, Intel Corporation. All Rights Reserved.\n" );

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)

   // Initialize support for communicating with the QST Subsystem

   if( !QstInitialize() )
   {

#if defined(__WIN32__)
      printf( "\n*** Cannot Load Communications DLL, ccode = %d\n\n", GetLastError() );
#else
      printf( "\n*** Failed Initialization of IMEI I/F, errno = %d (%s)\n\n", errno, strerror( errno ) );
#endif

      return( -1 );
   }

#endif

   // Cycle through the possible SST/PECI bus addresses

   for( uNumDev = 0, uSstAddr = 0x01; uSstAddr <= 0xFF; uSstAddr++ )
   {
      // Ping the device to see if it's present...

      // Prepare SST Ping packet. For PING, there's no command byte...

      pSstCommand->stSSTPacket.stSSTHeader.bySSTAddress  = (UINT8)uSstAddr;
      pSstCommand->stSSTPacket.stSSTHeader.byWriteLength = 0;
      pSstCommand->stSSTPacket.stSSTHeader.byReadLength  = 0;

      // Prepare QST Pass-Through Command wrapper

      // Note: The QST_SST_CMD_DATA macro counts the command byte. For PING -
      // the only bus transaction that doesn't have a command byte - we must
      // manually account for this byte not being present...

      pSstCommand->stHeader.byCommand        = QST_SST_PASS_THROUGH;
      pSstCommand->stHeader.byEntity         = 0;
      pSstCommand->stHeader.wCommandLength   = QST_SST_CMD_DATA(0) - 1;
      pSstCommand->stHeader.wResponseLength  = QST_SST_RSP_SIZE(0);

      // Perform Transaction

      if( !QstCommand2( pSstCommand, QST_SST_CMD_SIZE(0) - 1, pSstGetDibRsp, QST_SST_RSP_SIZE(0) ) )
      {

#if defined(__WIN32__)
         printf("\nPing Command Delivery Failed, error = 0x%08X\n\n", GetLastError() );
#else
         printf("\nPing Command Delivery Failed, errno = %d (%s)\n\n", errno, strerror( errno ) );
#endif

         break;
      }

      // If the command succeeded, device responded...

      if( pSstGetDibRsp->byStatus == QST_CMD_SUCCESSFUL )
      {
         uNumDev++;

         // For PECI Bus address range, just document device presence

         if( (uSstAddr >= SST_FIRST_PECI_ADDRESS) && (uSstAddr <= SST_LAST_PECI_ADDRESS) )
         {
            if( (uSstAddr >= SST_CPU_1_ADDRESS) && (uSstAddr <= SST_CPU_4_ADDRESS) )
               printf( "Processor %d detected at address 0x%02X\n\n", uSstAddr - SST_CPU_1_ADDRESS + 1, uSstAddr );
            else
               printf( "PECI Device detected at address 0x%02X\n\n", uSstAddr );

            continue;
         }

         // Document SST device presence

         printf( "SST Device detected at address 0x%02X\n", uSstAddr );

         // Prepare SST GetDIB packet

         pSstCommand->stSSTPacket.stSSTHeader.bySSTAddress  = (UINT8)uSstAddr;
         pSstCommand->stSSTPacket.stSSTHeader.byWriteLength = 1;                            // Command Byte only
         pSstCommand->stSSTPacket.stSSTHeader.byReadLength  = sizeof(SST_LONG_DIB);         // DIB Size in bytes
         pSstCommand->stSSTPacket.byCommandByte             = SST_GET_DIB;

         // Prepare QST Pass-Through Command wrapper

         pSstCommand->stHeader.byCommand        = QST_SST_PASS_THROUGH;
         pSstCommand->stHeader.byEntity         = 0;
         pSstCommand->stHeader.wCommandLength   = QST_SST_CMD_DATA(0);
         pSstCommand->stHeader.wResponseLength  = QST_SST_RSP_SIZE(sizeof(SST_LONG_DIB) / 2); // DIB Size in words

         // Perform Transaction

         if( !QstCommand2( pSstCommand, QST_SST_CMD_SIZE(0), pSstGetDibRsp, QST_SST_RSP_SIZE(sizeof(SST_LONG_DIB) / 2) ) )
         {

#if defined(__WIN32__)
            printf( "  GetDIB Command Delivery Failed, error = 0x%08X\n\n", GetLastError() );
#else
            printf( "  GetDIB Command Delivery Failed, errno = %d (%s)\n\n", errno, strerror(errno) );
#endif

            continue;
         }

         if( pSstGetDibRsp->byStatus == QST_CMD_SUCCESSFUL )
         {
            // Have DIB, display its contents

            printf( "  Dev Capabilities: 0x%02X, Ver: %d.%d, VID: 0x%04X, DID: 0x%04X\n",
                    *((UINT8*)((VOID*)&pSstGetDibRsp->stLongDIB.stDevCapabilities)),
                    pSstGetDibRsp->stLongDIB.stVersionRevision.uSSTVersion,
                    pSstGetDibRsp->stLongDIB.stVersionRevision.uMinorRevision,
                    pSstGetDibRsp->stLongDIB.uSSTVendorId,
                    pSstGetDibRsp->stLongDIB.uDeviceId                                      );

            printf( "  Dev I/F: 0x%02X, Func I/F: 0x%02X, Dev I/F Ext: 0x%02X\n",
                    *((UINT8*)((VOID*)&pSstGetDibRsp->stLongDIB.stDeviceIF)),
                    pSstGetDibRsp->stLongDIB.uFuncionIF,
                    *((UINT8*)((VOID*)&pSstGetDibRsp->stLongDIB.stDeviceIFExt))             );

            printf( "  Vendor Specific ID: 0x%02X%02X%02X, Client Addr: 0x%02X\n\n",
                    pSstGetDibRsp->stLongDIB.uVendorSpecId[2],
                    pSstGetDibRsp->stLongDIB.uVendorSpecId[1],
                    pSstGetDibRsp->stLongDIB.uVendorSpecId[0],
                    pSstGetDibRsp->stLongDIB.uClientDevAddr                                 );
         }
         else
         {
            // Failed to get DIB

            printf( "  Failed to get Device Information Block, status = 0x%02X (%s)\n\n",
                    pSstGetDibRsp->byStatus, QstError( pSstGetDibRsp->byStatus )            );
         }
      }
   }

#if defined(DYNAMIC_DLL_LOADING) || defined(__MSDOS__)
   QstCleanup();
#endif

   return( uNumDev );

#if _MSC_VER > 1000
   UNREFERENCED_PARAMETER( iArgs );
   UNREFERENCED_PARAMETER( pszArg );
#endif

}
