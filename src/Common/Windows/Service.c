/****************************************************************************/
/*                                                                          */
/*  Module:         Service.c                                               */
/*                                                                          */
/*  Description:    This module implements a  Framework  which  simplifies  */
/*                  the   task  of  implementing  Windows  Services.  It's  */
/*                  features can be summarized as follows:                  */
/*                                                                          */
/*              1.  Provides full management of Service Control and State,  */
/*                  allowing services to be implemented as a set of simple  */
/*                  call-back functions.                                    */
/*                                                                          */
/*              2.  Provides support for  simplified  Service  management,  */
/*                  allowing  a  Service's executable to also be used as a  */
/*                  utility, providing Service install,  uinstall,  start,  */
/*                  stop and pause capabilities.                            */
/*                                                                          */
/*              3.  For those Services requiring it, provides support  for  */
/*                  power  state  transitions,  adding  suspend/resume  to  */
/*                  to repertoire of supported Service State transitions.   */
/*                                                                          */
/*              4.  Provides support for Services to be executed from  the  */
/*                  command line, in a pseudo-service environment, so that  */
/*                  a  significant  portion  of  the  service  code can be  */
/*                  debugged (from  Visual  Studio,  for  example)  before  */
/*                  moving  it into the real Service environment (which is  */
/*                  less conducive to debugging).                           */
/*                                                                          */
/*  Notes:      1.  The current implementation of  the  Service  Framework  */
/*                  has the following set of limitations:                   */
/*                                                                          */
/*                  i.  It does not support the implementation  of  Device  */
/*                      or File System Drivers or Network Services.         */
/*                                                                          */
/*                  ii. It does not support the  reprocessing  of  startup  */
/*                      parameters. If startup parameters must be changed,  */
/*                      the Service will need to be stopped and restarted.  */
/*                                                                          */
/*              2.  The  current  implementation  of  the   Pseudo-Service  */
/*                  support has the following set of limitations:           */
/*                                                                          */
/*                  i.  It does not provide support  for  framework  call-  */
/*                      back   DoServicePause(),   DoServiceContinue()  or  */
/*                      DoServiceOpcode().                                  */
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

#ifndef _MT
#define _MT     // define _MT so that _beginthread()/_exitthread() are available
#endif

#include <windows.h>
#include <process.h>

#include "Service.h"

/****************************************************************************/
/* Global Module Variables                                                  */
/****************************************************************************/

static TCHAR                        tszServiceName[MAX_PATH] = _T("");
static TCHAR                        tszServiceWindow[MAX_PATH] = _T("");
static DWORD                        dwServiceCapabilities    = 0;

/****************************************************************************/
/****************************************************************************/
/************************** Pseudo-Service Support **************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Pseudo-Service-Specific Variables                                        */
/****************************************************************************/

static HWND                         hMessageWindow  = NULL;
static BOOL                         bThreadDone     = FALSE;

/****************************************************************************/
/* WindowProc() - Callback function used by the Windows message handler     */
/****************************************************************************/

static LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    DWORD dwDelay;

    switch( uMsg )
    {
    case WM_CLOSE:              // If window close is requested, shutdown the pseudo-service

        if( !bThreadDone )
            DoServiceShutdown();

        break;

    case WM_POWERBROADCAST:     // Process power management events

        switch( wParam )
        {
        case PBT_APMSUSPEND:            // System is suspending...

            if( dwServiceCapabilities & SUPPORT_SUSPEND_RESUME )
            {
                dwDelay = DoServiceSuspend();

                if( dwDelay )
                    Sleep( dwDelay );
            }

            break;

        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMECRITICAL:
        case PBT_APMRESUMESUSPEND:      // System is resuming...

            if( dwServiceCapabilities & SUPPORT_SUSPEND_RESUME )
            {
                dwDelay = DoServiceResume();

                if( dwDelay )
                    Sleep( dwDelay );
            }

            break;
        }

        break;

    case WM_PSEUDO_SERVICE_COMMAND:     // Request from application

        DoServiceOpcode( (DWORD)lParam, &dwDelay );
        break;
    }

    return( DefWindowProc( hWnd, uMsg, wParam, lParam ) );
}

/****************************************************************************/
/* CreateMessageWindow() - Creates (hidden) window used to receive power    */
/* management broadcast messages, etc.                                      */
/****************************************************************************/

static BOOL CreateMessageWindow( void )
{
    WNDCLASSEX  sWindowClass;
    ATOM        sClassAtom;
    BOOL        bSuccess;

    // Register the window class

    memset( &sWindowClass, 0, sizeof(WNDCLASSEX) );

    sWindowClass.cbSize        = sizeof( WNDCLASSEX );
    sWindowClass.hInstance     = NULL;
    sWindowClass.lpszClassName = tszServiceWindow;
    sWindowClass.lpfnWndProc   = WindowProc;

    sClassAtom = RegisterClassEx( &sWindowClass );

    // Create window (will never be seen)

    hMessageWindow = CreateWindowEx(  0, tszServiceWindow, tszServiceName,
                                      0, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                      GetDesktopWindow(), NULL, NULL, NULL );

    if( hMessageWindow == NULL )
    {
        // Window creation didn't work, cleanup and return FALSE

        bSuccess = UnregisterClass( tszServiceWindow, NULL );
        return( FALSE );
    }

    return( TRUE );
}

