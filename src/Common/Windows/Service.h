/****************************************************************************/
/*                                                                          */
/*  Module:         Service.h                                               */
/*                                                                          */
/*  Description:    This file defines the entry points and call-backs used  */
/*                  within and invoked  by  the  Service  Framework.  This  */
/*                  Framework   is   designed  to  simplify  the  task  of  */
/*                  implementing Windows Services. It's  features  can  be  */
/*                  summarized as follows:                                  */
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

#ifndef _SERVICE_H
#define _SERVICE_H

#include <windows.h>
#include <tchar.h>

/****************************************************************************/
/* WM_PSEUDO_SERVICE_COMMAND - Command code used to send a Service-specific */
/* command to the Service when running as a Pseudo-Service.                 */
/****************************************************************************/

#define WM_PSEUDO_SERVICE_COMMAND   (UINT)0x8888

/****************************************************************************/
/*  ServiceStart() - Main entry-point for the Service  Framework.  Through  */
/*  the  passing of command-line parameters, allows the same executable to  */
/*  be used as a utility for controlling the Service, as well as implement  */
/*  the  Service  functionality itself. As a Utility, it accepts any (one)  */
/*  of the following parameters:                                            */
/*                                                                          */
/*  -i      Install and start a Service executing                           */
/*  -is     Silently install and start a Service executing                  */
/*  -u      Uninstall the Service (stops executing instance)                */
/*  -us     Silently uninstall the Service (stops executing instance)       */
/*  -p      Pause executing instance of the Service                         */
/*  -ps     Silently pause executing instance of the Service                */
/*  -r      Resume paused instance of the Service                           */
/*  -rs     Silently resume paused instance of the Service                  */
/*  -b      Start (begin) the Service executing                             */
/*  -bs     Silently start (begin) the Service executing                    */
/*  -e      Stop (end) the executing instance of the Service                */
/*  -es     Silently stop (end) the executing instance of the Service       */
/*  -q      Query execution status for the Service                          */
/*                                                                          */
/*  Notes:                                                                  */
/*                                                                          */
/*  1.  The location of the executable file that is being used to  request  */
/*      the  Service's  installation  will  be  retained  and  used by the  */
/*      Service Manager to start all subsequent instances of the Service.   */
/*                                                                          */
/*  2.  Since Services cannot begin execution from the command-line  (they  */
/*      can  only  be  started  by  the service manager), an instance of a  */
/*      service initiated from the command prompt that does not  pass  any  */
/*      parameters to the Framework will immediately exit.                  */
/*                                                                          */
/*  3.  If initiated as a Service, control will not  be  returned  to  the  */
/*      invoking  function  (main()  for console apps; WinMain() for Win32  */
/*      apps) until Service termination is complette. That  is,  the  last  */
/*      line of a Service's main() function is typically:                   */
/*                                                                          */
/*                  return( ServiceStart( ... ) );                          */
/*                                                                          */
/*  This routine's parameters are detailed as follows:                      */
/*                                                                          */
/*  ptszServiceName         - String providing the name of the service.     */
/*                                                                          */
/*  ptszServiceDescription  - String providing description for the service  */
/*                            (appears in Services applet).                 */
/*                                                                          */
/*  ptszServiceWindow       - String providing name for Window Class used   */
/*                            when service is running as a pseudo-service.  */
/*                                                                          */
/*  byVersionMajor          - Major (MA) component of version number.       */
/*                                                                          */
/*  byVersionMonor          - Minor (MI) component of version number.       */
/*                                                                          */
/*  byBuildNumber           - Build Number (BN).                            */
/*                                                                          */
/*                            Note: The above three parameters are used to  */
/*                            build a version string of form MA.MI.BN.      */
/*                                                                          */
/*  iControlsAccepted       - Bitmap valus which indicates what  types  of  */
/*                            operations the Service supports. Set this to  */
/*                            one or more of the following:                 */
/*                                                                          */
/*                            SUPPORT_PAUSE_CONTINUE - Indicates that  the  */
/*                              service supports being paused and that the  */
/*                              DoServicePause()  and  DoServiceContinue()  */
/*                              call-backs  should  be  invoked  when  the  */
/*                              Pause/Resume commands are received.         */
/*                                                                          */
/*                            SUPPORT_SUSPEND_RESUME - Indicates that  the  */
/*                              service  has special processing that needs  */
/*                              to be performed when the system enters and  */
/*                              leaves  low-power  states  and  that   the  */
/*                              DoServiceSuspend()  and  DoServiceResume()  */
/*                              call-backs  should  be  invoked  when  the  */
/*                              Suspend/Resume commands are received.       */
/*                                                                          */
/*                            SUPPORT_CUSTOM_CONTROLS - Indicates that the  */
/*                              service  implements custom control signals  */
/*                              and that the  DoServiceOpcode()  call-back  */
/*                              should  be  invoked  when  a  user-defined  */
/*                              control code is received.                   */
/*                                                                          */
/*                            SUPPORT_PSEUDO_SERVICE - Indicates that,  if  */
/*                              the  Service  is  started from the command  */
/*                              line,  the  framework  should  provide   a  */
/*                              pseudo-service environment for the Service  */
/*                              to  run  in . This feature's purpose is to  */
/*                              allow the Service to be run in an environ-  */
/*                              ment more conducive to debugging.           */
/*                                                                          */
/*  argc                    - Indicates number of arguments. Should  be  2  */
/*                            if  running  in  utility  mode  (to install,  */
/*                            uninstall,  start,  stop,  pause or continue  */
/*                            service execution.                            */
/*                                                                          */
/*  argv                    - Array of pointers to parameter  strings.  If  */
/*                            executing  as  a Service Management utility,  */
/*                            argv[0] should be the  service's  executable  */
/*                            pathname  and  argv[1]  should be the actual  */
/*                            command-line parameter to be processed  (one  */
/*                            of the parameters detailed above).            */
/*                                                                          */
/****************************************************************************/

