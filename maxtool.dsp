# Microsoft Developer Studio Project File - Name="maxtool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=maxtool - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "maxtool.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "maxtool.mak" CFG="maxtool - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "maxtool - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "maxtool - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "maxtool - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "maxtool___Win32_Release"
# PROP BASE Intermediate_Dir "maxtool___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "maxtool___Win32_Release"
# PROP Intermediate_Dir "maxtool___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "maxtool___Win32_Debug"
# PROP BASE Intermediate_Dir "maxtool___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "sdk_inc" /I "maxtool" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_WINDOWS" /D "_AFXDLL" /D "_AFXEXT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib comctl32.lib dxguid.lib dsound.lib ddraw.lib dinput.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "maxtool - Win32 Release"
# Name "maxtool - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\anim_dialog.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\app__app.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\area__rectlist.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\checksum__checksum.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\critical_graph.cpp
# End Source File
# Begin Source File

SOURCE=.\critical_map.cpp
# End Source File
# Begin Source File

SOURCE=.\device__device.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dll__dll_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\drag_select.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor__editor.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor__path_api.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__solvegraph_breadth.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__solvemap_breadth.cpp
# End Source File
# Begin Source File

SOURCE=.\error__error.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\file__file.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\font__font.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__border_frame.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__camera.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__cheat.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__controller.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__cwin_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__demo.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__draw_context.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__f_tables.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_file.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_object.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_rand.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_render.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_texture_id.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__g1_tint.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__global_id.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__human.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__image_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__input.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__level_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__li_interface.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__li_objref.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__light.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__load3d.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__m_flow.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__make_tlist.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_cell.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_collision.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_data.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_fast.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_light.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_lod.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_movi.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_path.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_save.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_vars.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_vert.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_view.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__map_vis.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__menu.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__obj3d.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__options.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__overhead.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__path.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__player.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__reference.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__remove_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__resources.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__saver.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__screen_shot.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__selection.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__sky.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__solvemap_astar.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__sound_man.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__statistics.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__team_api.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__tick_count.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__tile.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__vert_table.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\golg__visible.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui__butbox.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\image__image.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inc__search.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\init__init.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jj_error.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__lisp.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__bmp_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__bmp_write.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__dir_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__dir_save.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jcapistd.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jdapistd.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jdatasrc.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jerror.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctflt.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctfst.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jfdctint.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctflt.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctfst.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctint.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jidctred.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jmemmgr.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jmemnobs.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jquant1.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jquant2.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__jutils.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg__wrtarga.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__jpg_write.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__pcx_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__tga_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__tga_write.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\loaders__wav_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\m1_commands.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\m1_info.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\m1_test.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\m1_update.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main__win_main.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math__random.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math__spline.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math__transform.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\max_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\maxcomm.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\maxtool.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\maxtool__max_object.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\memory__malloc.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\menu__menu.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\music__stream.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\navigate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\network__tcpip.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__ai_builder.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__ai_jim.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bank.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__base_launcher.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bases.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__beam_weapon.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bolt.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bomb_truck.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bomber.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bridger.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__bullet.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__buster_rocket.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__carcass.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__chunk_explosion.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__cloud.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__crate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__damager.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__debris.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__def_object.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__defaults.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__dropped_bomb.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__eleccar.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__engineer.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__explode_model.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__explosion1.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__field_camera.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__fire.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__fire_angle.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__flak.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__guided_missile.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__helicopter.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__jet.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__lawfirm.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__light_o.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__map_piece.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__miscobjs.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__model_collide.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__model_draw.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__model_id.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__moneycrate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__moneyplane.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__old_ids.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__particle_emitter.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__path_object.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__peontank.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__popup_turret.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__repairer.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__rocket.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__rocktank.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__sfx_obj.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__shockwave.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__shrapnel.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__smoke_trail.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__stank.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__stank_factory.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__structure_death.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__super_mortar.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__supergun.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__takeover_pad.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__tank_buster.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__target.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__tower_electric.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__tower_missile.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__trike.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__turret.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\objs__vehic_sounds.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\palette_pal.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pan.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__dx5__r1_dx5.cpp
# End Source File
# Begin Source File