/****************************************************************************/
/* DestroyMessageWindow() - Destroys the (hidden) window                    */
/****************************************************************************/

static void DestroyMessageWindow( void )
{
    DestroyWindow( hMessageWindow );
    UnregisterClass( tszServiceWindow, NULL );
    hMessageWindow = NULL;
}

/****************************************************************************/
/* WorkerThread() - Main entry point for worker thread                      */
/****************************************************************************/

static void WorkerThread( void *pParams )
{
    if( DoServiceInitialize( 0, NULL ) )
        DoServiceWork();

    bThreadDone = TRUE;
    _endthread();

    UNREFERENCED_PARAMETER( pParams );
}

/****************************************************************************/
/* RunAsPseudoService() - Creates a pseudo-service environment for the      */
/* invoking application to execute in.                                      */
/****************************************************************************/

static DWORD RunAsPseudoService( void )
{
    MSG         stMsg;
    HANDLE      hClearedEvent;

    // Kick off the worker thread

    bThreadDone = FALSE;

    if( _beginthread( WorkerThread, 0, NULL ) == -1 )
        return( ERROR_SERVICE_NO_THREAD );

    // Create a cleared Event object. This object will never be signalled, so we
    // can use it with MsgWaitForMultipleObjects to wait for messages with
    // a specific timeout.

    hClearedEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( hClearedEvent == NULL )
        return( GetLastError() );

    // Create a (hidden) window to provide a message pump.  This will be used to intercept
    // power management broadcast messages, etc. from the OS

    if( !CreateMessageWindow() )
        return( GetLastError() );

    // Main work loop

    for( ; ; )
    {
        // Check if the worker thread has terminated. If it has, we're done!

        if( bThreadDone )
            break;

        // Wait for a message or the timeout interval to expire

        MsgWaitForMultipleObjects( 1, &hClearedEvent, FALSE, 50, QS_ALLINPUT );

        // At least one message in the queue; process all of them

        while( PeekMessage( &stMsg, hMessageWindow, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &stMsg );
            DispatchMessage( &stMsg );
        }
    }

    // Clean up

    DestroyMessageWindow();
    CloseHandle( hClearedEvent );

    return( ERROR_SUCCESS );
}




/****************************************************************************/
/****************************************************************************/
/****************************** Service Support *****************************/
/****************************************************************************/
/****************************************************************************/

/****************************************************************************/
/* Service-Specific Definitions                                             */
/****************************************************************************/

#define STATUS_SUCCESS                          ((DWORD)0x00000000L)
#define STATUS_CREATE_SVC_ACCESS_DENIED         ((DWORD)0xC0000064L)
#define STATUS_CREATE_SVC_CIRCULAR_DEPENDENCY   ((DWORD)0xC0000065L)
#define STATUS_CREATE_SVC_DUP_NAME              ((DWORD)0xC0000066L)
#define STATUS_CREATE_SVC_INVALID_NAME          ((DWORD)0xC0000067L)
#define STATUS_CREATE_SVC_DESCRIPTION_ERROR     ((DWORD)0xC0000068L)
#define STATUS_CREATE_SVC_VERSION_ERROR         ((DWORD)0xC0000069L)
#define STATUS_CREATE_SVC_LOCK_ERROR            ((DWORD)0xC000006AL)
#define STATUS_CREATE_SVC_UNKNOWN               ((DWORD)0xC000006BL)
#define STATUS_DELETE_SVC                       ((DWORD)0xC000006CL)
#define STATUS_REGISTER_SERVICE_CONTROL_HANDLER ((DWORD)0xC000006DL)
#define STATUS_OPEN_SCM_ACCESS_DENIED           ((DWORD)0xC000006EL)
#define STATUS_OPEN_SCM_UNKNOWN                 ((DWORD)0xC000006FL)
#define STATUS_OPEN_SERVICE_ACCESS_DENIED       ((DWORD)0xC0000070L)
#define STATUS_OPEN_SERVICE_INVALID_NAME        ((DWORD)0xC0000071L)
#define STATUS_OPEN_SERVICE_UNKNOWN             ((DWORD)0xC0000072L)
#define STATUS_START_SVC_NO_THREAD              ((DWORD)0xC0000073L)
#define STATUS_START_SVC_REQUEST_TIMEOUT        ((DWORD)0xC0000074L)
#define STATUS_START_SVC_UNKNOWN                ((DWORD)0xC0000075L)

