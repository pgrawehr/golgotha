# Microsoft Developer Studio Project File - Name="Golgotha" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Golgotha - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Golgotha.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Golgotha.mak" CFG="Golgotha - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Golgotha - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Golgotha - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Golgotha - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "ReleaseVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D "_AFXEXT" /YX"pch.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib dinput.lib dsound.lib dxguid.lib wsock32.lib comctl32.lib winmm.lib version.lib d3d9.lib dxerr9.lib /nologo /version:1.1 /stack:0x27018e,0x100000 /subsystem:windows /profile /map:"Golgotha_VC6Rel.map" /debug /machine:I386 /out:"Golgotha_VC6Rel.exe"

!ELSEIF  "$(CFG)" == "Golgotha - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ""
# PROP Intermediate_Dir "DebugVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /D "_AFXEXT" /FR /YX"pch.h" /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x409 /v /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Golgotha_VC6.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dinput.lib dsound.lib dxguid.lib wsock32.lib comctl32.lib winmm.lib version.lib dxerr9.lib d3d9.lib /nologo /version:1.1 /stack:0x3d0900,0x200000 /subsystem:windows /map:"Golgotha_VC6.map" /debug /machine:I386 /out:"d:\Golgotha\Golgotha_VC6.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Golgotha - Win32 Release"
# Name "Golgotha - Win32 Debug"
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

SOURCE=.\device__device.cpp
# End Source File
# Begin Source File

SOURCE=.\dll__dll_man.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__critical_graph.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__critical_map.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__e_ai.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__editor.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__lisp_interaction.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__path_api.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__solvegraph_breadth.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__solvemap_astar2.cpp
# End Source File
# Begin Source File

SOURCE=.\editor__solvemap_breadth.cpp
# End Source File
# Begin Source File

SOURCE=.\error__error.cpp
# End Source File
# Begin Source File

SOURCE=.\file__dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\file__file.cpp
# End Source File
# Begin Source File

SOURCE=.\font__font.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__border_frame.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__camera.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__cheat.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__controller.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__cwin_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__demo.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__draw_context.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__f_tables.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__fli_load.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_file.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_object.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_rand.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_render.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_texture_id.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__g1_tint.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__global_id.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__human.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__image_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__input.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__level_load.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__li_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__li_objref.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__light.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__load3d.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__m_flow.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__main.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__make_tlist.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_cell.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_collision.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_data.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_light.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_lod.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_movi.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_save.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_vars.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_vert.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_view.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__map_vis.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__menu.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__obj3d.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__octree.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__options.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__overhead.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__path.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__player.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__reference.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__remove_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__resources.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__saver.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__screen_shot.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__selection.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__sky.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__solvemap_astar.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__sound_man.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__team_api.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__tick_count.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__tile.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__vert_table.cpp
# End Source File
# Begin Source File

SOURCE=.\golg__visible.cpp
# End Source File
# Begin Source File

SOURCE=.\gui__browse_tree.cpp
# End Source File
# Begin Source File

SOURCE=.\gui__butbox.cpp
# End Source File
# Begin Source File

SOURCE=.\gui__list_pick.cpp
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

SOURCE=.\lisp__file.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__functions.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__lisp.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__math.cpp
# End Source File
# Begin Source File

SOURCE=.\lisp__setup.cpp
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

SOURCE=.\maxtool__anim_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__animate.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__drag_select.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__draw_modes.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__m1_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__m1_info.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__m1_test.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__max_load.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__max_object.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__maxcomm.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__maxtool.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__mount_win.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__navigate.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__pan.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__render.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__saveas.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__st_edit.cpp
# End Source File
# Begin Source File

SOURCE=.\maxtool__translate.cpp
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

SOURCE=.\net__startup.cpp
# End Source File
# Begin Source File

SOURCE=.\network__tcpip.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__ai_builder.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__ai_jim.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__ai_joe.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__armor.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bank.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__base_launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bases.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__beam_weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bird.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bolt.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bomb_truck.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bomber.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bridger.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__bullet.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__buster_rocket.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__car.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__carcass.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__chunk_explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__cloud.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__cobra_tank.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__controllers.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__convoy.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__crate.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__damager.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__day_night.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__debris.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__def_object.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__defaults.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__dropped_bomb.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__eleccar.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__engineer.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__explode_model.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__explosion1.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__field_camera.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__fire.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__fire_angle.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__flak.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__goal.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__guided_missile.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__gun_port.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__helicopter.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__jet.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__lawfirm.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__light_o.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__map_piece.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__miracle.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__miscobjs.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__model_collide.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__model_draw.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__model_id.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__moneycrate.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__moneyplane.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__old_ids.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__particle_emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__path_object.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__peontank.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__popup_turret.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__repairer.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__rocket.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__rocktank.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__sfx_obj.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__shockwave.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__shrapnel.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__smoke_trail.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__sprite_object.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__stank.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__stank_factory.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__structure_death.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__super_mortar.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__supergun.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__takeover_pad.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__tank_buster.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__target.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__tower_electric.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__tower_missile.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__trike.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__turret.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__vehic_sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\objs__verybiggun.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionExtras.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionGetDXVer.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionMFCInit.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\Optionsdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Optionsheet.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionSound.cpp
# End Source File
# Begin Source File

