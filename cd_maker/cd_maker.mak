# Microsoft Developer Studio Generated NMAKE File, Based on cd_maker.dsp
!IF "$(CFG)" == ""
CFG=cd_maker - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. cd_maker - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "cd_maker - Win32 Release" && "$(CFG)" != "cd_maker - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "cd_maker.mak" CFG="cd_maker - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "cd_maker - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "cd_maker - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "cd_maker - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\cd_maker.exe"


CLEAN :
	-@erase "$(INTDIR)\area__rectlist.obj"
	-@erase "$(INTDIR)\cd_maker.obj"
	-@erase "$(INTDIR)\checksum__checksum.obj"
	-@erase "$(INTDIR)\device__device.obj"
	-@erase "$(INTDIR)\dll__dll_man.obj"
	-@erase "$(INTDIR)\error__error.obj"
	-@erase "$(INTDIR)\file__file.obj"
	-@erase "$(INTDIR)\image__image.obj"
	-@erase "$(INTDIR)\inc__search.obj"
	-@erase "$(INTDIR)\init__init.obj"
	-@erase "$(INTDIR)\jj_error.obj"
	-@erase "$(INTDIR)\main__win32_self_modify.obj"
	-@erase "$(INTDIR)\main__win_main.obj"
	-@erase "$(INTDIR)\math__random.obj"
	-@erase "$(INTDIR)\math__spline.obj"
	-@erase "$(INTDIR)\math__transform.obj"
	-@erase "$(INTDIR)\memory__malloc.obj"
	-@erase "$(INTDIR)\palette_pal.obj"
	-@erase "$(INTDIR)\string__string.obj"
	-@erase "$(INTDIR)\threads__win32_threads.obj"
	-@erase "$(INTDIR)\time__timedev.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cd_maker.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\cd_maker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cd_maker.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\cd_maker.pdb" /machine:I386 /out:"$(OUTDIR)\cd_maker.exe" 
LINK32_OBJS= \
	"$(INTDIR)\area__rectlist.obj" \
	"$(INTDIR)\cd_maker.obj" \
	"$(INTDIR)\checksum__checksum.obj" \
	"$(INTDIR)\device__device.obj" \
	"$(INTDIR)\dll__dll_man.obj" \
	"$(INTDIR)\error__error.obj" \
	"$(INTDIR)\file__file.obj" \
	"$(INTDIR)\image__image.obj" \
	"$(INTDIR)\inc__search.obj" \
	"$(INTDIR)\init__init.obj" \
	"$(INTDIR)\jj_error.obj" \
	"$(INTDIR)\main__win32_self_modify.obj" \
	"$(INTDIR)\main__win_main.obj" \
	"$(INTDIR)\math__random.obj" \
	"$(INTDIR)\math__spline.obj" \
	"$(INTDIR)\math__transform.obj" \
	"$(INTDIR)\memory__malloc.obj" \
	"$(INTDIR)\palette_pal.obj" \
	"$(INTDIR)\string__string.obj" \
	"$(INTDIR)\threads__win32_threads.obj" \
	"$(INTDIR)\time__timedev.obj"

"$(OUTDIR)\cd_maker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"

OUTDIR=d:\tp\demo\golgotha\cd_maker
INTDIR=d:\tp\demo\golgotha\cd_maker\debug
# Begin Custom Macros
OutDir=d:\tp\demo\golgotha\cd_maker
# End Custom Macros

ALL : ".\cd_maker.exe" "$(OUTDIR)\cd_maker.bsc"