/****************************************************************************/
/* Service-Specific Variables                                               */
/****************************************************************************/

static SERVICE_STATUS_HANDLE        hServiceStatus;
static SERVICE_STATUS               stServiceStatus;

static HKEY                         hRegKey;

static TCHAR                        tszServiceDescription[1024] = _T("");

static BYTE                         byProgramMajor,     // Major portion of version number
                                    byProgramMinor,     // Minor portion of version number
                                    byProgramHotFix;    // Hot Fix portion of version number
static WORD                         wProgramBuild;      // Build Number


/****************************************************************************/
/* OpenSCM() - Obtains handle to the Service Control Manager                */
/****************************************************************************/

static SC_HANDLE OpenSCM( void )
{
    SC_HANDLE hSCM = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

    if( !hSCM )
    {
        if( GetLastError() == ERROR_ACCESS_DENIED)
            stServiceStatus.dwServiceSpecificExitCode = STATUS_OPEN_SCM_ACCESS_DENIED;
        else
            stServiceStatus.dwServiceSpecificExitCode = STATUS_OPEN_SCM_UNKNOWN;

        stServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    return( hSCM );
}

/****************************************************************************/
/* OpenSvc() - Obtains handle for this Service                              */
/****************************************************************************/

static SC_HANDLE OpenSvc( SC_HANDLE hSCM )
{
    SC_HANDLE hService = OpenService( hSCM, tszServiceName, SERVICE_ALL_ACCESS );

    if( !hService )
    {
        switch( GetLastError())
        {
        case ERROR_ACCESS_DENIED:

            stServiceStatus.dwServiceSpecificExitCode = STATUS_OPEN_SERVICE_ACCESS_DENIED;
            break;

        case ERROR_INVALID_NAME:

            stServiceStatus.dwServiceSpecificExitCode = STATUS_OPEN_SERVICE_INVALID_NAME;
            break;

        case ERROR_SERVICE_DOES_NOT_EXIST:

            break;

        default:

            stServiceStatus.dwServiceSpecificExitCode = STATUS_OPEN_SERVICE_UNKNOWN;
            break;
        }

        if( stServiceStatus.dwServiceSpecificExitCode != STATUS_SUCCESS )
            stServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    return( hService );
}

/****************************************************************************/
/* OpenRegistry() - Opens a connection to the registry.                     */
/****************************************************************************/

static BOOL OpenRegistry( void )
{
    TCHAR tszKeyPath[129];

    _stprintf( tszKeyPath, _T( "System\\CurrentControlSet\\Services\\%s"), tszServiceName );

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, tszKeyPath, 0, KEY_ALL_ACCESS, &hRegKey ) == ERROR_SUCCESS )
        return( TRUE );
    else
        return( FALSE );
}

/****************************************************************************/
/* CloseRegistry() - Closes connection to the registry                      */
/****************************************************************************/

static void CloseRegistry( void )
{
    RegCloseKey( hRegKey );
    hRegKey = 0;
}

/****************************************************************************/
/* GetInstalledVersion() - Retrieves version number for the (previously)    */
/* installed version of the service and converts it into a string.          */
/****************************************************************************/

BOOL GetInstalledVersion( LPTSTR pszBuffer )
{
    DWORD   dwVal, dwStatus, dwMax = sizeof(dwVal);
    BOOL    bOpened = FALSE, bSuccess = FALSE;

    if( hRegKey == 0 )
    {
        if( !OpenRegistry() )
            return( FALSE);

        bOpened = TRUE;
    }

    dwStatus = RegQueryValueEx( hRegKey, _T("Version"), NULL, NULL, (LPBYTE)&dwVal, &dwMax );

    if( dwStatus == ERROR_SUCCESS )
    {
        BYTE byMajor  = (BYTE)(dwVal >> 24);
        BYTE byMinor  = (BYTE)(dwVal >> 16);
        BYTE byHotFix = (BYTE)(dwVal >>  8);

        WORD wBuild   = (WORD)(dwVal & 0xFF);

        if( wBuild == 0 )
        {
            dwStatus = RegQueryValueEx( hRegKey, _T("Build"), NULL, NULL, (LPBYTE)&dwVal, &dwMax );

            if( dwStatus == ERROR_SUCCESS )
                wBuild = (WORD)dwVal;
        }

        if( dwStatus == ERROR_SUCCESS )
        {
            if( byMajor == 0 )
            {
                // Old version without Hot Fix number included, need to adjust

                byMajor  = byMinor;
                byMinor  = byHotFix;
                byHotFix = 0;
            }

            _stprintf( pszBuffer, _T( "v%d.%d.%d.%d"), byMajor, byMinor, byHotFix, wBuild );
            bSuccess = TRUE;
        }
    }

    if( bOpened )
        CloseRegistry();

    return( bSuccess );
}