int ServiceStart( LPCTSTR ptszServiceName, LPCTSTR ptszServiceDescription,
                  LPCTSTR ptszServiceWindow, BYTE byVersionMajor,
                  BYTE byVersionMinor, BYTE byVersionHotFix,
                  WORD byBuildNUmber, DWORD dwControlsAccepted, int argc,
                  LPTSTR argv[] );

/****************************************************************************/
/* Definitions for ServiceStart()'s dwControlsAccepted parameter            */
/****************************************************************************/

#define SUPPORT_PAUSE_CONTINUE      0x01
#define SUPPORT_SUSPEND_RESUME      0x02
#define SUPPORT_CUSTOM_CONTROLS     0x04
#define SUPPORT_PSEUDO_SERVICE      0x08

#define SERVICE_CONTROL_USER        0x80    // First Service-specific OpCode

/****************************************************************************/
/*  DoServiceInitialize() - A Call-Back function that is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to do any necessary initialization. If the  implementation  */
/*  performed all necessary initialization in its WinMain() function, this  */
/*  function may simply return without doing anything.                      */
/****************************************************************************/

BOOL DoServiceInitialize( DWORD dwArgc, LPTSTR *lpszArgv  );

/****************************************************************************/
/*  DoServiceShutdown() - A Call-Back function  that  is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to do  any  necessary  cleanup  processing.  The  function  */
/*  should  return  an  indication, in milliseconds, of the amount of time  */
/*  that  will  be  necessary  to  perform  any  asynchronous   processing  */
/*  necessary  to  complete  the  operation.  Return  value  0  (zero)  if  */
/*  processing is fully completed by the call-back function.                */
/****************************************************************************/

DWORD DoServiceShutdown( void );