SOURCE=.\palette_pal.cpp
# End Source File
# Begin Source File

SOURCE=.\quantize__dither_quantize.cpp
# End Source File
# Begin Source File

SOURCE=.\quantize__histogram.cpp
# End Source File
# Begin Source File

SOURCE=.\quantize__median.cpp
# End Source File
# Begin Source File

SOURCE=.\render__dx5__r1_dx5.cpp
# End Source File
# Begin Source File

SOURCE=.\render__dx5__r1_dx5_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\render__dx9__render.cpp
# End Source File
# Begin Source File

SOURCE=.\render__dx9__texture.cpp
# End Source File
# Begin Source File

SOURCE=.\render__gtext_load.cpp
# End Source File
# Begin Source File

SOURCE=.\render__mip.cpp
# End Source File
# Begin Source File

SOURCE=.\render__mip_average.cpp
# End Source File
# Begin Source File

SOURCE=.\render__r1_api.cpp
# End Source File
# Begin Source File

SOURCE=.\render__r1_clip.cpp
# End Source File
# Begin Source File

SOURCE=.\render__r1_font.cpp
# End Source File
# Begin Source File

SOURCE=.\render__r1_res.cpp
# End Source File
# Begin Source File

SOURCE=.\render__r1_win.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__draw_line.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__mappers.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__r1_software.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__r1_software_globals.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__r1_software_texture.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__rasterize_affine.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__rasterize_perspective.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__span_buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__tint_manage.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__tri_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\render__software__win32_specific.cpp
# End Source File
# Begin Source File

SOURCE=.\render__tex_id.cpp
# End Source File
# Begin Source File

SOURCE=.\render__tmanage.cpp
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

SOURCE=.\time__timedev.cpp
# End Source File
# Begin Source File

SOURCE=.\transport__loader.cpp
# End Source File
# Begin Source File

SOURCE=.\transport__randomize.cpp
# End Source File
# Begin Source File

SOURCE=.\transport__transport.cpp
# End Source File
# Begin Source File

SOURCE=.\video__display.cpp
# End Source File
# Begin Source File

SOURCE=.\video__movie_engine.cpp
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

SOURCE=.\video__win32__dx9.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx9_mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\video__win32__dx9_util.cpp
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
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;hh"
# Begin Source File

SOURCE=.\lisp\abuse.h
# End Source File
# Begin Source File

SOURCE=.\error\alert.h
# End Source File
# Begin Source File

SOURCE=.\render\software\amd3d\amd3d.h
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

SOURCE=.\loaders\mp3\audio.h
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

SOURCE=.\objs\beam_weapon.h
# End Source File
# Begin Source File

SOURCE=.\memory\binary_tree.h
# End Source File
# Begin Source File

SOURCE=.\memory\bitarray.h
# End Source File
# Begin Source File

SOURCE=.\memory\bitarray2d.h
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

SOURCE=.\gui\browse_tree.h
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

SOURCE=.\objs\car.h
# End Source File
# Begin Source File

SOURCE=.\objs\carcass.h
# End Source File
# Begin Source File

SOURCE=.\app\cdatafile.h
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

SOURCE=.\net\client.h
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

SOURCE=.\objs\convoy.h
# End Source File
# Begin Source File

SOURCE=.\objs\crate.h
# End Source File
# Begin Source File

SOURCE=.\gui\create_dialog.h
# End Source File
# Begin Source File

SOURCE=.\critical_graph.h
# End Source File
# Begin Source File

SOURCE=.\critical_map.h
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

SOURCE=.\math\d_point.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\d_time.h
# End Source File
# Begin Source File

SOURCE=.\math\d_transform.h
# End Source File
# Begin Source File

SOURCE=.\math\d_vector.h
# End Source File
# Begin Source File

SOURCE=.\objs\damager.h
# End Source File
# Begin Source File

