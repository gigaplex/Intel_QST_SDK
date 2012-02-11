@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:    Programs\BuildAll.bat                                 rem
rem                                                                       rem
rem   Description:  Builds DOS and Windows executables for all  of  the   rem
rem                 Intel(R)   Quiet  System  Technology  (QST)  sample   rem
rem                 programs.                                             rem
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

echo Building DOS/Windows Executables for the QST SST/PECI Bus Scan Demo...

pushd BusTest
call Build.bat nopause
copy /Y Build.log ..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo Building DOS/Windows Executables for the QST Instrumentation Layer Demo...

pushd InstTest
call Build.bat nopause
copy /Y ..\Build.log+Build.log ..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo Building DOS/Windows Executables for the QST Status Demo...

pushd StatTest
call Build.bat nopause
copy /Y ..\Build.log+Build.log ..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo Building Windows Executable for the QST Instrumentation Layer ActiveX Wrapper Demo...

pushd InstXTest
call Build.bat nopause
copy /Y ..\Build.log+Build.log ..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo DOS/Windows Programs Built Successfully!
goto DONE

:FAILED
echo DOS/Windows Programs Build Failed!

:DONE
if "%1" == "" pause

