/****************************************************************************/
/*                                                                          */
/*  Module:         QstDiskServ.c                                           */
/*                                                                          */
/*  Description:    Implements a background service that regularly obtains  */
/*                  temperature readings from Hard Disk Drives (HDDs)  and  */
/*                  delivers  these  readings to the Intel(R) Quiet System  */
/*                  Technology (QST) subsystem for processing.              */
/*                                                                          */
/*  Notes:      1.  The Service determines how  many  Virtual  Temperature  */
/*                  Monitors  (VTMs) are enabled and configured to monitor  */
/*                  Hard Drive temperature sensors. If none are supported,  */
/*                  the Service terminates itself. If there are more  VTMs  */
/*                  for Hard Drives than there are Hard Drives that expose  */
/*                  their  temperature,  the Service will send 0 (zero) as  */
/*                  the temperature reading to the extra VTMs.  This  will  */
/*                  ensure that fans are not overridden to 100% because no  */
/*                  temperature readings are available.                     */
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

#include "QstDiskServ.h"
#include "Service.h"

/****************************************************************************/
/* Configuration                                                            */
/****************************************************************************/

// Timing Parameters

#define POLLING_INTERVAL            250     // Milliseconds
#define POLLING_COUNT               4       // Times POLLING_INTERVAL totals 1 second

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

#define NO_ASYNC_PROCESSING_TIME    0       // no processing delay

// ServiceStart() Parameters

#define SERVICE_SUPPORT               SUPPORT_PAUSE_CONTINUE        \
                                    | SUPPORT_SUSPEND_RESUME        \
                                    | SUPPORT_CUSTOM_CONTROLS       \
                                    | SUPPORT_PSEUDO_SERVICE

/****************************************************************************/
/* SERVICE_STATE - Defines possible states for the Service' state machine   */
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
/* Variables                                                                */
/****************************************************************************/

static FILE *               pFLog;
static SERVICE_STATE        eState = SERVICE_STATE_IDLE;
static int                  iPoll = 0;
static int                  iSmartTemps;
static int                  iVtmTemps;

/****************************************************************************/
/* Constants                                                                */
/****************************************************************************/

static LPCTSTR const tszServiceName =
    __TEXT( "Intel(R) Quiet System Technology Hard Disk Temperature Service" );

static LPCTSTR const tszServiceWindow =
    __TEXT( "QstHDDServiceWindow" );

static LPCTSTR const tszServiceDescription =
    __TEXT( "This service monitors hard drives that expose temperature "
            "readings via S.M.A.R.T. and delivers these readings to the "
            "Intel(R) Quiet System Technology Subsystem for processing."     );

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

    _tcscat( tszLogPath, _T("\\QstDiskServ.log") );
    pFLog = _tfopen( tszLogPath, _T("at") );
    return( pFLog != NULL );
}

/****************************************************************************/
/* LogEvent() - Outputs an entry to the Event Log                           */
/****************************************************************************/

static const char * const szLevel[] = { "INFO ", "WARN ", "ERROR" };

