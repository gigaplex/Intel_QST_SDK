@echo off

rem remremremremremremremremremremremremremremremremremremremremremremrem rem
rem                                                                       rem
rem   File Name:     Services\BuildAll.bat                                rem
rem                                                                       rem
rem   Description:   Builds Windows exutables for  the  sample  support   rem
rem                  services  for the Intel(R) Quiet System Technology   rem
rem                  (QST) Subsystem.                                     rem
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

pushd QstDiskServ

echo Building QstDiskServ Executables . . .

echo *********************** QstDiskServ Release *********************** >..\Build.log
msdev "QstDiskServ.dsp" /make "QstDiskServ - Win32 Release" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

echo ************************ QstDiskServ Debug ************************ >>..\Build.log
msdev "QstDiskServ.dsp" /make "QstDiskServ - Win32 Debug" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

echo *********************** QstDiskTest Release *********************** >>..\Build.log
msdev "QstDiskTest.dsp" /make "QstDiskTest - Win32 Release" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

echo ************************ QstDiskTest Debug ************************ >>..\Build.log
msdev "QstDiskTest.dsp" /make "QstDiskTest - Win32 Debug" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

popd
pushd QstProtServ

echo Building QstProtServ Executables . . .

echo *********************** QstProtServ Release *********************** >>..\Build.log
msdev "QstProtServ.dsp" /make "QstProtServ - Win32 Release" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

echo ************************ QstProtServ Debug ************************ >>..\Build.log
msdev "QstProtServ.dsp" /make "QstProtServ - Win32 Debug" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

popd
pushd QstProxyServ

echo Building QstProxyServ Executables . . .

echo *********************** QstProxyServ Release ********************** >>..\Build.log
msdev "QstProxyServ.dsp" /make "QstProxyServ - Win32 Release" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

echo ************************ QstProxyServ Debug *********************** >>..\Build.log
msdev "QstProxyServ.dsp" /make "QstProxyServ - Win32 Debug" /rebuild >>..\Build.log
if errorlevel 1 goto FAILED

popd
echo Windows Services Built Successfully!
goto DONE

:FAILED

popd
echo Windows Services Build Failed!
set FAILED=TRUE

:DONE

if "%1" == "" pause


