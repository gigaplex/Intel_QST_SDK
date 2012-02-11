/****************************************************************************/
/*                                                                          */
/*  Module:         QstProtServ.c                                           */
/*                                                                          */
/*  Description:    Provides the routines necessary to implement a service  */
/*                  that protects the system from thermal overrun. Working  */
/*                  in  conjunction  with  the   Intel(R)   Quiet   System  */
/*                  Technology  (QST)  Subsystem, the service monitors for  */
/*                  thermal sensors entering  the  Non-Recoverable  health  */
/*                  state. If this should occur, the service will initiate  */
/*                  the orderly shutdown of the system.                     */
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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#if defined(_WIN32) || defined(__WIN32__)
#pragma warning( disable : 4201 )
#endif

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

#include "QstProtServ.h"
#include "Service.h"
#include "QstInst.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

// Timing Parameters

#define POLLING_INTERVAL            100     // Milliseconds
#define POLLING_COUNT               100     // Times POLLING_INTERVAL totals 10 seconds

#define SHUTDOWN_DELAY              20      // Seconds

#define NO_ASYNC_PROCESSING_TIME    0       // no procesing delay

// ServiceStart() Parameters

#define SERVICE_SUPPORT             SUPPORT_PAUSE_CONTINUE        \
                                  | SUPPORT_SUSPEND_RESUME        \
                                  | SUPPORT_CUSTOM_CONTROLS       \
                                  | SUPPORT_PSEUDO_SERVICE

/****************************************************************************/
/* SERVICE_STATE - Defines the possible states that the Service may be in   */
/****************************************************************************/

typedef enum
{
    SERVICE_STATE_IDLE,
    SERVICE_STATE_STARTING,
    SERVICE_STATE_RUNNING,
    SERVICE_STATE_PAUSING,
    SERVICE_STATE_PAUSING_SUSPENDING,
    SERVICE_STATE_PAUSED,
    SERVICE_STATE_CONTINUING,
    SERVICE_STATE_SUSPENDING,
    SERVICE_STATE_SUSPENDING_PAUSING,
    SERVICE_STATE_SUSPENDED,
    SERVICE_STATE_RESUMING,
    SERVICE_STATE_PAUSED_SUSPENDED,
    SERVICE_STATE_TERMINATING,
    SERVICE_STATE_TERMINATED

} SERVICE_STATE;

/****************************************************************************/
/* LOG_LEVEL - Defines the possible log entry levels                        */
/****************************************************************************/

typedef enum
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR

} LOG_LEVEL;

/****************************************************************************/
/* Private Variables                                                        */
/****************************************************************************/

static FILE *               pFLog;
static SERVICE_STATE        eState = SERVICE_STATE_IDLE;
static int                  iPoll = 0;
static int                  iTemps;

/****************************************************************************/
/* Constant Variables                                                       */
/****************************************************************************/

static LPCTSTR const tszServiceName =
    __TEXT( "Intel(R) Quiet System Technology Thermal Protection Service"   );

static LPCTSTR const tszServiceWindow =
    __TEXT( "QstProtServWindow" );

static LPCTSTR const tszServiceDescription =
    __TEXT( "This service monitors the Temperature Sensors being managed "
            "by the Intel(R) Quiet System Technology Subsystem. If any "
            "Sensors enter the Non-Recoverable Health State, indicating "
            "that the temperature has reached dangerous levels, the "
            "service will protect the system by initiating its orderly "
            "shutdown."                                                     );

/****************************************************************************/
/* LogEvent() - Outputs an entry to the Event Log                           */
/****************************************************************************/

static const char * const szLevel[] = { "INFO ", "WARN ", "ERROR" };