SOURCE=.\render__dx5__r1_dx5_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\render__gtext_load.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__mip.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__mip_average.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__r1_api.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__r1_clip.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__r1_font.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__r1_res.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__r1_win.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__tex_id.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\render__tmanage.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\saveas.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound__dsound__direct_sound.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound__sfx_id.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sound__sound.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\st_edit.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\status__status.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\string__string.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\threads__win32_threads.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\time__timedev.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\translate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\tupdate.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\video__display.cpp

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

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

!IF  "$(CFG)" == "maxtool - Win32 Release"

!ELSEIF  "$(CFG)" == "maxtool - Win32 Debug"

# ADD CPP /FR

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\sound\dsound\a3d.h
# End Source File
# Begin Source File

SOURCE=.\error\alert.h
# End Source File
# Begin Source File

SOURCE=.\math\angle.h
# End Source File
# Begin Source File

SOURCE=.\font\anti_prop.h
# End Source File
# Begin Source File

SOURCE=.\app\app.h
# End Source File
# Begin Source File

SOURCE=.\arch.h
# End Source File
# Begin Source File

SOURCE=.\memory\array.h
# End Source File
# Begin Source File

SOURCE=.\file\async.h
# End Source File
# Begin Source File

SOURCE=.\objs\bank.h
# End Source File
# Begin Source File

SOURCE=.\objs\base_launcher.h
# End Source File
# Begin Source File

SOURCE=.\objs\bases.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\BaseTsd.h
# End Source File
# Begin Source File

SOURCE=.\objs\beam_weapon.h
# End Source File
# Begin Source File

SOURCE=.\memory\binary_tree.h
# End Source File
# Begin Source File

SOURCE=.\memory\bmanage.h
# End Source File
# Begin Source File

SOURCE=.\loaders\bmp_write.h
# End Source File
# Begin Source File

SOURCE=.\objs\bolt.h
# End Source File
# Begin Source File

SOURCE=.\objs\bomber.h
# End Source File
# Begin Source File

SOURCE=.\border_frame.h
# End Source File
# Begin Source File

SOURCE=.\menu\boxmenu.h
# End Source File
# Begin Source File

SOURCE=.\file\buf_file.h
# End Source File
# Begin Source File

SOURCE=.\objs\bullet.h
# End Source File
# Begin Source File

SOURCE=.\objs\buster_rocket.h
# End Source File
# Begin Source File

SOURCE=.\gui\butbox.h
# End Source File
# Begin Source File

SOURCE=.\gui\button.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\objs\carcass.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\cderror.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\cdjpeg.h
# End Source File
# Begin Source File

SOURCE=.\checksum\checksum.h
# End Source File
# Begin Source File

SOURCE=.\objs\chunk_explosion.h
# End Source File
# Begin Source File

SOURCE=.\image\color.h
# End Source File
# Begin Source File

SOURCE=.\window\colorwin.h
# End Source File
# Begin Source File

SOURCE=.\editor\contedit.h
# End Source File
# Begin Source File

SOURCE=.\image\context.h
# End Source File
# Begin Source File

SOURCE=.\controller.h
# End Source File
# Begin Source File

SOURCE=.\objs\crate.h
# End Source File
# Begin Source File

SOURCE=.\gui\create_dialog.h
# End Source File
# Begin Source File

SOURCE=.\window\cursor.h
# End Source File
# Begin Source File

SOURCE=.\cwin_man.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\d_light.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\d_time.h
# End Source File
# Begin Source File

SOURCE=.\objs\damager.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dd_cursor.h
# End Source File
# Begin Source File

SOURCE=.\objs\debris.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\debug_win.h
# End Source File
# Begin Source File

SOURCE=.\gui\deco_win.h
# End Source File
# Begin Source File

SOURCE=.\objs\def_object.h
# End Source File
# Begin Source File

SOURCE=.\objs\defaults.h
# End Source File
# Begin Source File

SOURCE=.\demo.h
# End Source File
# Begin Source File

SOURCE=.\device\device.h
# End Source File
# Begin Source File

SOURCE=.\loaders\dir_load.h
# End Source File
# Begin Source File

SOURCE=.\loaders\dir_save.h
# End Source File
# Begin Source File

SOURCE=.\sound\dsound\direct_sound.h
# End Source File
# Begin Source File