void LogEvent( LOG_LEVEL eLevel, const char *pszFormat, ... )
{
    char            szString[33];
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
/* DoServiceInitialize() - Service Framework Call-Back used to perform any  */
/* necessary initialization for the primary duties of the service. Our's    */
/* initializes communications with the QST Subsystem and enumerates the     */
/* temperature sensors.                                                     */
/****************************************************************************/

BOOL DoServiceInitialize( DWORD dwArgc, LPTSTR *lpszArgv )
{
    int     iIndex, iLast;
    char    szModel[65];

    // Open Log File

    if( !OpenLog() )
        return( FALSE );

    LogEvent( LOG_LEVEL_INFO, "Service Loading; Version = " PROGRAM_VERSION_STR );

    // Enumerate Temperature Sensors

    iVtmTemps = InitVtmUpdate();

    if( iVtmTemps < 0 )
    {
        LogEvent( LOG_LEVEL_ERROR, "Unable to ascertain VTM count; ccode = %d", GetLastError() );
        LogEvent( LOG_LEVEL_INFO,  "Service Terminated" );
        return( FALSE );
    }

    // No need to continue if no VTMs configured for HD monitoring...

    if( iVtmTemps == 0 )
    {
        LogEvent( LOG_LEVEL_INFO,  "No VTMs configured for HD monitoring" );
        LogEvent( LOG_LEVEL_INFO,  "Service Terminated" );
        DoneVtmUpdate();
        return( FALSE );
    }

    LogEvent( LOG_LEVEL_INFO, "Servicing %d VTMs", iVtmTemps );

    // Initialize S.M.A.R.T. Support

    iSmartTemps = InitSmartTemp();

    if( iSmartTemps == NO_RESULT_AVAIL )
    {
        LogEvent( LOG_LEVEL_ERROR, "Cannot initialize S.M.A.R.T. access; ccode = %d", GetLastError() );
        LogEvent( LOG_LEVEL_INFO,  "Service Terminated" );
        DoneVtmUpdate();
        return( FALSE );
    }

    LogEvent( LOG_LEVEL_INFO, "Servicing %d HDDs", iSmartTemps );

    // Document monitor-drive associations

    iLast = ( iVtmTemps > iSmartTemps )? iVtmTemps : iSmartTemps;

    for( iIndex = 0; iIndex < iLast; iIndex++ )
    {
        GetSmartModel( iIndex, szModel );
        LogEvent( LOG_LEVEL_INFO, "VTM %d (TM %d) receiving temperatures from HDD %d (%s)",
                  iIndex + 1, GetVtmIndex( iIndex ) + 1, GetSmartIndex( iIndex ) + 1, szModel );
    }

    return( TRUE );
}

/****************************************************************************/
/* DoServiceShutdown() - Service Framework Call-Back used to signal that    */
/* it's time to shutdown the service. Our's simply sets the service's state */
/* to TERMINATING. The main control loop will complete the termination at   */
/* the end of its next duty cycle.                                          */
/****************************************************************************/

DWORD DoServiceShutdown( void )
{
    eState = SERVICE_STATE_TERMINATING;
    return( POLLING_INTERVAL );
}

/****************************************************************************/
/* DoServiceWork() - Service Framework Call-Back used to perform the        */
/* primary duties of the Service.                                           */
/****************************************************************************/

void DoServiceWork( void )
{
    BOOL bInShutdown = FALSE;
    int  iIndex, iReading;

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

            // fall through...

        case SERVICE_STATE_RUNNING:

            if( ++iPoll >= POLLING_COUNT )
            {
                if( !bInShutdown )
                {
                    for( iIndex = 0; iIndex < iVtmTemps; iIndex++ )
                    {
                        if( iIndex < iSmartTemps )
                        {
                            iReading = GetSmartTemp( iIndex );

                            if( iReading == NO_TEMP_AVAIL )
                            {
                                LogEvent( LOG_LEVEL_ERROR, "Unable to obtain temperature from HDD %d, ccode = 0x%08X",
                                          iIndex + 1, GetLastError() );

                                goto TerminateService;
                            }
                        }
                        else
                            iReading = 0;

                        if( !PutVtmTemp( iIndex, iReading ) )
                        {
                            LogEvent( LOG_LEVEL_ERROR, "Unable to deliver temperature to VTM %d (TM %d), ccode = 0x%08X",
                                      iIndex + 1, GetVtmIndex( iIndex ) + 1, GetLastError() );

                            goto TerminateService;
                        }
                    }
                }

                iPoll = 0;
            }

            break;

        case SERVICE_STATE_PAUSING_SUSPENDING:

            eState = SERVICE_STATE_PAUSED_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused-Suspended" );
            goto StopUpdating;

        case SERVICE_STATE_SUSPENDING_PAUSING:

            eState = SERVICE_STATE_PAUSED_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused-Suspended" );
            goto StopUpdating;

        case SERVICE_STATE_SUSPENDING:

            eState = SERVICE_STATE_SUSPENDED;
            LogEvent( LOG_LEVEL_INFO, "Service Suspended" );
            goto StopUpdating;

        case SERVICE_STATE_PAUSING:

            eState = SERVICE_STATE_PAUSED;
            LogEvent( LOG_LEVEL_INFO, "Service Paused" );
            goto StopUpdating;

        case SERVICE_STATE_TERMINATING:
        TerminateService:

            eState = SERVICE_STATE_TERMINATED;
            LogEvent( LOG_LEVEL_INFO, "Service Terminated" );

            // fall through...

        StopUpdating:

            // Let QST know we won't be providing readings for a while...

            for( iIndex = 0; iIndex < iVtmTemps; iIndex++ )
            {
                if( !PutVtmStop( iIndex ) )
                {
                    LogEvent( LOG_LEVEL_WARN, "Unable to deliver inactivation to VTM %d (TM %d), ccode = 0x%08X",
                              iIndex + 1, GetVtmIndex( iIndex ) + 1, GetLastError() );
                }
            }

            if( eState != SERVICE_STATE_TERMINATED )
                break;

            // We're done; cleanup service resources

            DoneVtmUpdate();
            DoneSmartTemp();
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
        LogEvent( LOG_LEVEL_INFO, "Service Paused-Suspended" );
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
        LogEvent( LOG_LEVEL_INFO, "Service Paused-Suspended" );
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
