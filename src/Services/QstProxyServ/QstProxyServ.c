
#undef LOG_QST_COMMANDS             // define this to create log entries for
                                    // each QST Command proxied

/****************************************************************************/
/*                                                                          */
/*  Module:         QstInstServ.c                                           */
/*                                                                          */
/*  Description:    Implements a service that supports the instrumentation  */
/*                  of the Sensors and Fan Controllers that are managed by  */
/*                  the Intel(R) Quiet System Technology (QST) Subsystem.   */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                            INTEL CONFIDENTIAL                            */
/*     Copyright (c) 2008-2009, Intel Corporation. All Rights Reserved.     */
/*                                                                          */
/*  The source code  contained  or  described  herein  and  all  documents  */
/*  related to the source code ("Material") are owned by Intel Corporation  */
/*  or  its  suppliers  or  licensors.  Title to the Material remains with  */
/*  Intel  Corporation  or  its  suppliers  and  licensors.  The  Material  */
/*  contains trade secrets and proprietary and confidential information of  */
/*  Intel  or  its  suppliers  and licensors. The Material is protected by  */
/*  worldwide copyright and trade secret laws and  treaty  provisions.  No  */
/*  part  of  the  Material  may  be  used,  copied, reproduced, modified,  */
/*  published, uploaded, posted, transmitted, distributed, or disclosed in  */
/*  any way without Intel's prior express written permission.               */
/*                                                                          */
/*  No  license  under  any  patent,  copyright,  trade  secret  or  other  */
/*  intellectual  property  right  is  granted to or conferred upon you by  */
/*  disclosure  or  delivery  of  the  Materials,  either  expressly,   by  */
/*  implication, inducement, estoppel or otherwise. Any license under such  */
/*  intellectual  property rights must be express and approved by Intel in  */
/*  writing.                                                                */
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
#pragma warning( disable: 4201 )
#endif

#include <windows.h>
#include <tchar.h>
#include <shlobj.h>

#include "QstProxyInst.h"
#include "CompVer.h"
#include "Service.h"
#include "RegAccess.h"
#include "MilliTime.h"
#include "QstComm.h"
#include "QstCmd.h"
#include "QstCmdLeg.h"

/****************************************************************************/
/* Definitions                                                              */
/****************************************************************************/

// Registry Access

#define QST_REG_KEY                     "Software\\Intel\\QST"
#define QST_REG_POLLING                 "Polling Interval"

// Timing Parameters

#define POLLING_INTERVAL                50      // Milliseconds
#define DEF_POLLING_COUNT               20      // Times POLLING_INTERVAL totals 1 second

#define NO_ASYNC_PROCESSING_TIME        0       // no processing delay

// ServiceStart() Parameters

#define SERVICE_SUPPORT                 SUPPORT_PAUSE_CONTINUE    | \
                                        SUPPORT_SUSPEND_RESUME    | \
                                        SUPPORT_CUSTOM_CONTROLS   | \
                                        SUPPORT_PSEUDO_SERVICE

// QST Version information

#define QST_LEG_SUBSYSTEM_VERSION_MAJOR 5

// Log File Name (typically C:\Program Files\Intel\QST\QstProxyServ.log)

#define QST_LOG_DIRNAME                 _T("QST")
#define QST_LOG_FILENAME                _T("QstProxyServ.log")

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
/* pszStatStr[] - Array providing status strings                            */
/****************************************************************************/

static const char * const pszStatStr[] =
{
    "QST_CMD_SUCCESSFUL",
    "QST_CMD_REJECTED_UNSUPPORTED",
    "QST_CMD_REJECTED_LOCKED",
    "QST_CMD_REJECTED_PARAMETER",
    "QST_CMD_REJECTED_VERSION",
    "QST_CMD_FAILED_COMM_ERROR",
    "QST_CMD_FAILED_SENSOR_ERROR",
    "QST_CMD_FAILED_NO_MEMORY",
    "QST_CMD_FAILED_NO_RESOURCES",
    "QST_CMD_REJECTED_INVALID",
    "QST_CMD_REJECTED_CMD_SIZE",
    "QST_CMD_REJECTED_RSP_SIZE",
    "QST_CMD_REJECTED_CONTEXT"
};

/****************************************************************************/
/* Private Variables                                                        */
/****************************************************************************/

static SERVICE_STATE        eState = SERVICE_STATE_IDLE;

static HANDLE               hCmdMutex;
static HANDLE               hShMem;

static TCHAR                tszLogPath[256];

QST_PROXY_INST_SEG          *pQstInstSeg;
QST_DATA_SEGMENT            *pQstSeg;