void LogEvent( LOG_LEVEL eLevel, const char *pszFormat, ... )
{
    char            szString[20];
    time_t          tNow;
    struct tm *     tmNow;
    va_list         vaList;

    tNow  = time( NULL );
    tmNow = localtime( &tNow );

    strftime( szString, sizeof(szString), "%m/%d/%y %H:%M:%S", tmNow );
    fprintf( pFLog, "%s %s ", szString, szLevel[eLevel] );

    va_start( vaList, pszFormat );
    vfprintf( pFLog, pszFormat, vaList );
    va_end( vaList );

    fputc( '\n', pFLog );
}

/****************************************************************************/
/* OpenLog() - Opens the event log                                          */
/****************************************************************************/

static BOOL OpenLog( void )
{
    TCHAR tszLogPath[256];

    // Ascertain path for the Log File

    if( FAILED( SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES, NULL, 0, tszLogPath ) ) )
        _tcscpy( tszLogPath, _T("C:\\Program Files") );

    // Ensure our logging folder exists

    _tcscat( tszLogPath, _T("\\Intel") );

    if( !CreateDirectory( tszLogPath, NULL ) && (GetLastError() != ERROR_ALREADY_EXISTS) )
    {
        errno = ENOENT;
        return( FALSE );
    }

    _tcscat( tszLogPath, _T("\\QST") );

    if( !CreateDirectory( tszLogPath, NULL ) && (GetLastError() != ERROR_ALREADY_EXISTS) )
    {
        errno = ENOENT;
        return( FALSE );
    }

    // Open the Log File (for append; text mode)

    _tcscat( tszLogPath, _T("\\QstProtServ.log") );
    pFLog = _tfopen( tszLogPath, _T("at") );
    return( pFLog != NULL );
}

/****************************************************************************/
/* DoServiceInitialize() - Service Framework Call-Back used to perform any  */
/* necessary initialization for the primary duties of the service. Our's    */
/* initializes communications with the QST Subsystem and enumerates the     */
/* temperature sensors.                                                     */
/****************************************************************************/

BOOL DoServiceInitialize( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Open Log File

    if( !OpenLog() )
        return( FALSE );

    // Initialize QST Support

    if( !QstInstInitialize() )
    {
        LogEvent( LOG_LEVEL_ERROR, "Cannot initialize QST access; ccode = %d", GetLastError() );
        LogEvent( LOG_LEVEL_ERROR, "Service Load Aborted" );
        return( FALSE );
    }

    // Enumerate Temperature Sensors

    if( !QstGetSensorCount( TEMPERATURE_SENSOR, &iTemps ) || (iTemps == 0) )
    {
        LogEvent( LOG_LEVEL_WARN, "No Temperature Sensors available" );
        LogEvent( LOG_LEVEL_WARN, "Service Load Aborted" );
        return( FALSE );
    }

    // Detail initialization

    LogEvent( LOG_LEVEL_INFO, "Service Loading, Version = " PROGRAM_VERSION_STR );
    return( TRUE );
}

/****************************************************************************/
/* DoServiceShutdown() - Service Framework Call-Back used to signal that    */
/* it's time to shutdown of the service. Our's simply sets the service's    */
/* state to TERMINATING. The main control loop will complete the            */
/* termination at the end of its next duty cycle.                           */
/****************************************************************************/

DWORD DoServiceShutdown( void )
{
    eState = SERVICE_STATE_TERMINATING;
    return( POLLING_INTERVAL );
}

/****************************************************************************/
/* ShutdownSystem() - Initiates the shutdown of the system.                 */
/****************************************************************************/

static BOOL ShutdownSystem( void )
{
    HANDLE              hProcess;
    TOKEN_PRIVILEGES    stToken;

    // Get the current process token handle so we can get shutdown privilege

    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hProcess ) )
        return( FALSE );

    // Get shutdown privileges for this process

    if( !LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &stToken.Privileges[0].Luid ) )
        return( FALSE );

    stToken.PrivilegeCount            = 1;
    stToken.Privileges[0].Attributes  = SE_PRIVILEGE_ENABLED;

    if( !AdjustTokenPrivileges( hProcess, FALSE, &stToken, 0, (PTOKEN_PRIVILEGES)NULL, 0 ) )
        return( FALSE );

    // Display the shutdown dialog box and start the time-out countdown.

    return( (BOOL)InitiateSystemShutdownEx( NULL, "Non-Recoverable Thermal Situation", SHUTDOWN_DELAY, TRUE, FALSE, 0xAF5C ) );
}

