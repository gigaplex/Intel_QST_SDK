@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:     Libraries\Windows\Build.bat                          rem
rem                                                                       rem
rem   Description:   Builds Windows  versions  of  the  Intel(R)  Quiet   rem
rem                  System   Technology   (QST)   Instrumentation  and   rem
rem                  Communications libraries. Also builds the libSMART   rem
rem                  library, which  provides  support  for  extracting   rem
rem                  S.M.A.R.T. data from compliant hard drives.          rem
rem                                                                       rem
rem remremremremremremremremremremremremremremremremremremremremremremrem rem

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem    Copyright (c) 2005-2009, Intel Corporation. All Rights Reserved.   rem
rem                                                                       rem
rem   Redistribution and use  in  source  and  binary  forms,  with  or   rem
rem   without  modification,  are permitted provided that the following   rem
rem   conditions are met:                                                 rem
rem                                                                       rem
rem     - Redistributions  of  source  code  must  retain   the   above   rem
rem       copyright  notice,  this list of conditions and the following   rem
rem       disclaimer.                                                     rem
rem                                                                       rem
rem     - Redistributions in  binary  form  must  reproduce  the  above   rem
rem       copyright  notice,  this list of conditions and the following   rem
rem       disclaimer  in  the  documentation  and/or  other   materials   rem
rem       provided with the distribution.                                 rem
rem                                                                       rem
rem     - Neither the name of Intel Corporation nor the  names  of  its   rem
rem       contributors  may  be  used  to  endorse  or promote products   rem
rem       derived from this software  without  specific  prior  written   rem
rem       permission.                                                     rem
rem                                                                       rem
rem   DISCLAIMER: THIS SOFTWARE IS PROVIDED BY  THE  COPYRIGHT  HOLDERS   rem
rem   AND  CONTRIBUTORS  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,   rem
rem   INCLUDING,  BUT  NOT  LIMITED  TO,  THE  IMPLIED  WARRANTIES   OF   rem
rem   MERCHANTABILITY   AND   FITNESS  FOR  A  PARTICULAR  PURPOSE  ARE   rem
rem   DISCLAIMED.  IN  NO  EVENT  SHALL  INTEL   CORPORATION   OR   THE   rem
rem   CONTRIBUTORS  BE  LIABLE  FOR  ANY  DIRECT, INDIRECT, INCIDENTAL,   rem
rem   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  NOT   rem
rem   LIMITED  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF   rem
rem   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER  CAUSED   rem
rem   AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER IN CONTRACT, STRICT   rem
rem   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN   rem
rem   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  OF  THE   rem
rem   POSSIBILITY OF SUCH DAMAGE.                                         rem
rem                                                                       rem
rem remremremremremremremremremremremremremremremremremremremremremremrem rem

echo Building Release DLLs . . .

echo ************************* QstComm Release ************************* >Build.log
msdev "QstComm.dsp" /make "QstComm - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Release\QstComm.dll goto FAILED

echo ************************* QstInst Release ************************* >>Build.log
msdev "QstInst.dsp" /make "QstInst - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Release\QstInst.dll goto FAILED

echo *********************** QstProxyComm Release ********************** >Build.log
msdev "QstProxyComm.dsp" /make "QstProxyComm - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Proxy_Release\QstComm.dll goto FAILED

echo *********************** QstProxyInst Release ********************** >>Build.log
msdev "QstProxyInst.dsp" /make "QstProxyInst - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Proxy_Release\QstInst.dll goto FAILED

echo ************************* libSMART Release ************************ >>Build.log
msdev "libSMART.dsp" /make "libSMART - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Release\libSMART.dll goto FAILED

echo Building Debug DLLs . . .

echo ************************** QstComm Debug ************************** >>Build.log
msdev "QstComm.dsp" /make "QstComm - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Debug\QstComm.dll goto FAILED

echo ************************** QstInst Debug ************************** >>Build.log
msdev "QstInst.dsp" /make "QstInst - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Debug\QstInst.dll goto FAILED

echo ************************ QstProxyComm Debug *********************** >>Build.log
msdev "QstProxyComm.dsp" /make "QstProxyComm - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Proxy_Debug\QstComm.dll goto FAILED

echo ************************ QstProxyInst Debug *********************** >>Build.log
msdev "QstProxyInst.dsp" /make "QstProxyInst - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Proxy_Debug\QstInst.dll goto FAILED

echo ************************** libSMART Debug ************************* >>Build.log
msdev "libSMART.dsp" /make "libSMART - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto FAILED
if not exist Debug\libSMART.dll goto FAILED

echo Building ActiveX Controls...

echo ********************** QstInstX Debug/Release ********************* >>Build.log

pushd QstInstX
call Build.bat >>Build.log
popd
if not exist QstInstX\Debug\QstInstX.dll goto FAILED
if not exist QstInstX\Release\QstInstX.dll goto FAILED

echo Windows DLLs Built Successfully!
goto DONE

:FAILED

echo Windows DLLs Build Failed!
set FAILED=TRUE

:DONE

if "%1" == "" pause


