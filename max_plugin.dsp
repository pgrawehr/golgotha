# Microsoft Developer Studio Project File - Name="max_plugin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=max_plugin - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "max_plugin.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "max_plugin.mak" CFG="max_plugin - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "max_plugin - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "max_plugin - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "max_plugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAX_PLUGIN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAX_PLUGIN_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "max_plugin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAX_PLUGIN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "sdk_inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MAX_PLUGIN_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib mfc42d.lib nafxcwd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib max_plugin\core.lib max_plugin\util.lib max_plugin\geom.lib max_plugin\mesh.lib /nologo /dll /debug /machine:I386 /out:"plugin.dll" /pdbtype:sept
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "max_plugin - Win32 Release"
# Name "max_plugin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\app__app.cpp
# End Source File
# Begin Source File

SOURCE=.\area__rectlist.cpp
# End Source File
# Begin Source File

SOURCE=.\checksum__checksum.cpp
# End Source File
# Begin Source File

SOURCE=.\crkutil.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\device__device.cpp
# End Source File
# Begin Source File

SOURCE=.\dll__dll_man.cpp
# End Source File
# Begin Source File

SOURCE=.\error__error.cpp
# End Source File
# Begin Source File

SOURCE=.\file__file.cpp
# End Source File
# Begin Source File

SOURCE=.\font__font.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__obj3d.cpp
# End Source File
# Begin Source File

SOURCE=.\gui__butbox.cpp
# End Source File
# Begin Source File

SOURCE=.\image__image.cpp
# End Source File
# Begin Source File

SOURCE=.\import.cpp
# End Source File
# Begin Source File

SOURCE=.\inc__search.cpp
# End Source File
# Begin Source File

SOURCE=.\init__init.cpp
# End Source File
# Begin Source File

SOURCE=.\jj_error.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__lisp.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__bmp_load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__bmp_write.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__dir_load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__dir_save.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jcapistd.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jdapistd.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jdatasrc.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jerror.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctflt.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctfst.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctint.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctflt.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctfst.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctint.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctred.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jmemmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jmemnobs.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jquant1.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jquant2.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jutils.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__wrtarga.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg_load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__jpg_write.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__pcx_load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__tga_load.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__tga_write.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__wav_load.cpp
# End Source File
# Begin Source File

SOURCE=.\math__random.cpp
# End Source File
# Begin Source File

SOURCE=.\math__spline.cpp
# End Source File
# Begin Source File

SOURCE=.\math__transform.cpp
# End Source File
# Begin Source File

SOURCE=.\max_object.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__get_dir.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__maxcomm.cpp
# End Source File
# Begin Source File

SOURCE=.\memory__malloc.cpp
# End Source File
# Begin Source File

SOURCE=.\menu__menu.cpp
# End Source File
# Begin Source File

SOURCE=.\palette_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.def
# End Source File
# Begin Source File

SOURCE=.\string__string.cpp
# End Source File
# Begin Source File

SOURCE=.\threads__win32_threads.cpp
# End Source File
# Begin Source File

SOURCE=.\time__timedev.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\video__display.cpp
# End Source File
# Begin Source File

SOURCE=.\window__window.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;hh"
# Begin Source File

SOURCE=.\file\buf_file.h
# End Source File
# Begin Source File

SOURCE=.\gui\butbox.h
# End Source File
# Begin Source File

SOURCE=.\crkutil.h
# End Source File
# Begin Source File

SOURCE=.\crkutilr.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\plugin.rc
# End Source File
# End Group
# End Target
# End Project