/****************************************************************************/
/* PutInstalledVersion() - Stores the version number for the (now)          */
/* installed version of the service into the registry.                      */
/****************************************************************************/

BOOL PutInstalledVersion( void )
{
    DWORD dwStatus;
    BOOL  bOpened = FALSE;

    DWORD dwVal = ((DWORD)byProgramMajor  << 24)
                | ((DWORD)byProgramMinor  << 16)
                | ((DWORD)byProgramHotFix <<  8);

    if( hRegKey == 0 )
    {
        if( !OpenRegistry() )
            return( FALSE );

        bOpened = TRUE;
    }

    dwStatus = RegSetValueEx( hRegKey, _T("Version"), 0, REG_DWORD, (LPBYTE)&dwVal, (DWORD)sizeof(dwVal) );

    if( dwStatus == ERROR_SUCCESS )
    {
        dwVal = (DWORD)wProgramBuild;
        dwStatus = RegSetValueEx( hRegKey, _T("Build"), 0, REG_DWORD, (LPBYTE)&dwVal, (DWORD)sizeof(dwVal) );
    }

    if( bOpened )
        CloseRegistry();

    return( dwStatus == ERROR_SUCCESS );
}

/****************************************************************************/
/* IsInstalled() - Indicates whether service is installed                   */
/****************************************************************************/

static BOOL IsInstalled( void )
{
    BOOL      bInstalled = FALSE;
    SC_HANDLE hSCM = OpenSCM();

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            bInstalled = TRUE;
            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSCM );
    }

    return( bInstalled );
}

/****************************************************************************/
/* Query() - Queries the state of the Service                               */
/****************************************************************************/

static DWORD Query( void )
{
    DWORD     dwState = 0;
    SC_HANDLE hSCM = OpenSCM();

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            SERVICE_STATUS stStatus;

            if( QueryServiceStatus( hService, &stStatus ) )
                dwState = stStatus.dwCurrentState;

            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSCM );
    }

    return( dwState );
}

/****************************************************************************/
/* Uninstall() - Uninstalls the Service                                     */
/****************************************************************************/

static BOOL Uninstall( void )
{
    SC_HANDLE       hSCM = OpenSCM();
    SERVICE_STATUS  stServiceStatus2;
    BOOL            bSuccessful = FALSE;

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            ControlService( hService, SERVICE_CONTROL_STOP, &stServiceStatus2 );
            Sleep( 1000 );

            if( !DeleteService( hService ) )
            {
                stServiceStatus.dwServiceSpecificExitCode   = STATUS_DELETE_SVC;
                stServiceStatus.dwWin32ExitCode             = ERROR_SERVICE_SPECIFIC_ERROR;
            }
            else
                bSuccessful = TRUE;

            CloseServiceHandle( hService );
        }
        else
        {
            if( stServiceStatus.dwServiceSpecificExitCode == STATUS_SUCCESS )
                bSuccessful = TRUE;
        }

        CloseServiceHandle( hSCM );
    }

    return( bSuccessful );
}

/****************************************************************************/
/* Install() - Installs the Service                                         */
/****************************************************************************/

static BOOL Install( void )
{
    BOOL                    bSuccessful = FALSE;
    TCHAR                   tszFilePath[MAX_PATH];
    SC_LOCK                 hLock;
    SERVICE_DESCRIPTION     sdBuf;
    SC_HANDLE               hService;
    SC_HANDLE               hSCM = OpenSCM();

    if( hSCM )
    {
        GetModuleFileName( NULL, tszFilePath, sizeof(tszFilePath) );

        hService = CreateService( hSCM, tszServiceName, tszServiceName,
            SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
            SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, tszFilePath, NULL, NULL, NULL, NULL, NULL);

        if( hService )
        {
            hLock = LockServiceDatabase( hSCM );

            if( hLock )
            {
                if( PutInstalledVersion() )
                {
                    sdBuf.lpDescription = (LPTSTR)tszServiceDescription;

                    if( ChangeServiceConfig2( hService, SERVICE_CONFIG_DESCRIPTION, &sdBuf ) )
                    {
                        UnlockServiceDatabase( hLock );

                        if( StartService( hService, 0, NULL ) )
                            bSuccessful = TRUE;
                        else
                        {
                            switch( GetLastError() )
                            {
                            case ERROR_SERVICE_NO_THREAD:

                                stServiceStatus.dwServiceSpecificExitCode = STATUS_START_SVC_NO_THREAD;
                                break;

                            case ERROR_SERVICE_REQUEST_TIMEOUT:

                                stServiceStatus.dwServiceSpecificExitCode = STATUS_START_SVC_REQUEST_TIMEOUT;
                                break;

                            default:

                                stServiceStatus.dwServiceSpecificExitCode = STATUS_START_SVC_UNKNOWN;
                                break;

                            }

                            DeleteService( hService );
                        }
                    }
                    else
                    {
                        stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_DESCRIPTION_ERROR;
                        UnlockServiceDatabase( hLock );
                        DeleteService( hService );
                    }
                }
                else
                {
                    stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_VERSION_ERROR;
                    UnlockServiceDatabase( hLock );
                    DeleteService( hService );
                }
            }
            else
            {
                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_LOCK_ERROR;
                DeleteService( hService );
            }
        }
        else
        {
            switch( GetLastError() )
            {
            case ERROR_ACCESS_DENIED:

                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_ACCESS_DENIED;
                break;

            case ERROR_CIRCULAR_DEPENDENCY:

                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_CIRCULAR_DEPENDENCY;
                break;

            case ERROR_DUP_NAME:

                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_DUP_NAME;
                break;

            case ERROR_INVALID_NAME:

                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_INVALID_NAME;
                break;

            default:

                stServiceStatus.dwServiceSpecificExitCode = STATUS_CREATE_SVC_UNKNOWN;
            }

            stServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        }

        CloseServiceHandle( hSCM );
    }

    return( bSuccessful );
}