SOURCE=.\video\display.h
# End Source File
# Begin Source File

SOURCE=.\gui\divider.h
# End Source File
# Begin Source File

SOURCE=.\dll\dll.h
# End Source File
# Begin Source File

SOURCE=.\dll_export.h
# End Source File
# Begin Source File

SOURCE=.\dll\dll_man.h
# End Source File
# Begin Source File

SOURCE=.\window\dragwin.h
# End Source File
# Begin Source File

SOURCE=.\draw_context.h
# End Source File
# Begin Source File

SOURCE=.\objs\dropped_bomb.h
# End Source File
# Begin Source File

SOURCE=.\sound\dsound\ds_error.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx5.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx5_error.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx5_mouse.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx5_util.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx_cursor.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\dxfile.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_camera.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_light.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_mode.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_object.h
# End Source File
# Begin Source File

SOURCE=.\editor\e_res.h
# End Source File
# Begin Source File

SOURCE=.\editor\e_state.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_tile.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\e_time.h
# End Source File
# Begin Source File

SOURCE=.\editor\edit_id.h
# End Source File
# Begin Source File

SOURCE=.\editor\editor.h
# End Source File
# Begin Source File

SOURCE=.\objs\eleccar.h
# End Source File
# Begin Source File

SOURCE=.\objs\engineer.h
# End Source File
# Begin Source File

SOURCE=.\device\event.h
# End Source File
# Begin Source File

SOURCE=.\objs\explode_model.h
# End Source File
# Begin Source File

SOURCE=.\objs\explosion1.h
# End Source File
# Begin Source File

SOURCE=.\f_tables.h
# End Source File
# Begin Source File

SOURCE=.\objs\field_camera.h
# End Source File
# Begin Source File

SOURCE=.\file\file.h
# End Source File
# Begin Source File

SOURCE=.\file\file_man.h
# End Source File
# Begin Source File

SOURCE=.\file\file_open.h
# End Source File
# Begin Source File

SOURCE=.\editor\commands\fill.h
# End Source File
# Begin Source File

SOURCE=.\objs\fire.h
# End Source File
# Begin Source File

SOURCE=.\objs\fire_angle.h
# End Source File
# Begin Source File

SOURCE=.\memory\fixed_array.h
# End Source File
# Begin Source File

SOURCE=.\objs\flak.h
# End Source File
# Begin Source File

SOURCE=.\flare.h
# End Source File
# Begin Source File

SOURCE=.\font\font.h
# End Source File
# Begin Source File

SOURCE=.\g1_limits.h
# End Source File
# Begin Source File

SOURCE=.\g1_object.h
# End Source File
# Begin Source File

SOURCE=.\g1_rand.h
# End Source File
# Begin Source File

SOURCE=.\g1_render.h
# End Source File
# Begin Source File

SOURCE=.\g1_speed.h
# End Source File
# Begin Source File

SOURCE=.\g1_texture_id.h
# End Source File
# Begin Source File

SOURCE=.\g1_tint.h
# End Source File
# Begin Source File

SOURCE=.\g1_vert.h
# End Source File
# Begin Source File

SOURCE=.\file\get_filename.h
# End Source File
# Begin Source File

SOURCE=.\global_id.h
# End Source File
# Begin Source File

SOURCE=.\gui\gradiant.h
# End Source File
# Begin Source File

SOURCE=.\memory\growarry.h
# End Source File
# Begin Source File

SOURCE=.\memory\growheap.h
# End Source File
# Begin Source File

SOURCE=.\render\gtext_load.h
# End Source File
# Begin Source File

SOURCE=.\time\gui_prof.h
# End Source File
# Begin Source File

SOURCE=.\status\gui_stat.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\Guiddef.h
# End Source File
# Begin Source File

SOURCE=.\objs\guided_missile.h
# End Source File
# Begin Source File

SOURCE=.\height_info.h
# End Source File
# Begin Source File

SOURCE=.\objs\helicopter.h
# End Source File
# Begin Source File

SOURCE=.\quantize\histogram.h
# End Source File
# Begin Source File

SOURCE=.\human.h
# End Source File
# Begin Source File

SOURCE=.\sound\dsound\ia3d.h
# End Source File
# Begin Source File