/****************************************************************************/
/* DoServiceWork() - Service Framework Call-Back used to perform the        */
/* primary duties of the Service.                                           */
/****************************************************************************/

void DoServiceWork( void )
{
    BOOL        bInShutdown = FALSE;
    int         iIndex;
    QST_HEALTH  eHealth;

    // Main work loop

    for( eState = SERVICE_STATE_STARTING; ; )
    {
        Sleep( POLLING_INTERVAL );

        switch( eState )
        {
        case SERVICE_STATE_STARTING:
        case SERVICE_STATE_CONTINUING:
        case SERVICE_STATE_RESUMING:

            eState = SERVICE_STATE_RUNNING;
            LogEvent( LOG_LEVEL_INFO, "Service Running" );

            // fall-through...

        case SERVICE_STATE_RUNNING:

            if( ++iPoll >= POLLING_COUNT )
            {
                if( !bInShutdown )
                {
                    for( iIndex = 0; iIndex < iTemps; iIndex++ )
                    {
                        if( !QstGetSensorHealth( TEMPERATURE_SENSOR, iIndex, &eHealth ) )
                        {
                            LogEvent( LOG_LEVEL_ERROR, "Unable to obtain temperature from Sensor %d, ccode = 0x%08X", iIndex, GetLastError() );
                            goto TerminateService;
                        }

                        if( eHealth == HEALTH_NONRECOVERABLE )
                        {
                            LogEvent( LOG_LEVEL_WARN, "Temperature Sensor %d has gone Non-Recoverable", iIndex );

                            if( !ShutdownSystem() )
                            {
                                LogEvent( LOG_LEVEL_ERROR, "Unable to initiate system shutdown, ccode = 0x%08X", GetLastError() );
                                goto TerminateService;
                            }

                            LogEvent( LOG_LEVEL_INFO, "System shutdown successfully initiated" );
                            QstInstCleanup();
                            bInShutdown = TRUE;
                            break;
                        }
                    }
                }

                iPoll = 0;
            }

            break;

        case SERVICE_STATE_PAUSING_SUSPENDING:

            eState = SERVICE_STATE_PAUSED_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused/Suspended" );
            break;

        case SERVICE_STATE_SUSPENDING_PAUSING:

            eState = SERVICE_STATE_PAUSED_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused/Suspended" );
            break;

        case SERVICE_STATE_SUSPENDING:

            eState = SERVICE_STATE_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Suspended" );
            break;

        case SERVICE_STATE_PAUSING:

            eState = SERVICE_STATE_PAUSED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused" );
            break;

        case SERVICE_STATE_TERMINATING:
        TerminateService:

            eState = SERVICE_STATE_TERMINATED;
            LogEvent( LOG_LEVEL_INFO, "Service Terminated" );

            QstInstCleanup();
            fclose( pFLog );
            return;
        }
    }
}

/****************************************************************************/
/* DoServicePause() - Service Framework Call-Back used to signal that the   */
/* Service's operation needs to be paused.                                  */
/****************************************************************************/

DWORD DoServicePause( void )
{
    switch( eState )
    {
    case SERVICE_STATE_RUNNING:

        eState = SERVICE_STATE_PAUSING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_SUSPENDED:

        eState = SERVICE_STATE_PAUSED_SUSPENDED;
        LogEvent( LOG_LEVEL_INFO, "Service Paused/Suspended" );
        return( NO_ASYNC_PROCESSING_TIME );

    case SERVICE_STATE_SUSPENDING:

        eState = SERVICE_STATE_SUSPENDING_PAUSING;
        return( POLLING_INTERVAL );
    }

    return( NO_ASYNC_PROCESSING_TIME );
}