/****************************************************************************/
/* Pause() - Pauses the Service                                             */
/****************************************************************************/

static BOOL Pause( void )
{
    SC_HANDLE       hSCM = OpenSCM();
    SERVICE_STATUS  stServiceStatus2;
    BOOL            bSuccessful = FALSE;

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            if( ControlService( hService, SERVICE_CONTROL_PAUSE, &stServiceStatus2 ) )
                bSuccessful = TRUE;

            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSCM );
    }

    return( bSuccessful );
}

/****************************************************************************/
/* Continue() - Continues the Service                                       */
/****************************************************************************/

static BOOL Continue( void )
{
    SC_HANDLE       hSCM = OpenSCM();
    SERVICE_STATUS  stServiceStatus2;
    BOOL            bSuccessful = FALSE;

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            if( ControlService( hService, SERVICE_CONTROL_CONTINUE, &stServiceStatus2 ) )
                bSuccessful = TRUE;

            CloseServiceHandle( hService );
        }

        CloseServiceHandle( hSCM );
    }

    return( bSuccessful );
}

/****************************************************************************/
/* Stop() - Stops the Service                                               */
/****************************************************************************/

static BOOL Stop( void )
{
    SC_HANDLE       hSCM = OpenSCM();
    SERVICE_STATUS  stServiceStatus2;
    BOOL            bSuccessful = FALSE;

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            if( ControlService( hService, SERVICE_CONTROL_STOP, &stServiceStatus2 ) )
                bSuccessful = TRUE;

            CloseServiceHandle( hService );
        }
    }

    return( bSuccessful );
}

/****************************************************************************/
/* Start() - Starts the Service                                             */
/****************************************************************************/

static BOOL Start( void )
{
    SC_HANDLE   hSCM = OpenSCM();
    BOOL        bSuccessful = FALSE;

    if( hSCM )
    {
        SC_HANDLE hService = OpenSvc( hSCM );

        if( hService )
        {
            if( StartService( hService, 0, NULL ) )
                bSuccessful = TRUE;

            CloseServiceHandle( hService );
        }
    }

    return( bSuccessful );
}

/****************************************************************************/
/* SetStatus() - Sets the status for the Service                            */
/****************************************************************************/

static void SetStatus( DWORD dwState, DWORD dwDelay )
{
    stServiceStatus.dwCurrentState = dwState;
    stServiceStatus.dwWaitHint     = dwDelay;

    SetServiceStatus( hServiceStatus, &stServiceStatus );
}

/****************************************************************************/
/* ServiceControl() - Service's Control Handler function. Processes control */
/* requests from the Service Manager.                                       */
/****************************************************************************/