/****************************************************************************/
/*  DoServiceWork() - A Call-Back function that is invoked by the  Service  */
/*  Framework  when  all  Service initialization is completed. It provides  */
/*  the  implementation  with  the  opportunity  to   perform   its   main  */
/*  functionality.  The  Service  will  be  terminated  when this function  */
/*  exits (RETURNs).                                                        */
/****************************************************************************/

void DoServiceWork( void );

/****************************************************************************/
/*  DoServicePause() - A Call-Back function that is invoked by the Service  */
/*  Framework  in  order to provide the implementation with an opportunity  */
/*  to perform any processing  necessary  during  entry  into  the  paused  */
/*  state.  The  function should return an indication, in milliseconds, of  */
/*  the amount of time that will be necessary to perform any  asynchronous  */
/*  processing  necessary to complete the operation. Return value 0 (zero)  */
/*  if the operation is fully completed by the call-back function.          */
/****************************************************************************/

DWORD DoServicePause( void );

/****************************************************************************/
/*  DoServiceContinue() - A Call-Back function  that  is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to do any processing  necessary  while  exiting  from  the  */
/*  paused   state.   The   function   should  return  an  indication,  in  */
/*  milliseconds, of the amount of time that will be necessary to  perform  */
/*  any  asynchronous  processing  necessary  to  complete  the operation.  */
/*  Return value 0 (zero) if the operation is fully completed by the call-  */
/*  back function.                                                          */
/****************************************************************************/

DWORD DoServiceContinue( void );

/****************************************************************************/
/*  DoServiceSuspend() - A Call-Back  function  that  is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to perform any processing necessary during entry into  the  */
/*  suspended   state.  The  function  should  return  an  indication,  in  */
/*  milliseconds, of the amount of time that will be necessary to  perform  */
/*  any  asynchronous  processing  necessary  to  complete  the operation.  */
/*  Return value 0 (zero) if the operation is fully completed by the call-  */
/*  back function.                                                          */
/****************************************************************************/

DWORD DoServiceSuspend( void );

/****************************************************************************/
/*  DoServiceResume() - A  Call-Back  function  that  is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to do any processing  necessary  while  exiting  from  the  */
/*  suspended   state.  The  function  should  return  an  indication,  in  */
/*  milliseconds, of the amount of time that will be necessary to  perform  */
/*  any  asynchronous  processing  necessary  to  complete  the operation.  */
/*  Return value 0 (zero) if the operation is fully completed by the call-  */
/*  back function.                                                          */
/****************************************************************************/

DWORD DoServiceResume( void );

/****************************************************************************/
/*  DoServiceOpcode() - A  Call-Back  function  that  is  invoked  by  the  */
/*  Service  Framework  in  order  to  provide  the implementation with an  */
/*  opportunity to process  any  implementation-specific  Control  Signals  */
/*  received  through  Service's  Control Handler. Implementation-specific  */
/*  control  signal  opcodes  are  in  the  range  128-255.  The  callback  */
/*  should return ERROR_CALL_NOT_IMPLEMENTED for all opcodes that are  not  */
/*  supported  by  the  Service  implementation. For opcodes supported and  */
/*  successfully processed, NO_ERROR  should  be  returned.  The  function  */
/*  should   return,   via  variable  pdwAsyncOpTime,  an  indication,  in  */
/*  milliseconds, of the amount of time that will be necessary to  perform  */
/*  any  asynchronous  processing  necessary  to  complete  the operation.  */
/*  Value 0 (zero) should be specified if the operation is fully completed  */
/*  by the call-back function. The  contents  of  this  variable  will  be  */
/*  ignored if the response code is ERROR_CALL_NOT_IMPLEMENTED.             */
/****************************************************************************/

DWORD DoServiceOpcode( DWORD dwOpcode, DWORD *pdwAsyncOpTime  );

#ifdef __cplusplus
}
#endif

#endif /* ndef _SERVICE_H */