SOURCE=.\image\image.h
# End Source File
# Begin Source File

SOURCE=.\image\image16.h
# End Source File
# Begin Source File

SOURCE=.\image\image32.h
# End Source File
# Begin Source File

SOURCE=.\image\image8.h
# End Source File
# Begin Source File

SOURCE=.\menu\image_item.h
# End Source File
# Begin Source File

SOURCE=.\image_man.h
# End Source File
# Begin Source File

SOURCE=.\gui\image_win.h
# End Source File
# Begin Source File

SOURCE=.\init\init.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\isllist.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jdct.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jerror.h
# End Source File
# Begin Source File

SOURCE=.\objs\jet.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jinclude.h
# End Source File
# Begin Source File

SOURCE=.\jj_error.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg_load.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg_write.h
# End Source File
# Begin Source File

SOURCE=.\loaders\jpg\jversion.h
# End Source File
# Begin Source File

SOURCE=.\device\kernel.h
# End Source File
# Begin Source File

SOURCE=.\menu\key_item.h
# End Source File
# Begin Source File

SOURCE=.\device\key_man.h
# End Source File
# Begin Source File

SOURCE=.\device\keys.h
# End Source File
# Begin Source File

SOURCE=.\memory\lalloc.h
# End Source File
# Begin Source File

SOURCE=.\level_load.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_alloc.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_class.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_dialog.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_error.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_init.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_load.h
# End Source File
# Begin Source File

SOURCE=.\li_objref.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_optr.h
# End Source File
# Begin Source File

SOURCE=.\gui\li_pull_menu.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_types.h
# End Source File
# Begin Source File

SOURCE=.\lisp\li_vect.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\objs\light_o.h
# End Source File
# Begin Source File

SOURCE=.\lisp\lisp.h
# End Source File
# Begin Source File

SOURCE=.\gui\list_box.h
# End Source File
# Begin Source File

SOURCE=.\loaders\load.h
# End Source File
# Begin Source File

SOURCE=.\load3d.h
# End Source File
# Begin Source File

SOURCE=.\network\login.h
# End Source File
# Begin Source File

SOURCE=.\m_flow.h
# End Source File
# Begin Source File

SOURCE=.\main\main.h
# End Source File
# Begin Source File

SOURCE=.\make_tlist.h
# End Source File
# Begin Source File

SOURCE=.\map.h
# End Source File
# Begin Source File

SOURCE=.\map_cell.h
# End Source File
# Begin Source File

SOURCE=.\map_data.h
# End Source File
# Begin Source File

SOURCE=.\map_light.h
# End Source File
# Begin Source File

SOURCE=.\map_man.h
# End Source File
# Begin Source File

SOURCE=.\objs\map_piece.h
# End Source File
# Begin Source File

SOURCE=.\map_vars.h
# End Source File
# Begin Source File

SOURCE=.\map_vert.h
# End Source File
# Begin Source File

SOURCE=.\map_view.h
# End Source File
# Begin Source File

SOURCE=.\math\matrix.h
# End Source File
# Begin Source File

SOURCE=.\quantize\median.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\menu\menu.h
# End Source File
# Begin Source File

SOURCE=.\menu\menuitem.h
# End Source File
# Begin Source File

SOURCE=.\mess_id.h
# End Source File
# Begin Source File

SOURCE=.\render\mip.h
# End Source File
# Begin Source File

SOURCE=.\render\mip_average.h
# End Source File
# Begin Source File

SOURCE=.\objs\miscobjs.h
# End Source File
# Begin Source File

SOURCE=.\objs\model_collide.h
# End Source File
# Begin Source File

SOURCE=.\objs\model_draw.h
# End Source File
# Begin Source File

SOURCE=.\objs\model_id.h
# End Source File
# Begin Source File

SOURCE=.\objs\moneycrate.h
# End Source File
# Begin Source File

SOURCE=.\objs\moneyplane.h
# End Source File
# Begin Source File

SOURCE=.\network\net_addr.h
# End Source File
# Begin Source File

SOURCE=.\network\net_find.h
# End Source File
# Begin Source File

SOURCE=.\network\net_prot.h
# End Source File
# Begin Source File

SOURCE=.\network\net_sock.h
# End Source File
# Begin Source File

