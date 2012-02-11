/****************************************************************************/
/*                                                                          */
/*  Module:         QstComm.c                                               */
/*                                                                          */
/*  Description:    Implements functions allowing the  QST  Communications  */
/*                  (QstComm) DLL to be used with the simplicity of static  */
/*                  DLL  loading,  yet actually loads the DLL dymanically.  */
/*                  The  QST  Communications  DLL  provides  support   for  */
/*                  applications  to deliver commands to the QST Subsystem  */
/*                  for processing.                                         */
/*                                                                          */
/*  Notes:      1.  This is helper module, designed to support the dynamic  */
/*                  linking  of  programs with the QstComm DLL. It is NOT   */
/*                  needed for programs that statically link to the DLL.    */
/*                                                                          */
/*              2.  In order to use this module from your application, you  */
/*                  need to do the following:                               */
/*                                                                          */
/*                  a.  #define DYNAMIC_DLL_LOADING  before  you  #include  */
/*                      header file QstComm.h.                              */
/*                                                                          */
/*                  b.  Invoke function QstInitialize(), in order to  get   */
/*                      the  DLL  loaded  into  your application's address  */
/*                      space.                                              */
/*                                                                          */
/*                  c.  Invoke  function  QstCommand(),  as necessary, to   */
/*                      communicate commands to the QST Subsystem.          */
/*                                                                          */
/*                  d.  When done with DLL, call function QstCleanup() to   */
/*                      have the DLL unloaded.                              */
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
#include "QstComm.h"

/****************************************************************************/
/* Module Variables                                                         */
/****************************************************************************/

static BOOL                bQstCommDLL = FALSE;
static HMODULE             hQstCommDLL = NULL;
static DWORD               dwCCode;

static PFN_QST_COMMAND     pfQstCommand;
static PFN_QST_COMMAND     pfQstCommand2;

/****************************************************************************/
/* QstInitialize() - Initializes I/F for accessing service of the DLL       */
/****************************************************************************/

BOOL QstInitialize( void )
{
   // Load the DLL

   hQstCommDLL = LoadLibrary( QST_COMM_DLL );

   if( hQstCommDLL )
   {
      // Build pointer to the DLL's function(s)

      pfQstCommand  = (PFN_QST_COMMAND)GetProcAddress( hQstCommDLL, MAKEINTRESOURCE( QST_COMM_ORD ) );
      pfQstCommand2 = (PFN_QST_COMMAND)GetProcAddress( hQstCommDLL, MAKEINTRESOURCE( QST_COMM2_ORD ) );

      if( pfQstCommand && pfQstCommand2 )
         return( bQstCommDLL = TRUE );
      else
         dwCCode = GetLastError();

      FreeLibrary( hQstCommDLL );
   }
   else
      dwCCode = GetLastError();

   return( bQstCommDLL = FALSE );
}

/****************************************************************************/
/* QstCleanup() - Cleans up I/F to DLL.                                     */
/****************************************************************************/

void QstCleanup( void )
{
   if( bQstCommDLL )
   {
      FreeLibrary( hQstCommDLL );

      bQstCommDLL = FALSE;
      hQstCommDLL = NULL;
   }
}

/****************************************************************************/
/* QstCommand() - Sends command to the QST Subsystem and awaits response    */
/****************************************************************************/

BOOL APIENTRY QstCommand( void *pvCmdBuf, size_t tCmdSize, void *pvRspBuf, size_t tRspSize )
{
   if( bQstCommDLL )
      return( pfQstCommand( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
   else
   {
      SetLastError( dwCCode );
      return( FALSE );
   }
}

/****************************************************************************/
/* QstCommand2() - Sends command to the QST Subsystem and awaits response   */
/****************************************************************************/

BOOL APIENTRY QstCommand2( void *pvCmdBuf, size_t tCmdSize, void *pvRspBuf, size_t tRspSize )
{
   if( bQstCommDLL )
      return( pfQstCommand2( pvCmdBuf, tCmdSize, pvRspBuf, tRspSize ) );
   else
   {
      SetLastError( dwCCode );
      return( FALSE );
   }
}