CLEAN :
	-@erase "$(INTDIR)\area__rectlist.obj"
	-@erase "$(INTDIR)\area__rectlist.sbr"
	-@erase "$(INTDIR)\cd_maker.obj"
	-@erase "$(INTDIR)\cd_maker.sbr"
	-@erase "$(INTDIR)\checksum__checksum.obj"
	-@erase "$(INTDIR)\checksum__checksum.sbr"
	-@erase "$(INTDIR)\device__device.obj"
	-@erase "$(INTDIR)\device__device.sbr"
	-@erase "$(INTDIR)\dll__dll_man.obj"
	-@erase "$(INTDIR)\dll__dll_man.sbr"
	-@erase "$(INTDIR)\error__error.obj"
	-@erase "$(INTDIR)\error__error.sbr"
	-@erase "$(INTDIR)\file__file.obj"
	-@erase "$(INTDIR)\file__file.sbr"
	-@erase "$(INTDIR)\image__image.obj"
	-@erase "$(INTDIR)\image__image.sbr"
	-@erase "$(INTDIR)\inc__search.obj"
	-@erase "$(INTDIR)\inc__search.sbr"
	-@erase "$(INTDIR)\init__init.obj"
	-@erase "$(INTDIR)\init__init.sbr"
	-@erase "$(INTDIR)\jj_error.obj"
	-@erase "$(INTDIR)\jj_error.sbr"
	-@erase "$(INTDIR)\main__win32_self_modify.obj"
	-@erase "$(INTDIR)\main__win32_self_modify.sbr"
	-@erase "$(INTDIR)\main__win_main.obj"
	-@erase "$(INTDIR)\main__win_main.sbr"
	-@erase "$(INTDIR)\math__random.obj"
	-@erase "$(INTDIR)\math__random.sbr"
	-@erase "$(INTDIR)\math__spline.obj"
	-@erase "$(INTDIR)\math__spline.sbr"
	-@erase "$(INTDIR)\math__transform.obj"
	-@erase "$(INTDIR)\math__transform.sbr"
	-@erase "$(INTDIR)\memory__malloc.obj"
	-@erase "$(INTDIR)\memory__malloc.sbr"
	-@erase "$(INTDIR)\palette_pal.obj"
	-@erase "$(INTDIR)\palette_pal.sbr"
	-@erase "$(INTDIR)\string__string.obj"
	-@erase "$(INTDIR)\string__string.sbr"
	-@erase "$(INTDIR)\threads__win32_threads.obj"
	-@erase "$(INTDIR)\threads__win32_threads.sbr"
	-@erase "$(INTDIR)\time__timedev.obj"
	-@erase "$(INTDIR)\time__timedev.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cd_maker.bsc"
	-@erase "$(OUTDIR)\cd_maker.pdb"
	-@erase ".\cd_maker.exe"
	-@erase ".\cd_maker.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D "_AFXEXT" /D "_AFXDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\cd_maker.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cd_maker.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\area__rectlist.sbr" \
	"$(INTDIR)\cd_maker.sbr" \
	"$(INTDIR)\checksum__checksum.sbr" \
	"$(INTDIR)\device__device.sbr" \
	"$(INTDIR)\dll__dll_man.sbr" \
	"$(INTDIR)\error__error.sbr" \
	"$(INTDIR)\file__file.sbr" \
	"$(INTDIR)\image__image.sbr" \
	"$(INTDIR)\inc__search.sbr" \
	"$(INTDIR)\init__init.sbr" \
	"$(INTDIR)\jj_error.sbr" \
	"$(INTDIR)\main__win32_self_modify.sbr" \
	"$(INTDIR)\main__win_main.sbr" \
	"$(INTDIR)\math__random.sbr" \
	"$(INTDIR)\math__spline.sbr" \
	"$(INTDIR)\math__transform.sbr" \
	"$(INTDIR)\memory__malloc.sbr" \
	"$(INTDIR)\palette_pal.sbr" \
	"$(INTDIR)\string__string.sbr" \
	"$(INTDIR)\threads__win32_threads.sbr" \
	"$(INTDIR)\time__timedev.sbr"