static BYTE                 byRsp[sizeof(QST_SET_SUBSYSTEM_CONFIG_CMD)];
static BOOL                 bLegacy;

/****************************************************************************/
/* Service Constants                                                        */
/****************************************************************************/

static LPCTSTR const tszServiceDescription  =
    __TEXT( "Proxies instrumentation of Sensors and Controllers managed "
            "by the Intel(R) Quiet System Technology (QST) Subsystem."     );

/****************************************************************************/
/* LogEvent() - Outputs an entry to the Event Log                           */
/****************************************************************************/

typedef enum { LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR } LOG_LEVEL;
static const char * const szLevel[] = { "INFO  ", "WARN  ", "ERROR " };

#define LogError(x) LogEvent( LOG_LEVEL_ERROR, "Unable to %s, ccode = 0x%08X", x, GetLastError() );

void LogEvent( LOG_LEVEL eLevel, const char *pszFormat, ... )
{
    static char     szBuffer[256];
    int             iLen;
    time_t          tNow = time( NULL );
    struct tm *     tmNow = localtime( &tNow );
    DWORD           dwResult;
    va_list         vaList;
    HANDLE          hLog;

    // Format the log entry

    strftime( szBuffer, sizeof(szBuffer), "%m/%d/%y %H:%M:%S ", tmNow );
    strcpy( &szBuffer[18], szLevel[eLevel] );

    va_start( vaList, pszFormat );
    iLen = 24 + vsprintf( &szBuffer[24], pszFormat, vaList );
    va_end( vaList );

    szBuffer[iLen++] = '\r';
    szBuffer[iLen++] = '\n';

    // Append the entry to the log file

    hLog = CreateFile( tszLogPath, GENERIC_READ | GENERIC_WRITE, 0,
                       NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL );

    if( hLog != INVALID_HANDLE_VALUE )
    {
        SetFilePointer( hLog, 0, NULL, FILE_END );
        WriteFile( hLog, szBuffer, (DWORD)iLen, &dwResult, NULL );
        CloseHandle( hLog );
    }
}

/****************************************************************************/
/* InitEventLogging() - Initializes support for event logging               */
/****************************************************************************/

static BOOL InitEventLogging( void )
{
    // Ascertain base path for the Log File

    if( FAILED( SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES, NULL, 0, tszLogPath ) ) )
        _tcscpy( tszLogPath, _T("C:\\Program Files") );

    // Ensure our logging folder exists

    _tcscat( tszLogPath, _T("\\Intel") );

    if( !CreateDirectory( tszLogPath, NULL ) && (GetLastError() != ERROR_ALREADY_EXISTS) )
        return( FALSE );

    _tcscat( tszLogPath, "\\" );
    _tcscat( tszLogPath, QST_LOG_DIRNAME );

    if( !CreateDirectory( tszLogPath, NULL ) && (GetLastError() != ERROR_ALREADY_EXISTS) )
        return( FALSE );

    // Append filename to complete pathname

    _tcscat( tszLogPath, "\\" );
    _tcscat( tszLogPath, QST_LOG_FILENAME );

    return( TRUE );
}

/****************************************************************************/
/* BeginCritical() - Ensures we have access to QST Proxy Service and gets   */
/* us exclusive access to the communication segment.                        */
/****************************************************************************/

static BOOL BeginCritical( DWORD dwTimeout )
{
   switch( WaitForSingleObject( hCmdMutex, dwTimeout ) )
   {
   case WAIT_ABANDONED:
   case WAIT_OBJECT_0:

      return( TRUE );

   case WAIT_TIMEOUT:
   default:

      return( FALSE );
   }
}

/****************************************************************************/
/* EndCritical() - Releases exclusive access to the communcations segment   */
/****************************************************************************/

static void EndCritical( void )
{
   ReleaseMutex( hCmdMutex );
}

/****************************************************************************/
/* CheckLegacy() - Determines what kind of QST Subsystem is below us. We    */
/* use this to determine which interface (QstCommand() or QstCommand2() we  */
/* will use to process QST commands that are passed to us.                  */
/****************************************************************************/

