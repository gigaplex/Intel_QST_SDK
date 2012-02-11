
/****************************************************************************/
/*                                                                          */
/*  Module:         QstDLL.c                                                */
/*                                                                          */
/*  Description:    One of a set of modules that  implement  the  Intel(R)  */
/*                  Quiet System Technology (QST) Instrumentation Library.  */
/*                  This  library exposes a set of functions that simplify  */
/*                  support for the  enumeration  and  monitoring  of  the  */
/*                  various  sensors  and  fan  speed controllers that are  */
/*                  supported  by  QST.   This   module   implements   the  */
/*                  environment-specific  portion  of  the  library, which  */
/*                  allows it to be built  into  a  Windows  Dynamic  Link  */
/*                  Library (DLL), a Linux Shared Object (SO) file or as a  */
/*                  bindable function library for DOS.                      */
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

#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
#include <windows.h>
#include "RegAccess.h"
#elif defined(__SOLARIS__) || defined(__sun__) || defined(__LINUX__) || defined(__linux__)
#include "INIFile.h"
#endif

#include "QstDll.h"
#include "CritSect.h"
#include "GlobMem.h"

/****************************************************************************/
/* Variables                                                                */
/****************************************************************************/

QST_DATA_SEGMENT                *pQstSeg;

HCRITSECT                       hCritSect;
HGLOBMEM                        hGlobMem;

#if defined(__WIN32__)
   DWORD                        dwInitError;
#else
   int                          iInitErrno;
#endif

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#define GLOB_MEM_ID             0xAF5C020           // Global Memory Id
#define CRIT_SECT_TYPE          0xAF5C030           // Critical Section Type

#define QST_DEF_POLLING         1000                // Default = 1000ms (1 second)

#if defined(__WIN32__)

#define QST_REG_KEY             "Software\\Intel\\QST"
#define QST_REG_POLLING         "Polling Interval"

#elif defined(__LINUX__) || defined(__SOLARIS__)

#define INI_FILE_NAME           "QST.ini"           // INI file name
#define INI_FILE_PARAG          "Instrumentation"   // Paragraph name
#define INI_FILE_PARAM          "PollingInterval"   // Parameter name
#define BUFF_SIZE               131                 // Buffer size

/****************************************************************************/
/* strupr() - Converts string to uppercase. Provided here for Linux, which  */
/* doesn't provide one.                                                     */
/****************************************************************************/

static strupr( char *pszString )
{
    for( ; *pszString; pszString++ )
    {
        if( islower( *pszString ) )
            *pszString = toupper( *pszString );
    }
}

/****************************************************************************/
/* CleanupString() - Trims whitespace and control characters from beginning */
/* and end of string and converts alphabetic characters to uppercase. We    */
/* need this function to support Linux, since we're using INI files for     */
/* persistent variable storage.                                             */
/****************************************************************************/

static void CleanupString( char *szString )
{
   int iIndex;

   // Strip off trailing spaces

   for( iIndex = strlen( szString ) - 1;
        iIndex && (isspace( szString[iIndex] ) || iscntrl( szString[iIndex] ));
        --iIndex )
   {
      szString[iIndex] = '\0';
   }

   // Strip off leading spaces

   for( iIndex = 0; isspace(szString[iIndex]); iIndex++ );

   if( iIndex )
      strcpy( szString, &szString[iIndex] );

   strupr( szString );
}

#endif  // defined(__LINUX__)

/****************************************************************************/
/* Delay() - Implements an 'n' millisecond delay. May be called with 0 as   */
/* the count, in order to implement recovery delays for slow ASICs.         */
/****************************************************************************/

