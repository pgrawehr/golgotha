# Microsoft Developer Studio Project File - Name="install" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=install - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "install.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "install.mak" CFG="install - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "install - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "install - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "install - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "install___Win32_Release"
# PROP BASE Intermediate_Dir "install___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "install___Win32_Release"
# PROP Intermediate_Dir "install___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "install - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "install___Win32_Debug"
# PROP BASE Intermediate_Dir "install___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dinput.lib dsound.lib dxguid.lib wsock32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "install - Win32 Release"
# Name "install - Win32 Debug"
# Begin Group "Quellcodedateien"

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

SOURCE=.\golg__draw_context.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__f_tables.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_file.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_rand.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_tint.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__image_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__light.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__m_flow.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__make_tlist.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__menu.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__obj3d.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__path.cpp
# End Source File
# Begin Source File

SOURCE=.\gui__butbox.cpp
# End Source File
# Begin Source File

SOURCE=.\image__image.cpp
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

SOURCE=.\loaders__mp3__common.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__dct64.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__decode.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__decode_2to1.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__decode_4to1.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__equalizer.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__getlopt.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__layer1.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__layer2.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__layer3.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3__tabinit.cpp
# End Source File
# Begin Source File

SOURCE=.\loaders__mp3_load.cpp
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

SOURCE=.\main__win32_self_modify.cpp
# End Source File
# Begin Source File

SOURCE=.\main__win_main.cpp
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

SOURCE=.\memory__malloc.cpp
# End Source File
# Begin Source File

SOURCE=.\menu__menu.cpp
# End Source File
# Begin Source File

SOURCE=.\music__stream.cpp
# End Source File
# Begin Source File

SOURCE=.\palette_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\setup.cpp
# End Source File
# Begin Source File

SOURCE=.\sound__dsound__direct_sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound__sfx_id.cpp
# End Source File
# Begin Source File

SOURCE=.\sound__sound.cpp
# End Source File
# Begin Source File

SOURCE=.\status__status.cpp
# End Source File
# Begin Source File

SOURCE=.\string__string.cpp
# End Source File
# Begin Source File

SOURCE=.\threads__win32_threads.cpp
# End Source File
# Begin Source File

SOURCE=.\tick_count.cpp
# End Source File
# Begin Source File

SOURCE=.\time__timedev.cpp
# End Source File
# Begin Source File

SOURCE=.\video__display.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dd_cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx5.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx5_error.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx5_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx5_util.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx_cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__win32.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__win32_input.cpp
# End Source File
# Begin Source File

SOURCE=.\window__window.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