static BOOL CheckLegacy( void )
{
    QST_GENERIC_CMD             stGenCmd;
    QST_GET_SUBSYSTEM_INFO_RSP  stInfoRsp;

    // Get QST Subsystem Information

    stGenCmd.stHeader.byCommand       = QST_GET_SUBSYSTEM_INFO;
    stGenCmd.stHeader.byEntity        = 0;
    stGenCmd.stHeader.wCommandLength  = QST_CMD_DATA_SIZE(QST_GENERIC_CMD);
    stGenCmd.stHeader.wResponseLength = sizeof(QST_GET_SUBSYSTEM_INFO_RSP);

    if( !QstCommand2( &stGenCmd, sizeof(QST_GENERIC_CMD), &stInfoRsp, sizeof(QST_GET_SUBSYSTEM_INFO_RSP) ) )
    {
       LogError( "communicate with QST Subsystem" );
       return( FALSE );
    }

    if( stInfoRsp.byStatus )
    {
       LogEvent( LOG_LEVEL_ERROR, "Unable to communicate with QST Subsystem, cond = 0x%02X (%s)",
                 stInfoRsp.byStatus, pszStatStr[stInfoRsp.byStatus] );

       return( FALSE );
    }

    // Analyze Subsystem Information for compatibility

    bLegacy = (stInfoRsp.uMajorVersionNumber <= QST_LEG_SUBSYSTEM_VERSION_MAJOR);
    return( TRUE );
}


/****************************************************************************/
/* DoServiceInitialize() - Service Framework Call-Back used to perform any  */
/* necessary initialization for the primary duties of the service.          */
/****************************************************************************/

BOOL DoServiceInitialize( DWORD dwArgc, LPTSTR *lpszArgv )
{
    int iValue;

    // Open Log File

    if( InitEventLogging() )
    {
        SECURITY_DESCRIPTOR sd;

        // Setup security for allocated objects

        InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
        SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );

        // Attempt to create the Shared Memory Segment

        hShMem = CreateFileMapping( INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, (DWORD)0,
                                    (DWORD)sizeof(QST_PROXY_INST_SEG), QST_COMM_SEG_NAME );

        if( hShMem )
        {
            if( GetLastError() == ERROR_ALREADY_EXISTS )
                LogEvent( LOG_LEVEL_WARN, "Shared memory segment already exists" );

            // Open segment's access to other processes

            SetKernelObjectSecurity( hShMem, DACL_SECURITY_INFORMATION, &sd );

            // Map Shared Memory Segment into our address space

            pQstInstSeg = (QST_PROXY_INST_SEG *)MapViewOfFile( hShMem, FILE_MAP_WRITE, 0, 0, 0 );

            if( pQstInstSeg )
            {
                pQstSeg = &pQstInstSeg->stDataSeg;

                // Clear its contents

                memset( pQstInstSeg, 0, sizeof(QST_PROXY_INST_SEG) );

                // Create mutex used for serializing shared memory segment writes

                hCmdMutex = CreateMutex( NULL, FALSE, QST_COMM_MUTEX );

                if( hCmdMutex )
                {
                    if( BeginCritical( 0 ) )
                    {
                        // Open mutex's access to other processes

                        SetKernelObjectSecurity( hCmdMutex, DACL_SECURITY_INFORMATION, &sd );

                        // Do full initializaton for QST Subsystem access

                        if( InitializeQst( TRUE ) )
                        {
                            // Setup Registry access

                            if( OpenRegistry( QST_REG_KEY ) )
                            {
                                // Obtain our polling interval

                                if( iReadRegistry( QST_REG_POLLING, NO_INDEX, DEF_POLLING_COUNT * POLLING_INTERVAL, &iValue ) )
                                {
                                    pQstInstSeg->stDataSeg.dwPollingInterval = (DWORD)iValue;

                                    if( CheckLegacy() )
                                    {
                                        EndCritical();

                                        // We're ready to go; detail initialization

                                        LogEvent( LOG_LEVEL_INFO, "Service Loading, Version = " COMP_VERSION_STR );
                                        return( TRUE );
                                    }
                                }

                                // Something has gone wrong; unwind what we've done and document failure

                                else
                                    LogError( "read registry" );

                                CloseRegistry();
                            }
                            else
                                LogError( "initialize registry access" );

                            CleanupQst();
                        }
                        else
                            LogError( "initialize QST Access" );

                        EndCritical();
                    }
                    else
                        LogEvent( LOG_LEVEL_INFO, "Mutex appears deadlocked" );

                    CloseHandle( hCmdMutex );
                }
                else
                    LogError( "create Mutex" );

                UnmapViewOfFile( pQstInstSeg );
            }
            else
                LogError( "map shared memory" );

            CloseHandle( hShMem );
        }
        else
            LogError( "allocate shared memory" );
    }

    return( FALSE );
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
    MILLITIME stMilliTime;

    // Main work loop

    CurrMTime( &stMilliTime );

    for( eState = SERVICE_STATE_STARTING; eState != SERVICE_STATE_TERMINATING; )
    {
        // Wait until next interval

        AddMTime( &stMilliTime, 0, POLLING_INTERVAL );
        WaitMTime( &stMilliTime );

        // Process for state

        switch( eState )
        {
        case SERVICE_STATE_STARTING:
        case SERVICE_STATE_CONTINUING:
        case SERVICE_STATE_RESUMING:

            eState = SERVICE_STATE_RUNNING;
            LogEvent( LOG_LEVEL_INFO, "Service Running" );

            // fall-through...

        case SERVICE_STATE_RUNNING:

            if( !BeginCritical( INFINITE ) )
                LogError( "enter critical section" );

            // Do Service-Specific Maintenance

            if( !GetTempMonUpdateQst() )
                LogError( "update temperature monitors" );

            if( !GetFanMonUpdateQst() )
                LogError( "update fan speed monitors" );

            if( pQstInstSeg->stDataSeg.iVoltMons )
            {
                if( !GetVoltMonUpdateQst() )
                    LogError( "update voltage monitors" );
            }

            if( pQstInstSeg->stDataSeg.iCurrMons )
            {
                if( !GetCurrMonUpdateQst() )
                    LogError( "update current monitors" );
            }

            if( !GetFanCtrlUpdateQst() )
                LogError( "update fan controllers" );

            EndCritical();
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
        }
    }

    eState = SERVICE_STATE_TERMINATED;
    LogEvent( LOG_LEVEL_INFO, "Service Terminated" );

    CloseRegistry();
    CleanupQst();
    CloseHandle( hCmdMutex );
    UnmapViewOfFile( pQstInstSeg );
    CloseHandle( hShMem );
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
/* NewPollingInterval() - Updates Polling Interval value in registry     */
/****************************************************************************/