SOURCE=.\objs\day_night.h
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

SOURCE=.\quantize\dither_quantize.h
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

SOURCE=.\video\win32\dx9_util.h
# End Source File
# Begin Source File

SOURCE=.\video\win32\dx_cursor.h
# End Source File
# Begin Source File

SOURCE=.\dxfile.h
# End Source File
# Begin Source File

SOURCE=.\memory\dynque.h
# End Source File
# Begin Source File

SOURCE=.\editor\mode\e_ai.h
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

SOURCE=.\g1_menu.h
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

SOURCE=.\loaders\mp3\getlopt.h
# End Source File
# Begin Source File

SOURCE=.\global_id.h
# End Source File
# Begin Source File

SOURCE=.\objs\goal.h
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

SOURCE=.\objs\guided_missile.h
# End Source File
# Begin Source File

SOURCE=.\memory\hashtable.h
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

SOURCE=.\loaders\mp3\huffman.h
# End Source File
# Begin Source File

SOURCE=.\human.h
# End Source File
# Begin Source File

SOURCE=.\sound\dsound\ia3d.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\id.h
# End Source File
# Begin Source File

SOURCE=.\image\image.h
# End Source File
# Begin Source File

SOURCE=.\image\image16.h
# End Source File
# Begin Source File

SOURCE=.\image\image24.h
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

SOURCE=.\render\software\inline_fpu.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\install.h
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

SOURCE=.\lisp\li_all.h
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

SOURCE=.\lisp\li_file.h
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

SOURCE=.\editor\lisp_interaction.h
# End Source File
# Begin Source File

SOURCE=.\gui\list_box.h
# End Source File
# Begin Source File

SOURCE=.\gui\list_pick.h
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

SOURCE=.\maxtool\m1_info.h
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

SOURCE=.\memory\malloc.h
# End Source File
# Begin Source File

SOURCE=.\map.h
# End Source File
# Begin Source File

SOURCE=.\map_cell.h
# End Source File
# Begin Source File

SOURCE=.\map_collision.h
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

SOURCE=.\map_singleton.h
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

SOURCE=.\render\software\mappers.h
# End Source File
# Begin Source File

SOURCE=.\math\matrix.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\max_load.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\max_object.h
# End Source File
# Begin Source File

SOURCE=.\maxcomm.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\maxcomm.h
# End Source File
# Begin Source File

SOURCE=.\quantize\median.h
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

SOURCE=.\objs\miracle.h
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

SOURCE=.\video\movie_engine.h
# End Source File
# Begin Source File

SOURCE=.\loaders\mp3_load.h
# End Source File
# Begin Source File

SOURCE=.\loaders\mp3\mpg123.h
# End Source File
# Begin Source File

SOURCE=.\loaders\mp3\mpversion.h
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

SOURCE=.\memory\new.h
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

SOURCE=.\octree.h
# End Source File
# Begin Source File

SOURCE=.\objs\old_ids.h
# End Source File
# Begin Source File

SOURCE=.\OptionExtras.h
# End Source File
# Begin Source File

SOURCE=.\OptionInfo.h
# End Source File
# Begin Source File

SOURCE=.\Optionmfcinit.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\Optionsdialog.h
# End Source File
# Begin Source File

SOURCE=.\Optionsheet.h
# End Source File
# Begin Source File

SOURCE=.\OptionSound.h
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

SOURCE=.\path_api.h
# End Source File
# Begin Source File

SOURCE=.\objs\path_object.h
# End Source File
# Begin Source File

SOURCE=.\editor\dialogs\path_win.h
# End Source File
# Begin Source File

SOURCE=.\pch.h
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

SOURCE=.\pragma.h
# End Source File
# Begin Source File

SOURCE=.\device\processor.h
# End Source File
# Begin Source File

SOURCE=.\Processor.h
# End Source File
# Begin Source File

SOURCE=.\time\profile.h
# End Source File
# Begin Source File

SOURCE=.\time\profile_stack.h
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

SOURCE=.\render\software\r1_software.h
# End Source File
# Begin Source File

SOURCE=.\render\software\r1_software_globals.h
# End Source File
# Begin Source File

SOURCE=.\render\software\r1_software_texture.h
# End Source File
# Begin Source File

SOURCE=.\render\software\r1_software_types.h
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

SOURCE=.\transport\randomize.h
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

SOURCE=.\render\dx9\render.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\render2.h
# End Source File
# Begin Source File

SOURCE=.\objs\repairer.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resources.h
# End Source File
# Begin Source File