"$(OUTDIR)\cd_maker.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ddraw.lib dinput.lib dsound.lib dxguid.lib wsock32.lib comctl32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\cd_maker.pdb" /debug /machine:I386 /out:"cd_maker.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\area__rectlist.obj" \
	"$(INTDIR)\cd_maker.obj" \
	"$(INTDIR)\checksum__checksum.obj" \
	"$(INTDIR)\device__device.obj" \
	"$(INTDIR)\dll__dll_man.obj" \
	"$(INTDIR)\error__error.obj" \
	"$(INTDIR)\file__file.obj" \
	"$(INTDIR)\image__image.obj" \
	"$(INTDIR)\inc__search.obj" \
	"$(INTDIR)\init__init.obj" \
	"$(INTDIR)\jj_error.obj" \
	"$(INTDIR)\main__win32_self_modify.obj" \
	"$(INTDIR)\main__win_main.obj" \
	"$(INTDIR)\math__random.obj" \
	"$(INTDIR)\math__spline.obj" \
	"$(INTDIR)\math__transform.obj" \
	"$(INTDIR)\memory__malloc.obj" \
	"$(INTDIR)\palette_pal.obj" \
	"$(INTDIR)\string__string.obj" \
	"$(INTDIR)\threads__win32_threads.obj" \
	"$(INTDIR)\time__timedev.obj"

".\cd_maker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("cd_maker.dep")
!INCLUDE "cd_maker.dep"
!ELSE 
!MESSAGE Warning: cannot find "cd_maker.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cd_maker - Win32 Release" || "$(CFG)" == "cd_maker - Win32 Debug"
SOURCE=..\area__rectlist.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\area__rectlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\area__rectlist.obj"	"$(INTDIR)\area__rectlist.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\cd_maker.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\cd_maker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\cd_maker.obj"	"$(INTDIR)\cd_maker.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\checksum__checksum.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\checksum__checksum.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\checksum__checksum.obj"	"$(INTDIR)\checksum__checksum.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\device__device.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\device__device.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\device__device.obj"	"$(INTDIR)\device__device.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\dll__dll_man.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\dll__dll_man.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\dll__dll_man.obj"	"$(INTDIR)\dll__dll_man.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\error__error.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\error__error.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\error__error.obj"	"$(INTDIR)\error__error.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\file__file.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\file__file.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\file__file.obj"	"$(INTDIR)\file__file.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\image__image.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\image__image.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\image__image.obj"	"$(INTDIR)\image__image.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\inc__search.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\inc__search.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\inc__search.obj"	"$(INTDIR)\inc__search.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\init__init.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\init__init.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\init__init.obj"	"$(INTDIR)\init__init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\jj_error.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\jj_error.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\jj_error.obj"	"$(INTDIR)\jj_error.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\main__win32_self_modify.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\main__win32_self_modify.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\main__win32_self_modify.obj"	"$(INTDIR)\main__win32_self_modify.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\main__win_main.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\main__win_main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\main__win_main.obj"	"$(INTDIR)\main__win_main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\math__random.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\math__random.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\math__random.obj"	"$(INTDIR)\math__random.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\math__spline.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\math__spline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\math__spline.obj"	"$(INTDIR)\math__spline.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\math__transform.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\math__transform.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\math__transform.obj"	"$(INTDIR)\math__transform.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\memory__malloc.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\memory__malloc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\memory__malloc.obj"	"$(INTDIR)\memory__malloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\palette_pal.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\palette_pal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\palette_pal.obj"	"$(INTDIR)\palette_pal.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\string__string.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\string__string.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\string__string.obj"	"$(INTDIR)\string__string.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\threads__win32_threads.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\threads__win32_threads.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\threads__win32_threads.obj"	"$(INTDIR)\threads__win32_threads.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\time__timedev.cpp

!IF  "$(CFG)" == "cd_maker - Win32 Release"


"$(INTDIR)\time__timedev.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "cd_maker - Win32 Debug"


"$(INTDIR)\time__timedev.obj"	"$(INTDIR)\time__timedev.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