static void Delay( int iMilliseconds )
{
    if( iMilliseconds )
    {

#if defined(__WIN32__)

        Sleep( iMilliseconds );

#elif defined(__LINUX__) || defined(__SOLARIS__)

        struct timespec stTime;

        stTime.tv_sec   = (time_t)(iMilliseconds / 1000);
        stTime.tv_nsec  = 1000000L * (iMilliseconds % 1000);    // 1,000,000 ns = 1 ms

        while( (nanosleep( &stTime, &stTime ) == -1) && (errno == EINTR) );

#elif defined(__MSDOS__)

#if (CLOCKS_PER_SEC != 1000)
#error clock() implementation does not support millisecond resolution!!
#endif

        clock_t tStart, tCurrent, tEnd;

        tCurrent = tStart = clock();
        tEnd = tStart + (clock_t)iMilliseconds;

        // Handle wraparound

        if( tEnd < tCurrent )
        {
          while( tCurrent )
             tCurrent = clock();
        }

        // Complete the delay

        while( tCurrent < tEnd )
            tCurrent = clock();

#else
#error Operating environment not recognized
#endif

    }
}

/****************************************************************************/
/* InitRegistryAccess() - Initializes access to DLL's registry key content  */
/****************************************************************************/

static BOOL InitRegistryAccess( BOOL bInitQstSeg )
{

#if defined(__WIN32__)

   if( !OpenRegistry( QST_REG_KEY ) )
      return( FALSE );

#endif

   if( bInitQstSeg )
   {

#if defined(__WIN32__)

      int iValue;

      if( !iReadRegistry( QST_REG_POLLING, NO_INDEX, QST_DEF_POLLING, &iValue ) )
         return( FALSE );

      pQstSeg->dwPollingInterval = (DWORD)iValue;

#elif defined(__LINUX__) || defined(__SOLARIS__)

      char szBuff[BUFF_SIZE+1], *pszBuff;

      // Get and process entry from INI file

      if( GetINIEntry( INI_FILE_NAME, INI_FILE_PARAG, INI_FILE_PARAM, szBuff, BUFF_SIZE ) )
      {
         CleanupString( szBuff );
         pQstSeg->dwPollingInterval = (int)strtol( szBuff, &pszBuff, 10 );

         if( *pszBuff == '\0' )
            return( TRUE );
      }

      // No INI file, no entry in file or entry was bad...

      pQstSeg->dwPollingInterval = QST_DEF_POLLING;
      sprintf( szBuff, "%d", pQstSeg->dwPollingInterval );

      return( PutINIEntry( INI_FILE_NAME, INI_FILE_PARAG, INI_FILE_PARAM, szBuff ) );

#else

      pQstSeg->dwPollingInterval = QST_DEF_POLLING;

#endif

   }

   return( TRUE );
}

/****************************************************************************/
/* CleanupRegistryAccess() - Cleans up access to DLL's registry key content */
/****************************************************************************/

static void CleanupRegistryAccess( void )
{

#if defined(__WIN32__)
   CloseRegistry();
#endif

}

/****************************************************************************/
/* UpdatePollingInterval() - Updates Polling Interval value in registry     */
/****************************************************************************/

BOOL UpdatePollingInterval( DWORD dwInterval )
{

#if defined(__WIN32__)

   if( !iWriteRegistry( QST_REG_POLLING, NO_INDEX, (int)dwInterval ) )
      return( FALSE );

#elif defined(__LINUX__) || defined(__SOLARIS__)

   char szBuff[16];

   sprintf( szBuff, "%d", dwInterval );

   if( !PutINIEntry( INI_FILE_NAME, INI_FILE_PARAG, INI_FILE_PARAM, szBuff ) )
      return( FALSE );

#endif

   pQstSeg->dwPollingInterval = dwInterval;
   time( &pQstSeg->tTimePollingIntervalChanged );
   return( TRUE );
}

/****************************************************************************/
/* EnterCriticalSection() - Initiates critical section                      */
/****************************************************************************/

BOOL BeginCriticalSection( void )
{
   return( EnterCritSect( hCritSect ) );
}

/****************************************************************************/
/* EndCriticalSection - Terminates critical section                         */
/****************************************************************************/

void EndCriticalSection( void )
{
   LeaveCritSect( hCritSect );
}

/****************************************************************************/
/* InitSharedMemoryPrimary() - Performs initialization for primary user     */
/* (creator) of the shared memory segment.                                  */
/****************************************************************************/