static DWORD WINAPI ServiceControl( DWORD dwCCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext )
{
    DWORD dwStatus  = NO_ERROR;
    DWORD dwDelay   = 0;

    switch( dwCCode )
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:      // Want us to get out of dodge

        SetStatus( SERVICE_STOP_PENDING, 0 );

        dwDelay = DoServiceShutdown();

        if( dwDelay )
            SetStatus( SERVICE_STOP_PENDING, dwDelay + 2 );

        break;

    case SERVICE_CONTROL_PAUSE:         // Want us to pause operation

        SetStatus( SERVICE_PAUSE_PENDING, 0 );

        dwDelay = DoServicePause();

        if( dwDelay )
        {
            SetStatus( SERVICE_PAUSE_PENDING, dwDelay + 2 );
            Sleep( dwDelay );
        }

        SetStatus( SERVICE_PAUSED, 0 );
        break;

    case SERVICE_CONTROL_CONTINUE:      // Want us to continue (resume) operation

        SetStatus( SERVICE_CONTINUE_PENDING, 0 );

        dwDelay = DoServiceContinue();

        if( dwDelay )
        {
            SetStatus( SERVICE_CONTINUE_PENDING, dwDelay + 2 );
            Sleep( dwDelay );
        }

        SetStatus( SERVICE_RUNNING, 0 );
        break;

    case SERVICE_CONTROL_POWEREVENT:    // Received powwer management event

        switch( dwEventType )
        {
        case PBT_APMSUSPEND:                // System suspending...

            dwDelay = DoServiceSuspend();

            if( dwDelay )
                Sleep( dwDelay );

            break;

        case PBT_APMRESUMEAUTOMATIC:
        case PBT_APMRESUMECRITICAL:
        case PBT_APMRESUMESUSPEND:          // System resuming...

            dwDelay = DoServiceResume();

            if( dwDelay )
                Sleep( dwDelay );

            break;
        }

        SetStatus( SERVICE_RUNNING, 0 );
        break;

    default:                            // Want us to process application-specific command code

        if( (dwServiceCapabilities & SUPPORT_CUSTOM_CONTROLS) && (dwCCode >= SERVICE_CONTROL_USER) )
        {
            dwStatus = DoServiceOpcode( dwCCode, &dwDelay );

            if( dwStatus != ERROR_CALL_NOT_IMPLEMENTED )
                Sleep( dwDelay );
        }
        else
            dwStatus = ERROR_CALL_NOT_IMPLEMENTED;

        SetStatus( SERVICE_RUNNING, 0 );
        break;
    }

    return( dwStatus );

    UNREFERENCED_PARAMETER( lpEventData );
    UNREFERENCED_PARAMETER( lpContext );
}

/****************************************************************************/
/* ServiceMain() - Service's Main function                                  */
/****************************************************************************/

static VOID WINAPI ServiceMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the service

    stServiceStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    stServiceStatus.dwWin32ExitCode           = NO_ERROR;
    stServiceStatus.dwServiceSpecificExitCode = NO_ERROR;
    stServiceStatus.dwCheckPoint              = 0;
    stServiceStatus.dwWaitHint                = 0;
    stServiceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    if( (dwServiceCapabilities & SUPPORT_PAUSE_CONTINUE) )
        stServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;

    if( (dwServiceCapabilities & SUPPORT_SUSPEND_RESUME) )
        stServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_POWEREVENT;

    hServiceStatus = RegisterServiceCtrlHandlerEx( tszServiceName, ServiceControl, NULL );

    // Registration completed; get the service running...

    if( hServiceStatus != NULL )
    {
        SetStatus( SERVICE_START_PENDING, 0 );

        if( DoServiceInitialize( dwArgc, lpszArgv ) )
        {
            SetStatus( SERVICE_RUNNING, 0 );
            DoServiceWork();
        }

        SetStatus( SERVICE_STOPPED, 0 );
    }
}

/****************************************************************************/
/* ServiceStart() - Installs, Uninstalls or Starts the service. Command     */
/* line argument is parsed and the appropriate action performed:            */
/*                                                                          */
/*      -i      Installs the Service and starts it running                  */
/*      -is     Silently installs the Service and starts it running         */
/*      -u      Uninstalls the Service                                      */
/*      -us     Silently uninstalls the Service                             */
/*      -p      Pauses the Service                                          */
/*      -ps     Silently pauses the Service                                 */
/*      -r      Resumes the Service                                         */
/*      -rs     Silently resumes the Service                                */
/*      -b      Starts (begins) the Service                                 */
/*      -bs     Silently starts (begins) the Service                        */
/*      -e      Stops (ends) the Service                                    */
/*      -es     Silently stops (ends) the Service                           */
/*      <none>  Starts the Service running                                  */
/*                                                                          */
/****************************************************************************/

static SERVICE_TABLE_ENTRY stDispatchTable[] =
{
    { tszServiceName, ServiceMain },
    { NULL,           NULL        }
};

