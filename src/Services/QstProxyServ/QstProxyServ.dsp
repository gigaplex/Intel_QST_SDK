# Microsoft Developer Studio Project File - Name="QstProxyServ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=QstProxyServ - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QstProxyServ.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QstProxyServ.mak" CFG="QstProxyServ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QstProxyServ - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "QstProxyServ - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QstProxyServ - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MD /W4 /GX /O2 /I "..\..\DLLs\QstInst" /I "..\..\Include" /I "..\..\Common" /I "..\..\Common\Windows" /I "..\..\Libraries\Common" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QST_SERVICE" /D "DYNAMIC_DLL_LOADING" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib setupapi.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "QstProxyServ - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MDd /W4 /Gm /GX /ZI /Od /I "..\..\DLLs\QstInst" /I "..\..\Include" /I "..\..\Common" /I "..\..\Common\Windows" /I "..\..\Libraries\Common" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QST_SERVICE" /D "DYNAMIC_DLL_LOADING" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib setupapi.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "QstProxyServ - Win32 Release"
# Name "QstProxyServ - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Libraries\Common\AccessQst.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Windows\CritSect.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Windows\GlobMem.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\LegTranslationFuncs.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\MilliTime.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Windows\QstCommD.c
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\QstDll.c
# End Source File
# Begin Source File

SOURCE=.\QstProxyServ.c
# End Source File
# Begin Source File

SOURCE=.\QstProxyServ.rc
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\RegAccess.c
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\Service.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Libraries\Common\AccessQst.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Build.h
# End Source File
# Begin Source File

SOURCE=.\CompVer.h
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\CritSect.h
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\GlobMem.h
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\LegTranslationFuncs.h
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\MilliTime.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\QstCfg.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\QstCfgLeg.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\QstCmd.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\QstCmdLeg.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\QstComm.h
# End Source File
# Begin Source File

SOURCE=..\..\Libraries\Common\QstDll.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\QstProxyComm.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\QstProxyInst.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\RegAccess.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Windows\Service.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\typedef.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\Common\Fan.ico
# End Source File
# End Group
# End Target
# End Project
