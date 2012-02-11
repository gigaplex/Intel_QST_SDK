@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:     BuildAll.bat                                         rem
rem                                                                       rem
rem   Description:   Builds DOS and Windows versions  of  the  Intel(R)   rem
rem                  Quiet System Technology (QST) libraries and sample   rem
rem                  programs and services.                               rem
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

echo Building Libraries . . .

pushd src\Libraries
call BuildAll.bat nopause
copy /Y Build.log ..\..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo Building Programs . . .

pushd src\Programs
call BuildAll.bat nopause
copy /Y ..\..\Build.log+Build.log ..\..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

pushd src\Services
call BuildAll.bat nopause
copy /Y ..\..\Build.log+Build.log ..\..\Build.log
popd

if "%FAILED%"=="TRUE" goto FAILED

echo SDK Libraries and Executables Built Successfully!
goto DONE

:FAILED
echo SDK Libraries and Executables Build Failed!

:DONE
if "%1" == "" pause

