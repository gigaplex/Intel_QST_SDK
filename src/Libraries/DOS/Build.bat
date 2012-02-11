@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:     Libraries\DOS\Build.bat                              rem
rem                                                                       rem
rem   Description:   Builds DOS versions of the Intel(R)  Quiet  System   rem
rem                  Technology  (QST) Instrumentation & Communications   rem
rem                  libraries.                                           rem
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

set OLDINCLUDE=%INCLUDE%

rem Try the two standard places where Open Watcom could be installed...
if exist C:\watcom\setvars.bat call C:\watcom\setvars.bat
if exist C:\progra~1\watcom\setvars.bat call C:\progra~1\watcom\setvars.bat

if exist Build.log del /f Build.log

set BUILDDIR=Release
set BUILDOPTS=/ox /d0 /mf /wx /hw /fp6 /6r /bt=dos /i=..\..\Include /i=..\..\Common /i=..\Common

goto DOBUILD

:DEBUG

set BUILDDIR=Debug
set BUILDOPTS=/od /d2 /dTRACE /mf /wx /hw /fp6 /6r /bt=dos /i=..\..\Include /i=..\..\Common /i=..\Common

:DOBUILD

if not exist %BUILDDIR% md %BUILDDIR%

echo Building %BUILDDIR%\QstComm6r.lib . . .
echo *********************************************** QstComm %BUILDDIR% >>Build.log

%WATCOM%\binnt\wcc386 QstComm.c /fo=%BUILDDIR%\QstComm.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 ..\Common\LegTranslationFuncs.c /fo=%BUILDDIR%\LegTranslationFuncs.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 heci.c /fo=%BUILDDIR%\heci.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wlib /n %BUILDDIR%\QstComm6r.lib +%BUILDDIR%\QstComm.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstComm6r.lib +%BUILDDIR%\LegTranslationFuncs.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstComm6r.lib +%BUILDDIR%\heci.obj >>Build.log
if errorlevel 1 goto ERROR

echo Building %BUILDDIR%\QstInst6r.lib . . .
echo *********************************************** QstInst %BUILDDIR% >>Build.log

%WATCOM%\binnt\wcc386 ..\Common\AccessQst.c /fo=%BUILDDIR%\AccessQst.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 ..\Common\QstDll.c /fo=%BUILDDIR%\QstDll.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 ..\Common\QstInst.c /fo=%BUILDDIR%\QstInst.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 ..\Common\Millitime.c /fo=%BUILDDIR%\Millitime.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 CritSect.c /fo=%BUILDDIR%\CritSect.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wcc386 GlobMem.c /fo=%BUILDDIR%\GlobMem.obj %BUILDOPTS% >>Build.log
if errorlevel 1 goto ERROR

%WATCOM%\binnt\wlib /n %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\AccessQst.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\QstDll.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\QstInst.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\Millitime.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\CritSect.obj >>Build.log
if errorlevel 1 goto ERROR
%WATCOM%\binnt\wlib /b %BUILDDIR%\QstInst6r.lib +%BUILDDIR%\GlobMem.obj >>Build.log
if errorlevel 1 goto ERROR

if "%BUILDDIR%" == "Release" goto DEBUG

rem echo Building Disassembly Listings . . .
rem echo *********************************************** Disassembly Listings >>Build.log

rem %WATCOM%\binnt\wdis %BUILDDIR%\QstComm /l=%BUILDDIR%\QstComm.lst /s=QstComm.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\LegTranslationFuncs /l=%BUILDDIR%\LegTranslationFuncs.lst /s=..\Common\LegTranslationFuncs.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\heci /l=%BUILDDIR%\heci.lst /s=heci.c >>Build.log
rem if errorlevel 1 goto ERROR

rem %WATCOM%\binnt\wdis %BUILDDIR%\AccessQst /l=%BUILDDIR%\AccessQst.lst /s=..\Common\AccessQst.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\QstDll /l=%BUILDDIR%\QstDll.lst /s=..\Common\QstDll.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\QstInst /l=%BUILDDIR%\QstInst.lst /s=..\Common\QstInst.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\Millitime /l=%BUILDDIR%\Millitime.lst /s=..\Common\Millitime.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\CritSect /l=%BUILDDIR%\CritSect.lst /s=CritSect.c >>Build.log
rem if errorlevel 1 goto ERROR
rem %WATCOM%\binnt\wdis %BUILDDIR%\GlobMem /l=%BUILDDIR%\GlobMem.lst /s=GlobMem.c >>Build.log
rem if errorlevel 1 goto ERROR

echo DOS Libraries Built Successfully!
goto DONE

:ERROR

echo DOS Libraries Build Failed!
set FAILED=TRUE

:DONE

set INCLUDE=%OLDINCLUDE%
if "%1" == "" pause