SOURCE=.\math\num_type.h
# End Source File
# Begin Source File

SOURCE=.\obj3d.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\obj_win.h
# End Source File
# Begin Source File

SOURCE=.\object_definer.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\object_picker.h
# End Source File
# Begin Source File

SOURCE=.\objs\old_ids.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\palette\pal.h
# End Source File
# Begin Source File

SOURCE=.\objs\particle_emitter.h
# End Source File
# Begin Source File

SOURCE=.\path.h
# End Source File
# Begin Source File

SOURCE=.\objs\path_object.h
# End Source File
# Begin Source File

SOURCE=.\objs\peontank.h
# End Source File
# Begin Source File

SOURCE=.\math\pi.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\pick_win.h
# End Source File
# Begin Source File

SOURCE=.\font\plain.h
# End Source File
# Begin Source File

SOURCE=.\playback.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\player_type.h
# End Source File
# Begin Source File

SOURCE=.\editor\pmenu.h
# End Source File
# Begin Source File

SOURCE=.\math\point.h
# End Source File
# Begin Source File

SOURCE=.\poly\poly.h
# End Source File
# Begin Source File

SOURCE=.\poly\polyclip.h
# End Source File
# Begin Source File

SOURCE=.\poly\polydraw.h
# End Source File
# Begin Source File

SOURCE=.\objs\popup_turret.h
# End Source File
# Begin Source File

SOURCE=.\device\processor.h
# End Source File
# Begin Source File

SOURCE=.\time\profile.h
# End Source File
# Begin Source File

SOURCE=.\time\profile_stack.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\PropIdl.h
# End Source File
# Begin Source File

SOURCE=.\menu\pull.h
# End Source File
# Begin Source File

SOURCE=.\memory\que.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_api.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_clip.h
# End Source File
# Begin Source File

SOURCE=.\render\dx5\r1_dx5.h
# End Source File
# Begin Source File

SOURCE=.\render\dx5\r1_dx5_texture.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_font.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_res.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_vert.h
# End Source File
# Begin Source File

SOURCE=.\render\r1_win.h
# End Source File
# Begin Source File

SOURCE=.\file\ram_file.h
# End Source File
# Begin Source File

SOURCE=.\file\ram_file_man.h
# End Source File
# Begin Source File

SOURCE=.\math\random.h
# End Source File
# Begin Source File

SOURCE=.\range.h
# End Source File
# Begin Source File

SOURCE=.\area\rectlist.h
# End Source File
# Begin Source File

SOURCE=.\reference.h
# End Source File
# Begin Source File

SOURCE=.\app\registry.h
# End Source File
# Begin Source File

SOURCE=.\remove_man.h
# End Source File
# Begin Source File

SOURCE=.\objs\repairer.h
# End Source File
# Begin Source File

SOURCE=.\resources.h
# End Source File
# Begin Source File

SOURCE=.\compress\rle.h
# End Source File
# Begin Source File

SOURCE=.\objs\rocket.h
# End Source File
# Begin Source File

SOURCE=.\objs\rocktank.h
# End Source File
# Begin Source File

SOURCE=.\rotation.h
# End Source File
# Begin Source File

SOURCE=.\saver.h
# End Source File
# Begin Source File

SOURCE=.\saver_id.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\scene.h
# End Source File
# Begin Source File

SOURCE=.\objs\scream.h
# End Source File
# Begin Source File

SOURCE=.\gui\scroll_bar.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\scroll_picker.h
# End Source File
# Begin Source File

SOURCE=.\search.h
# End Source File
# Begin Source File

SOURCE=.\selection.h
# End Source File
# Begin Source File

SOURCE=.\gui\seperate.h
# End Source File
# Begin Source File

SOURCE=.\sfx_id.h
# End Source File
# Begin Source File

SOURCE=.\sound\sfx_id.h
# End Source File
# Begin Source File

SOURCE=.\objs\sfx_obj.h
# End Source File
# Begin Source File

SOURCE=.\objs\shockwave.h
# End Source File
# Begin Source File

SOURCE=.\objs\shrapnel.h
# End Source File
# Begin Source File

SOURCE=.\sky.h
# End Source File
# Begin Source File