int ServiceStart( LPCTSTR ptszServiceName, LPCTSTR ptszServiceDescription,
                  LPCTSTR ptszServiceWindow, BYTE byMajor, BYTE byMinor,
                  BYTE byHotFix, WORD wBuild,
                  DWORD dwServiceCapabilitiesAccepted, int iArgs, LPTSTR pszArg[] )
{
    static TCHAR    tszMessage[133], tszPrevVersion[33], tszThisVersion[33], tszServiceNameVer[133];
    BOOL            bInstalled, bSilent = FALSE;
    DWORD           dwStatus;
    int             iStatus;

    // Save off service info

    byProgramMajor        = byMajor;
    byProgramMinor        = byMinor;
    byProgramHotFix       = byHotFix;
    wProgramBuild         = wBuild;
    dwServiceCapabilities = dwServiceCapabilitiesAccepted;

    _tcscpy( tszServiceName, ptszServiceName );
    _tcscpy( tszServiceDescription, ptszServiceDescription );
    _tcscpy( tszServiceWindow, ptszServiceWindow );

    // Prepare service and version strings

    _stprintf( tszThisVersion, _T( "v%d.%d.%d.%d"), byMajor, byMinor, byHotFix, wBuild );
    _stprintf( tszServiceNameVer, _T( "%s %s"), ptszServiceName, tszThisVersion );

    // Check for command line parameter; if there's none, we're executing
    // as a service or pseudo-service (or failing)...

    if( iArgs <= 1 )
    {
        // Attempt start of Service Dispatcher

        if( StartServiceCtrlDispatcher( stDispatchTable ) )
            return( ERROR_SUCCESS );

        // If we can't connect to the Service Manager, it's an indication that
        // we have probably been started from the command line. In this case,
        // and if a desire has been indicated by the Service, we will provide a
        // pseudo-service environment for it to run in.

        dwStatus = GetLastError();

        if( (dwStatus == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT) && (dwServiceCapabilities & SUPPORT_PSEUDO_SERVICE) )
            dwStatus = RunAsPseudoService();

        return( (int)dwStatus );
    }

    // Process Install requests

    bInstalled = IsInstalled();

    if( bInstalled )
    {
        if( !GetInstalledVersion( tszPrevVersion ) )
            _tcscpy( tszPrevVersion, _T("v1.0.0.0") );
    }


    if( _tcsicmp( pszArg[1], _T("-is") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-i") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( bSilent )
                iStatus = IDYES;
            else
            {
                _stprintf( tszMessage, _T( "%s %s Already Installed, Replace?"), tszServiceName, tszPrevVersion );
                iStatus = MessageBox( NULL, tszMessage, tszServiceName, MB_YESNO | MB_DEFBUTTON2 );
            }

            if( iStatus == IDYES )
            {
                Uninstall();

                if( Install() )
                {
                    if( !bSilent )
                    {
                        _stprintf( tszMessage, _T( "%s Successfully Installed and Started"), tszServiceNameVer );
                        MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                    }

                    return( ERROR_SUCCESS );
                }
                else
                {
                    dwStatus = GetLastError();

                    if( !bSilent )
                    {
                        switch( dwStatus )
                        {
                        case ERROR_SERVICE_REQUEST_TIMEOUT:
                        case ERROR_SERVICE_NO_THREAD:

                            _stprintf( tszMessage, _T("%s Install Failed, Could Not Start"), tszServiceNameVer );
                            break;

                        default:

                            _stprintf( tszMessage, _T("%s Install Failed, ccode = %d"), tszServiceNameVer, dwStatus );
                            break;
                        }

                        MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                    }

                    return( (int)dwStatus );
                }
            }
            else
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T( "%s Install Cancelled"), tszServiceNameVer );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_CANCELLED );
            }
        }
        else
        {
            if( Install() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T( "%s Successfully Installed"), tszServiceNameVer );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    switch( dwStatus )
                    {
                    case ERROR_SERVICE_REQUEST_TIMEOUT:
                    case ERROR_SERVICE_NO_THREAD:

                        _stprintf( tszMessage, _T("%s Install Failed, Could Not Start"), tszServiceNameVer );
                        break;

                    default:

                        _stprintf( tszMessage, _T("%s Install Failed, ccode = %d"), tszServiceNameVer, dwStatus );
                        break;
                    }

                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
    }

    // Process Uninstall requests

    if( _tcsicmp( pszArg[1], _T("-us") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-u") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( Uninstall() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Successfully Uninstalled"), tszServiceName, tszPrevVersion );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Uninstall Failed, ccode = %d"), tszServiceName, tszPrevVersion, dwStatus );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
        else
        {
            if( !bSilent )
            {
                _stprintf( tszMessage, _T( "%s Is Not Installed"), tszServiceName );
                MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
            }

            return( ERROR_NOT_FOUND );
        }
    }

    // Process Pause Requests

    if( _tcsicmp( pszArg[1], _T("-ps") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-p") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( Pause() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Successfully Paused"), tszServiceName, tszPrevVersion );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    if( dwStatus == ERROR_SERVICE_NOT_ACTIVE )
                        _stprintf( tszMessage, _T("%s %s Is Not Running"), tszServiceName, tszPrevVersion );
                    else
                        _stprintf( tszMessage, _T("%s %s Pause Failed, ccode = %d"), tszServiceName, tszPrevVersion, dwStatus );

                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
        else
        {
            if( !bSilent )
            {
                _stprintf( tszMessage, _T("%s Is Not Installed"), tszServiceName );
                MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
            }

            return( ERROR_NOT_FOUND );
        }
    }

    // Process Resume Requests

    if( _tcsicmp( pszArg[1], _T("-cs") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-c") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( Continue() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Successfully Continued"), tszServiceName, tszPrevVersion );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    if( dwStatus == ERROR_SERVICE_NOT_ACTIVE )
                        _stprintf( tszMessage, _T("%s %s Is Not Running"), tszServiceName, tszPrevVersion );
                    else
                        _stprintf( tszMessage, _T("%s %s Continue Failed, ccode = %d"), tszServiceName, tszPrevVersion, dwStatus );

                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
        else
        {
            if( !bSilent )
            {
                _stprintf( tszMessage, _T("%s Is Not Installed"), tszServiceName );
                MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
            }

            return( ERROR_NOT_FOUND );
        }
    }

    // Process Start Requests

    if( _tcsicmp( pszArg[1], _T("-bs") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-b") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( Start() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Successfully Started"), tszServiceName, tszPrevVersion );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    if( dwStatus == ERROR_SERVICE_ALREADY_RUNNING )
                        _stprintf( tszMessage, _T("%s %s Is Already Running"), tszServiceName, tszPrevVersion );
                    else
                        _stprintf( tszMessage, _T("%s %s Start Failed, ccode = %d"), tszServiceName, tszPrevVersion, dwStatus );

                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
        else
        {
            if( !bSilent )
            {
                _stprintf( tszMessage, _T("%s Is Not Installed"), tszServiceName );
                MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
            }

            return( ERROR_NOT_FOUND );
        }
    }

    // Process Stop Requests

    if( _tcsicmp( pszArg[1], _T("-es") ) == 0 )
        bSilent = TRUE;

    if( (_tcsicmp( pszArg[1], _T("-e") ) == 0) || bSilent )
    {
        if( bInstalled )
        {
            if( Stop() )
            {
                if( !bSilent )
                {
                    _stprintf( tszMessage, _T("%s %s Successfully Stopped"), tszServiceName, tszPrevVersion );
                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( ERROR_SUCCESS );
            }
            else
            {
                dwStatus = GetLastError();

                if( !bSilent )
                {
                    if( dwStatus == ERROR_SERVICE_NOT_ACTIVE )
                        _stprintf( tszMessage, _T("%s %s Is Not Running"), tszServiceName, tszPrevVersion );
                    else
                        _stprintf( tszMessage, _T("%s %s Stop Failed, ccode = %d"), tszServiceName, tszPrevVersion, dwStatus );

                    MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
                }

                return( (int)dwStatus );
            }
        }
        else
        {
            if( !bSilent )
            {
                _stprintf( tszMessage, _T("%s Is Not Installed"), tszServiceName );
                MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
            }

            return( ERROR_NOT_FOUND );
        }
    }

    // Process query requests

    if( _tcsicmp( pszArg[1], _T("-q") ) == 0 )
    {
        if( bInstalled )
        {
            TCHAR tszState[33];

            dwStatus = ERROR_SUCCESS;

            switch( Query() )
            {
            case SERVICE_PAUSED:
            case SERVICE_PAUSE_PENDING:

                _tcscpy( tszState, _T("Is Paused") );
                break;

            case SERVICE_RUNNING:
            case SERVICE_CONTINUE_PENDING:
            case SERVICE_START_PENDING:

                _tcscpy( tszState, _T("Is Running") );
                break;

            case SERVICE_STOPPED:
            case SERVICE_STOP_PENDING:

                _tcscpy( tszState, _T("Is Stopped") );
                break;

            default:

                dwStatus = GetLastError();
                _stprintf( tszState, _T("Query Failed, ccode = %d"), dwStatus );
                break;
            }

            _stprintf( tszMessage, _T("%s %s %s"), tszServiceName, tszPrevVersion, tszState );
        }
        else
        {
            dwStatus = ERROR_NOT_FOUND;
            _stprintf( tszMessage, _T("%s Is Not Installed"), tszServiceName );
        }

        MessageBox( NULL, tszMessage, tszServiceName, MB_OK );
        return( (int)dwStatus );
    }

    // No argument we recognize; tell invoker what arguments are supported

    MessageBox( NULL, _T( "Parameters:\n\n"
                            "   -i     Install Service\n"
                            "   -is    Silently Install Service\n"
                            "   -u     Uninstall Service\n"
                            "   -us    Silently Uninstall Service\n"
                            "   -p     Pauses the Service\n"
                            "   -ps    Silently pauses the Service\n"
                            "   -c     Continues the Service\n"
                            "   -cs    Silently continues the Service\n"
                            "   -b     Starts (begins) the Service\n"
                            "   -bs    Silently starts (begins) the Service\n"
                            "   -e     Stops (ends) the Service\n"
                            "   -es    Silently stops (ends) the Service\n"
                            "   -q     Query state of the Service\n"
                          ), tszServiceName, MB_OK );

    if( _tcsicmp( pszArg[1], _T("-?") ) )
        return( ERROR_NOT_SUPPORTED );
    else
        return( ERROR_SUCCESS );
}
