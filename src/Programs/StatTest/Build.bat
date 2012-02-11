@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:     Programs\StatTest\Build.bat                          rem
rem                                                                       rem
rem   Description:   Builds DOS  and  Windows  executables  for  sample   rem
rem                  program   StatTest,   which  demonstrates  how  to   rem
rem                  extract and display status information  from/about   rem
rem                  the   Intel(R)   Quiet   System  Technology  (QST)   rem
rem                  Subsystem.                                           rem
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

rem Building Windows Targets

echo Building Windows Targets . . .

echo ******************************************************************* >Build.log
msdev "StatTest.dsp" /make "StatTest - Win32 Release" /rebuild >>Build.log
if errorlevel 1 goto WINRFAIL

echo ******************************************************************* >>Build.log
msdev "StatTest.dsp" /make "StatTest - Win32 Debug" /rebuild >>Build.log
if errorlevel 1 goto WINDFAIL

rem Building DOS Targets

echo Building DOS Targets . . .

if not exist DOS md DOS

set OLDINCLUDE=%INCLUDE%
set OPTS=/mf /wx /hw /fp6 /6r /bt=dos /i=..\..\Include /i=..\..\Common

rem Try the two standard places for Open Watcom to be installed...
if exist C:\watcom\setvars.bat          call C:\watcom\setvars.bat
if exist C:\progra~1\watcom\setvars.bat call C:\progra~1\watcom\setvars.bat

set BUILD=Release
set MODELOPTS=/ox /d0

goto DOBUILD

:DEBUG
set BUILD=Debug
set MODELOPTS=/od /d2 /dTRACE

:DOBUILD
set TEMPDIR=DOS\%BUILD%

if     exist %TEMPDIR% del /f /q %TEMPDIR%
if not exist %TEMPDIR% md %TEMPDIR%

echo ******************************************************************* >>Build.log

%WATCOM%\binnt\wcc386 StatTest.c               /fo=%TEMPDIR%\StatTest.obj  %MODELOPTS% %OPTS% >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wcc386 ..\..\Common\AccessQst.c /fo=%TEMPDIR%\AccessQst.obj %MODELOPTS% %OPTS% >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wcc386 ..\..\Common\UsageStr.c  /fo=%TEMPDIR%\UsageStr.obj  %MODELOPTS% %OPTS% >>Build.log
if errorlevel 1 goto ERROR

if "%BUILD%" == "Debug" echo debug dwarf >%TEMPDIR%\Build.lnk

echo disable    108                                     >>%TEMPDIR%\Build.lnk
echo @%WATCOM%\binnt\wlink.lnk                          >>%TEMPDIR%\Build.lnk
echo option     dosseg                                  >>%TEMPDIR%\Build.lnk
echo system     pmodew                                  >>%TEMPDIR%\Build.lnk
echo file       %TEMPDIR%\StatTest                      >>%TEMPDIR%\Build.lnk
echo file       %TEMPDIR%\AccessQst                     >>%TEMPDIR%\Build.lnk
echo file       %TEMPDIR%\UsageStr                      >>%TEMPDIR%\Build.lnk
echo library    ..\..\Libraries\DOS\Release\QstInst6r   >>%TEMPDIR%\Build.lnk
echo library    ..\..\Libraries\DOS\Release\QstComm6r   >>%TEMPDIR%\Build.lnk
echo name       %TEMPDIR%\StatTest.exe                  >>%TEMPDIR%\Build.lnk
echo option     map=%TEMPDIR%\StatTest.map              >>%TEMPDIR%\Build.lnk

%WATCOM%\binnt\wlink @%TEMPDIR%\Build >>Build.log
if errorlevel 1 goto ERROR

if "%BUILD%" == "Release" goto DEBUG

%WATCOM%\binnt\wdis %TEMPDIR%\StatTest  /l=%TEMPDIR%\StatTest.lst  /s=StatTest.c               >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wdis %TEMPDIR%\AccessQst /l=%TEMPDIR%\AccessQst.lst /s=..\..\Common\AccessQst.c >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wdis %TEMPDIR%\UsageStr  /l=%TEMPDIR%\UsageStr.lst  /s=..\..\Common\UsageStr.c  >>Build.log
if errorlevel 1 goto ERROR

echo Build Successful!!
goto DOSDONE

:ERROR
echo DOS %BUILD% Build Failed!!
set FAILED=TRUE

:DOSDONE
set INCLUDE=%OLDINCLUDE%
goto DONE

:WINRFAIL
echo Windows Release Build Failed!!
goto WINFAIL

:WINDFAIL
echo Windows Debug Build Failed!!

:WINFAIL
set FAILED=TRUE

:DONE
if "%1" == "" pause