SOURCE=.\compress\rle.h
# End Source File
# Begin Source File

SOURCE=.\objs\road_object.h
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

SOURCE=.\net\server.h
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

SOURCE=.\solvegraph.h
# End Source File
# Begin Source File

SOURCE=.\solvegraph_breadth.h
# End Source File
# Begin Source File

SOURCE=.\solvemap.h
# End Source File
# Begin Source File

SOURCE=.\solvemap_astar.h
# End Source File
# Begin Source File

SOURCE=.\solvemap_astar2.h
# End Source File
# Begin Source File

SOURCE=.\solvemap_breadth.h
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

SOURCE=.\render\software\span_buffer.h
# End Source File
# Begin Source File

SOURCE=.\math\spline.h
# End Source File
# Begin Source File

SOURCE=.\objs\sprite.h
# End Source File
# Begin Source File

SOURCE=.\objs\sprite_id.h
# End Source File
# Begin Source File

SOURCE=.\objs\sprite_object.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\st_edit.h
# End Source File
# Begin Source File

SOURCE=.\memory\stack.h
# End Source File
# Begin Source File

SOURCE=.\objs\stank.h
# End Source File
# Begin Source File

SOURCE=.\net\startup.h
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

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\string\str_checksum.h
# End Source File
# Begin Source File

SOURCE=.\music\stream.h
# End Source File
# Begin Source File

SOURCE=.\string\string.h
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

SOURCE=.\loaders\mp3\tables.h
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

SOURCE=.\render\dx9\texture.h
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

SOURCE=.\time\time.h
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

SOURCE=.\transport\transport.h
# End Source File
# Begin Source File

SOURCE=.\math\trig.h
# End Source File
# Begin Source File

SOURCE=.\objs\trigger.h
# End Source File
# Begin Source File

SOURCE=.\objs\trike.h
# End Source File
# Begin Source File

SOURCE=.\maxtool\tupdate.h
# End Source File
# Begin Source File

SOURCE=.\objs\turret.h
# End Source File
# Begin Source File

SOURCE=.\math\vector.h
# End Source File
# Begin Source File

SOURCE=.\objs\vehic_sounds.h
# End Source File
# Begin Source File

SOURCE=.\version.h
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

SOURCE=.\render\software\win32_specific.h
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

SOURCE=.\window\wmanager.h
# End Source File
# Begin Source File

SOURCE=.\loaders\mp3\xfermem.h
# End Source File
# End Group
# Begin Group "Data Files"

# PROP Default_Filter "res;scm"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource\3dcheck.bmp
# End Source File
# Begin Source File

SOURCE=.\resource\95check.bmp
# End Source File
# Begin Source File

SOURCE=.\resource\cur00001.cur
# End Source File
# Begin Source File

SOURCE=.\resource\cur00002.cur
# End Source File
# Begin Source File

SOURCE=.\resource\error.ico
# End Source File
# Begin Source File

SOURCE=.\golgotha.rc
# End Source File
# Begin Source File

SOURCE=.\resource\help.cur
# End Source File
# Begin Source File

SOURCE=.\resource\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\resource\ico102.ico
# End Source File
# Begin Source File

SOURCE=.\resource\mfctest.rc2
# End Source File
# Begin Source File

SOURCE=.\resource\minifwnd.bmp
# End Source File
# Begin Source File

SOURCE=.\resource\modelfil.ico
# End Source File
# Begin Source File

SOURCE=.\resource\move4way.cur
# End Source File
# Begin Source File

SOURCE=.\resource\nodrop.cur
# End Source File
# Begin Source File

SOURCE=.\resource\ntcheck.bmp
# End Source File
# Begin Source File

SOURCE=.\resource\pointer.cur
# End Source File
# Begin Source File

SOURCE=.\resource\sarrows.cur
# End Source File
# Begin Source File

SOURCE=.\resource\splith.cur
# End Source File
# Begin Source File

SOURCE=.\resource\splitv.cur
# End Source File
# Begin Source File

SOURCE=.\resource\trck4way.cur
# End Source File
# Begin Source File

SOURCE=.\resource\trcknesw.cur
# End Source File
# Begin Source File

SOURCE=.\resource\trckns.cur
# End Source File
# Begin Source File

SOURCE=.\resource\trcknwse.cur
# End Source File
# Begin Source File

SOURCE=.\resource\trckwe.cur
# End Source File
# End Group
# Begin Source File

SOURCE=.\Hints.txt
# End Source File
# Begin Source File

SOURCE=.\Manual.doc
# End Source File
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