static BOOL InitSharedMemory( BOOL bCreator )
{
   // Clear its contents (sets bInitComplete to FALSE)

   if( bCreator )
       memset( pQstSeg, 0, sizeof(QST_DATA_SEGMENT) );

   // Do full initializaton for QST Subsystem access

   if( InitializeQst( bCreator ) )
   {
      // Do full initialization for Registry access

      if( InitRegistryAccess( bCreator ) )
      {
         // Do initialization for Critical Section support

         hCritSect = CreateCritSect( CRIT_SECT_TYPE, FALSE );

         if( hCritSect )
         {
            // We're successfully initialized!

            if( bCreator )
               pQstSeg->bInitComplete = TRUE;
            else
               while( !pQstSeg->bInitComplete )
                  Delay( 5 );

            return( TRUE );
         }

         // Failed, start unwinding

         CloseCritSect( hCritSect );
      }

      CleanupRegistryAccess();
   }

   CleanupQst();
   return( FALSE );
}

/****************************************************************************/
/* QstInstInitialize() - Initializes global memory segment access and       */
/* interface to QST Subsystem.                                              */
/****************************************************************************/

#if defined(__WIN32__) || defined(__LINUX__) || defined(__SOLARIS__)
   static
#endif

BOOL QstInstInitialize( void )
{
   // Start with attempt to lookup existing segment

   BOOL bCreator = FALSE;
   hGlobMem      = LookupGlobMem( GLOB_MEM_ID, sizeof(QST_DATA_SEGMENT) );

   // If that failed, we're creator; attempt creation

   if( !hGlobMem )
   {
       bCreator = TRUE;
       hGlobMem = CreateGlobMem( GLOB_MEM_ID, sizeof(QST_DATA_SEGMENT), TRUE );
   }

   if( hGlobMem )
   {
      // We have segment, map it into address space

      pQstSeg = (QST_DATA_SEGMENT *)MapGlobMem( hGlobMem );

      if( pQstSeg )
      {
         // Now initialize the segment as appropriate

         if( InitSharedMemory( bCreator ) )
            return( TRUE );

         // Failed, start unwinding

         UnmapGlobMem( pQstSeg );
      }

      CloseGlobMem( hGlobMem );
   }

   // If we get to here, initialization failed. Save error info for later

#if defined(__WIN32__)
   dwInitError = GetLastError();
#else
   iInitErrno = errno;
#endif

   pQstSeg = NULL;
   return( FALSE );
}

/****************************************************************************/
/* QstInstCleanup() - Cleans up global memory segment access and interface  */
/* to QST Subsystem.                                                        */
/****************************************************************************/

#if defined(__WIN32__) || defined(__LINUX__) || defined(__SOLARIS__)
   static
#endif

void QstInstCleanup( void )
{
   CloseCritSect( hCritSect );
   CleanupRegistryAccess();
   CleanupQst();

   UnmapGlobMem( pQstSeg );
   CloseGlobMem( hGlobMem );
}

#if defined(__WIN32__)
/****************************************************************************/
/* DllMain() - Handles Processes attaching to and detaching from DLL        */
/****************************************************************************/

BOOL WINAPI DllMain( HANDLE hModule, DWORD dwReason, LPVOID pvReserved )
{
   switch( dwReason )
   {
   case DLL_PROCESS_ATTACH:

      if( !QstInstInitialize() )
         return( FALSE );

      break;

   case DLL_PROCESS_DETACH:

      QstInstCleanup();
      break;
   }

   return( TRUE );

   UNREFERENCED_PARAMETER( hModule );
   UNREFERENCED_PARAMETER( pvReserved );
}

#elif defined(__LINUX__) || defined(__SOLARIS__)
/****************************************************************************/
/* Bind main init/cleanup functions for load-/unload-time execution         */
/****************************************************************************/

static void ModuleInit( void ) { QstInstInitialize(); }

#ifdef USE_CTORS
   static void (*const init_array [])( void ) __attribute__ ((section (".ctors")))      = { ModuleInit,     };
   static void (*const fini_array [])( void ) __attribute__ ((section (".dtors")))      = { QstInstCleanup, };
#else
   static void (*const init_array [])( void ) __attribute__ ((section (".init_array"))) = { ModuleInit,     };
   static void (*const fini_array [])( void ) __attribute__ ((section (".fini_array"))) = { QstInstCleanup, };
#endif

#endif