static BOOL NewPollingInterval( int iInterval )
{
    if( !iWriteRegistry( QST_REG_POLLING, NO_INDEX, iInterval ) )
        return( FALSE );

    pQstInstSeg->stDataSeg.dwPollingInterval = iInterval;
    time( &pQstInstSeg->stDataSeg.tTimePollingIntervalChanged );

    return( TRUE );
}

/****************************************************************************/
/* ExecQstCommand() - Delivers commands to a legacy QST Subsystem. We log   */
/* these operations only if LOG_QST_COMMANDS is defined.                    */
/****************************************************************************/

static BOOL ExecQstCommand( void )
{
    BOOL                    bSuccess;
    P_QST_LEG_GENERIC_CMD   pstCmd  = (P_QST_LEG_GENERIC_CMD)pQstInstSeg->stCommSeg.byCmdRsp;
    P_QST_LEG_GENERIC_RSP   pstRsp  = (P_QST_LEG_GENERIC_RSP)byRsp;
    WORD                    wCmdLen = (WORD)(pstCmd->stHeader.wCommandLength + sizeof(QST_LEG_CMD_HEADER));
    WORD                    wRspLen = pstCmd->stHeader.wResponseLength;

#ifdef LOG_QST_COMMANDS
    BYTE                    byCmd   = pstCmd->stHeader.byCommand;
#endif

    bSuccess = QstCommand( pstCmd, (size_t)wCmdLen, pstRsp, (size_t)wRspLen );

    if( bSuccess )
    {
        if( pstRsp->byStatus )
            wRspLen = 1;

        memcpy( pstCmd, pstRsp, wRspLen );
        pQstInstSeg->stCommSeg.wLength = wRspLen;

#ifdef LOG_QST_COMMANDS
        LogEvent( LOG_LEVEL_INFO, "Executed legacy command 0x%02X, status = 0x%02X (%s)",
                  byCmd, pstRsp->byStatus, pszStatStr[pstRsp->byStatus] );
#endif

    }

#ifdef LOG_QST_COMMANDS
    else
        LogEvent( LOG_LEVEL_INFO, "Failed legacy command 0x%02X execution, ccode = 0x%08X",
                  byCmd, GetLastError() );
#endif

    return( bSuccess );
}

/****************************************************************************/
/* ExecQstCommand2() - Delivers commands to a 2.0 QST Subsystem. We log     */
/* these operations only if LOG_QST_COMMANDS is defined.                    */
/****************************************************************************/