/****************************************************************************/
/* DoServiceContinue() - Service Framework Call-Back used to signal that    */
/* the Service's operation may continue.                                    */
/****************************************************************************/

DWORD DoServiceContinue( void )
{
    switch( eState )
    {
    case SERVICE_STATE_PAUSED_SUSPENDED:

        eState = SERVICE_STATE_SUSPENDED;
        LogEvent( LOG_LEVEL_INFO, "Service Suspended" );
        return( NO_ASYNC_PROCESSING_TIME );

    case SERVICE_STATE_PAUSED:

        eState = SERVICE_STATE_CONTINUING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_PAUSING_SUSPENDING:

        eState = SERVICE_STATE_SUSPENDING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_PAUSING:

        eState = SERVICE_STATE_RUNNING;
        return( NO_ASYNC_PROCESSING_TIME );
    }

    return( NO_ASYNC_PROCESSING_TIME );
}

/****************************************************************************/
/* DoServiceSuspend() - Service Framework Call-Back used to signal that the */
/* Service's operation needs to be suspended.                               */
/****************************************************************************/

DWORD DoServiceSuspend( void )
{
    switch( eState )
    {
    case SERVICE_STATE_RUNNING:

        eState = SERVICE_STATE_SUSPENDING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_PAUSED:

        eState = SERVICE_STATE_PAUSED_SUSPENDED;
        LogEvent( LOG_LEVEL_INFO, "Service Paused/Suspended" );
        return( NO_ASYNC_PROCESSING_TIME );

    case SERVICE_STATE_PAUSING:

        eState = SERVICE_STATE_PAUSING_SUSPENDING;
        return( POLLING_INTERVAL );
    }

    return( NO_ASYNC_PROCESSING_TIME );
}

/****************************************************************************/
/* DoServiceResume() - Service Framework Call-Back used to signal that      */
/* the Service's operation may resume.                                      */
/****************************************************************************/

DWORD DoServiceResume( void )
{
    switch( eState )
    {
    case SERVICE_STATE_PAUSED_SUSPENDED:

        eState = SERVICE_STATE_PAUSED;
        LogEvent( LOG_LEVEL_INFO, "Service Paused" );
        return( NO_ASYNC_PROCESSING_TIME );

    case SERVICE_STATE_SUSPENDED:

        eState = SERVICE_STATE_RESUMING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_SUSPENDING_PAUSING:

        eState = SERVICE_STATE_PAUSING;
        return( POLLING_INTERVAL );

    case SERVICE_STATE_SUSPENDING:

        eState = SERVICE_STATE_RUNNING;
        return( NO_ASYNC_PROCESSING_TIME );
    }

    return( NO_ASYNC_PROCESSING_TIME );
}

/****************************************************************************/
/* DoServiceOpcode() - Service Framework Call-Back used to signal the       */
/* arrival of a Service-specific event (indicated by the Opcode passed).    */
/* Since this service has no user-defined opcodes, we simply return         */
/* ERROR_CALL_NOT_IMPLEMENTED.                                              */
/****************************************************************************/

DWORD DoServiceOpcode( DWORD dwOpcode, DWORD *pdwAsyncOpTime )
{
    return( ERROR_CALL_NOT_IMPLEMENTED );
}

/****************************************************************************/
/* main() - Execution entry point for the Service                           */
/****************************************************************************/

int main( int argc, char *argv[] )
{

#ifdef _DEBUG

#ifdef BREAK_ON_SERVICE_INITIATE

    if( argc <= 1 )
        DebugBreak();

#endif

#ifdef BREAK_ON_UTILITY_INITIATE

    if( argc >= 2 )
        DebugBreak();

#endif

#endif

    // Start up the service (returns only when service is terminating)

    return( ServiceStart( tszServiceName, tszServiceDescription, tszServiceWindow,
                          PROGRAM_VERSION, SERVICE_SUPPORT, argc, argv ) );
}