SOURCE=.\gui\slider.h
# End Source File
# Begin Source File

SOURCE=.\objs\smoke_trail.h
# End Source File
# Begin Source File

SOURCE=.\gui\smp_dial.h
# End Source File
# Begin Source File

SOURCE=.\solvemap_astar.h
# End Source File
# Begin Source File

SOURCE=.\sound\sound.h
# End Source File
# Begin Source File

SOURCE=.\sound_man.h
# End Source File
# Begin Source File

SOURCE=.\sound\sound_types.h
# End Source File
# Begin Source File

SOURCE=.\math\spline.h
# End Source File
# Begin Source File

SOURCE=.\objs\stank.h
# End Source File
# Begin Source File

SOURCE=.\file\static_file.h
# End Source File
# Begin Source File

SOURCE=.\statistics.h
# End Source File
# Begin Source File

SOURCE=.\status\status.h
# End Source File
# Begin Source File

SOURCE=.\string\str_checksum.h
# End Source File
# Begin Source File

SOURCE=.\music\stream.h
# End Source File
# Begin Source File

SOURCE=.\objs\structure_death.h
# End Source File
# Begin Source File

SOURCE=.\window\style.h
# End Source File
# Begin Source File

SOURCE=.\file\sub_section.h
# End Source File
# Begin Source File

SOURCE=.\objs\super_mortar.h
# End Source File
# Begin Source File

SOURCE=.\objs\supergun.h
# End Source File
# Begin Source File

SOURCE=.\gui\tab_bar.h
# End Source File
# Begin Source File

SOURCE=.\objs\tank_buster.h
# End Source File
# Begin Source File

SOURCE=.\objs\target.h
# End Source File
# Begin Source File

SOURCE=.\team_api.h
# End Source File
# Begin Source File

SOURCE=.\render\tex_cache.h
# End Source File
# Begin Source File

SOURCE=.\render\tex_heap.h
# End Source File
# Begin Source File

SOURCE=.\render\tex_id.h
# End Source File
# Begin Source File

SOURCE=.\render\tex_no_heap.h
# End Source File
# Begin Source File

SOURCE=.\gui\text.h
# End Source File
# Begin Source File

SOURCE=.\gui\text_input.h
# End Source File
# Begin Source File

SOURCE=.\gui\text_scroll.h
# End Source File
# Begin Source File

SOURCE=.\menu\textitem.h
# End Source File
# Begin Source File

SOURCE=.\loaders\tga_write.h
# End Source File
# Begin Source File

SOURCE=.\threads\threads.h
# End Source File
# Begin Source File

SOURCE=.\tick_count.h
# End Source File
# Begin Source File

SOURCE=.\tile.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\tile_picker.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\tile_win.h
# End Source File
# Begin Source File

SOURCE=.\time\timedev.h
# End Source File
# Begin Source File

SOURCE=.\render\tmanage.h
# End Source File
# Begin Source File

SOURCE=.\render\tnode.h
# End Source File
# Begin Source File

SOURCE=.\math\transform.h
# End Source File
# Begin Source File

SOURCE=.\math\trig.h
# End Source File
# Begin Source File

SOURCE=.\objs\trike.h
# End Source File
# Begin Source File

SOURCE=.\objs\turret.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\Tvout.h
# End Source File
# Begin Source File

SOURCE=.\math\vector.h
# End Source File
# Begin Source File

SOURCE=.\objs\vehic_sounds.h
# End Source File
# Begin Source File

SOURCE=.\objs\verybiggun.h
# End Source File
# Begin Source File

SOURCE=.\visible.h
# End Source File
# Begin Source File

SOURCE=.\loaders\wav_load.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\win32_input.h
# End Source File
# Begin Source File

SOURCE=.\window\win_evt.h
# End Source File
# Begin Source File

SOURCE=.\main\win_main.h
# End Source File
# Begin Source File

SOURCE=.\window\window.h
# End Source File
# Begin Source File

SOURCE=..\Programme\Ms_sdk\Include\WinEFS.h
# End Source File
# Begin Source File

SOURCE=.\window\wmanager.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\maxtool.ico
# End Source File
# Begin Source File

SOURCE=.\maxtool.rc
# End Source File
# End Group
# End Target
# End Project