static BOOL ExecQstCommand2( void )
{
    BOOL                bSuccess;
    P_QST_GENERIC_CMD   pstCmd  = (P_QST_GENERIC_CMD)pQstInstSeg->stCommSeg.byCmdRsp;
    P_QST_GENERIC_RSP   pstRsp  = (P_QST_GENERIC_RSP)byRsp;
    WORD                wCmdLen = (WORD)(pstCmd->stHeader.wCommandLength + sizeof(QST_CMD_HEADER));
    WORD                wRspLen = pstCmd->stHeader.wResponseLength;

#ifdef LOG_QST_COMMANDS
    BYTE                byCmd   = pstCmd->stHeader.byCommand;
#endif

    bSuccess = QstCommand2( pstCmd, (size_t)wCmdLen, pstRsp, (size_t)wRspLen );

    if( bSuccess )
    {
        if( pstRsp->byStatus )
            wRspLen = 1;

        memcpy( pstCmd, pstRsp, wRspLen );
        pQstInstSeg->stCommSeg.wLength = wRspLen;

#ifdef LOG_QST_COMMANDS
        LogEvent( LOG_LEVEL_INFO, "Executed command 0x%02X, status = 0x%02X (%s)",
                  byCmd, pstRsp->byStatus, pszStatStr[pstRsp->byStatus] );
#endif

    }

#ifdef LOG_QST_COMMANDS
    else
        LogEvent( LOG_LEVEL_INFO, "Failed command 0x%02X execution, ccode = 0x%08X",
                  byCmd, GetLastError() );
#endif

    return( bSuccess );
}

/****************************************************************************/
/* DoServiceOpcode() - Service Framework Call-Back used to signal the       */
/* arrival of a Service-specific event (indicated by the Opcode passed).    */
/* Threshold and Polling Interval change are rare enough that we will log   */
/* all of these we perform.                                                 */
/****************************************************************************/

DWORD DoServiceOpcode( DWORD dwOpcode, DWORD *pdwAsyncOpTime )
{
    HANDLE hRspSem;

    pdwAsyncOpTime = 0;
    pQstInstSeg->dwStatus = NO_ERROR;

    switch( dwOpcode )
    {
    case SET_TEMP_THRESH:

        if( SetTempThreshQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                              pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetTempThreshQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "SetTempThreshQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_FAN_THRESH:

        if( SetFanThreshQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                             pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetFanThreshQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "GetFanMonUpdateQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_VOLT_THRESH_LOW:

        if( SetVoltThreshLowQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                                 pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetVoltThreshLowQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "SetVoltThreshLowQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_VOLT_THRESH_HIGH:

        if( SetVoltThreshHighQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                                  pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetVoltThreshHighQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "SetVoltThreshHighQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_CURR_THRESH_LOW:

        if( SetCurrThreshLowQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                                 pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetCurrThreshLowQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "SetCurrThreshLowQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_CURR_THRESH_HIGH:

        if( SetCurrThreshHighQst( pQstInstSeg->iSensor, pQstInstSeg->stThresh.fNonCritical,
                                  pQstInstSeg->stThresh.fCritical, pQstInstSeg->stThresh.fNonRecoverable ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed SetCurrThreshHighQst( %d )", pQstInstSeg->iSensor );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "SetCurrThreshHighQst() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case SET_POLLING_INTERVAL:

        if( NewPollingInterval( (int)pQstInstSeg->dwNewPollingInterval ) )
        {
            LogEvent( LOG_LEVEL_INFO, "Successfully executed NewPollingInterval( %d )", pQstInstSeg->dwNewPollingInterval );
        }
        else
        {
            pQstInstSeg->dwStatus = GetLastError();
            LogEvent( LOG_LEVEL_WARN, "UpdatePollingInterval() failed, ccode = 0x%08X", pQstInstSeg->dwStatus );
        }

        break;

    case EXEC_QST_COMMAND:

        if( !( ( bLegacy )? ExecQstCommand() : ExecQstCommand2() ) )
            pQstInstSeg->dwStatus = GetLastError();

        break;

    default:

        pQstInstSeg->dwStatus = ERROR_INVALID_FUNCTION;
        break;
    }

    // Signal that the command has been completed

    hRspSem = OpenSemaphore( SEMAPHORE_ALL_ACCESS, FALSE, pQstInstSeg->stCommSeg.tszRspSem );

    if( hRspSem != NULL )
    {
        ReleaseSemaphore( hRspSem, 1, NULL );
        CloseHandle( hRspSem );
    }

    return( NO_ERROR );
}

/****************************************************************************/
/* main() - Execution entry point for the Service                           */
/****************************************************************************/

int main( int iArgs, char *pszArg[] )
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

    return( ServiceStart( QST_SERVICE_NAME, tszServiceDescription, QST_SERVICE_WINDOW,
                          COMP_VERSION, SERVICE_SUPPORT, iArgs, pszArg ) );
}
