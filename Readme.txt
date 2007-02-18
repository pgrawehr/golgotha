------------------------------------------------
Usefull Informations about the Golgotha - Source
------------------------------------------------

For more info, read Manual.doc. 
Setup instructions (from cd): Just run setup.exe in the same directory as golgotha resides. 
Important: If you don't install the textures, the objects or the bitmaps, the CD must be 
in the drive to play. There won't be a senseful error message if it isn't, and the 
program may silently fail. 
When running Golgotha for the first time, it may need to rebuild the texture cache 
(depending on the capabilities of your hardware). This takes quite long, but needs to
be done only once as long as you don't change the render settings. The g_decompressed
folder should always be user-writeable. 
A message saying that some write-permissions don't exist when running for the first time
can be ignored. 

Parameters: 
----------
For unix type systems, the registry is replaced by a file called "golgotha.ini".
-no_full:	Open the game in a window on the desktop (default is fullscreen)
			This is required for debugging except if a second monitor (or 
			remote- debugging) is available. 
-full	        Start in fullscreen mode, even if the registry setting is windowed
-default        Use 640x480x16, regardless of registry setting. Usefull if you
			choosed some setting that doesn't work on your system
			and you therefore cannot change the settings any more.
-setup		Show the configuration dialog right on startup (use if golgotha won't start
		with the default settings)
-no_sound       Disable sound
-eout filename	Write (append) the debug log to the given file.
-max_memory n	Maximum amount of Memory to be used for i4_mem_manager
-edit		Start in edit-Mode
-eval		Command line lisp-evaluation, don't use for now.
-frame_lock     Force one frame per think() loop.
-movie		What format?, where?, when? Unimplemented
-sfxdbg		Display some debug information about sounds. (i.e. Position, Loops)
-no_demo	(default)
-cd_first	check golgotha.cd before local files. Has no effect if no 
			golgotha.cd present. Unpacked files on a CD are always used last.
			(See Search-Path below)
-demo		(ignored)
-3dsound	Use DirectSound3D, defaults to true now.
-display <location> Specify the connection to the X-Server (i.e localhost:2) 
                (Only for unix versions)
-ron		Enable alternative handling or keyboard reapeat events.
		    Usually not required (X server drivers only)
-no_mitshm  Disable MITSHM extension on X Windows display driver. Try this if you find
			that golgotha crashes with strange X warnings. (X server drivers only)


File Search-Path
----------------
The Code behaves as follows if it looks for a file.
(see file__file.cpp)
Case A: -cd_first is not specified (default)
1. Path relative to current directory. 
2. Root of first CD-Rom Drive
3. \golgotha on first CD-Rom Drive
4. Root of second CD-Rom Drive
5. \golgotha on second CD-Rom Drive
6. (more CD-Roms)
7. golgotha.cd (the first one that was found, if one)

Case B: -cd_first is specified
1. golgotha.cd
2. Path relative to current dir.
3. Same as above...

If the file is not found, the system may print a debug message to the debugger.
See file__file.cpp for debugging options.


Data-Files:
-----------
.res	System-config-files (and compiled windows resources (don't mix!))
textures\*.jpg 
textures\*.tga The Textures. TGA-Files may have ARGB format. Use TGA whenever
		you need alpha channels or partially transparent textures.
		TGA files must not be in compressed format! 
		Be sure that there is newer a texture available in both formats.
		Internally, jpg would be prefered over tga, but that's most probably
		not what you want since tga's usually have bether quality than jpg.
g_compressed\.gtx temporary storage for TGA-files. _deprecated_
g_decompressed_(%Bitdepth)\*.gtx Unpacked Graphics-files (to different
		mip-levels)
		Only TGA textues still use this technique. JPG are run time decompressed
		_deprecated_ for all cases
g_decompressed\*.dat texture cache files for different rendering devices.
		They are automatically created and updated if necessary.
.scm	lisp AI-Scripts and in-game configs. 
		They should replace the .res - files sometime, as they are much more 
		powerfull.
.wav	You know what that is, don't you? For Effects, format is 
		16Bit 22khz, Mono. For Background music, usually 16bit 22khz, Stereo
.mp3	Music, not currently used in this format.
.gmod	Golgotha Model Files: Contains the objects (buildings, tanks, trees...)
bitmap\ This directory contains files that are used directly. (Icons, 
		Main-Menu GFX)
sfx\	Here are all the sounds.
.level  The maps or savegames.
Makefile.* Makefiles for different platforms and different compilers.
		Copy the Makefile you want (i.e. 'Makefile.linux.gcc') to 
		'Makefile' and type 'gmake' to build the project. 
		For MSVC, use Golgotha.dsw as workspace file.




Some Symbols for preprocessor
-----------------------------
_WINDOWS  ->use this for windows
__linux  -> and this for linux
__sgi   -> and this for sgi
SUN4   -> and this for SUN4
(for the above ones, check the different Makefiles in the source code distribution)
_DEBUG -> Use this one instead of next
DEBUG  -> Should be Replaced with MSVC's _DEBUG
WIN32 -> Also defined in windows
i4_NEW_CHECK -> Leave as is.
i4_MEM_CHECK
i4_MEM_CLEAR
USE_ASM -> This one and the next is for the assembly implementation of
	the software renderer. Check out render__software__mappers.cpp.
USE_AMD3D needs some extra-files render__software__amd3d...
Hint: Assembly code can currently only be used with MSVC. 
LI_TYPE_CHECK -> Almost always set (performance degradation should be negligible)
[JPG_LOADER] -> Don't touch these, as long as everything works...
ENTROPY_OPT_SUPPORTED
C_MULTISCAN_FILES_SUPPORTED
FULL_COEF_BUFFER_SUPPORTED
Many more in loaders__jpg__jcapistd.cpp



Remarks
-------
Critical Sections don't really work. (This program is multi-threaded!)
This Bug has been fixed.

objs__map_piece.cpp(938): i4_warning("i can't get there!"); [Illegal Waypoint cause for malfunction of Unit creation?]
This Bug has been fixed.

fix 3D-Sound.
This has been fixed
Removed obsolete references to a3d.dll

Allow use of 24 or 32Bit color-depth.
Seems to partially work. Needs more testing and some additional coding to get
some parts (i.e. the radar) working in these modes.

The in-game-messages are corrupted
This bug has been fixed by partially reimplementing the Text-Window.

Decode and Play MP3
Not first priority, as playing MP3 needs more CPU-Power than WAV.

golg__map_lod.cpp(302): 
The setting below should be user-definable as it has a big influence on 
quality and rendering speed. 
i4_float fx = pos.x - mx, fy = pos.y - my, fz = pos.z - g1_get_vertex(mx,my)->get_height(); 
		i4_float d1 = fx*fx+fy*fy+fz*fz; 
		i4_float d2 = p->metric*200; //here!!!
render__tmanager(504):
desired_width = max_texture_dimention; reducing max_texture_dimention here
will reduce texture quality but needs less memory. (must be multiple of 2)
setting it to 8 will use the preloaded lowest miplevel.
These settings can now be controlled in the options dialog. 
Known problem: The textures are now scaled to the choosen
size even if they are much smaller. (Waste of memory)
This has been fixed. The setting allows to define the highest miplevel for all textures.
This includes foreground ones (like the target-crosshairs), will fix that sometime.

if some parts of your code access i4_win32_startup_options, be aware
that the options can change at runtime!


Initalization (in init_init.cpp: void i4_init(void))
0=I4_INIT_TYPE_MEMORY_MANAGER,     // main i4 memory manager
1=I4_INIT_TYPE_PRIORITY,	   // things that need to be initialized before anything else
2=I4_INIT_TYPE_THREADS,            // initialized thread info
3=I4_INIT_TYPE_LISP_MEMORY,        // for lisp object allocations - uses i4 memory manager,
4=I4_INIT_TYPE_LISP_BASE_TYPES,    // adds lisp types into the system (li_int.. etc)
5=I4_INIT_TYPE_LISP_FUNCTIONS,     // adds lisp functions (li_load & any user li_automatic..)
6=I4_INIT_TYPE_STRING_MANAGER,
7=I4_INIT_TYPE_FILE_MANAGER,
8=I4_INIT_TYPE_DLLS,
9=I4_INIT_TYPE_BEFORE_OTHER,       // init just before the big mess starts
10=I4_INIT_TYPE_OTHER,
11=I4_INIT_TYPE_AFTER_ALL	  // For objects that rely on some that get inited on OTHER.

Improve the LISP-ENGINE to the level of Abuse
must implement most system functions, namely the arithmetic functions and 
the "defun" operator
new types: LI_USER_FUNCTION, LI_VECTOR (one dimensional array of arbitrary size)
Need to get the emacs-lisp-examples to run.
Most of this has been implemented. flame.scm (the emacs flamer) works!

Fix the mapcar-code
fixed.

Fix Mouse in FS, don't reallocate the mouse-backbuf every time
fixed.

Fix Maxtool in FS
fixed. (currently, if double-buffering, we copy the frontbuffer back every frame)

Save Detail level in registry
done.

Improve opening-screen
done. Might be even better. Need to add possibility to play intro-movie.
This is possible now. Use (play_video FILENAME) for it.

Allow exclusive focus on the keyboard for textual input (no side-effects desired)
Implementation done.

Even modal windows are possible now.

Allow direct access to options and lisp engine on command-line
not yet implemented.

Fix the sky and the sky-select window
still buggy.

Known Problems:
User Functions get their arguments once evaluated (li_bind_params evaluates args), while
System functions get their parameters unevaluated.
This is an inherent problem of the language, that cannot be changed, CLISP 
has same problem.

Reseting the Texture Manager when a map is loaded is impossible
Cause all Texture handles get lost. Need to implement a "reopen()"
method to free memory but not unload the tman. Then it should be 
possible to create a new instance of the texture manager.
Creating a new instance of the tman and then reverting to the default
one must restore the global pointers to the instances.
Done, works mainly. Some trouble with the sky. Need to fix that anyway.
Did many fixes again. Restoring the textures after ALT+TAB seems to work now.

g1_saver_classes get leaks references.
Fixed.

verybiggun needs fixes.
Done, seems to work. Fires very inaccuratelly.

field_camera occupy_location() must be changed.
Done. References to field_cameras seem to get leaked.

bird needs fixes.
Done. Looks fine.

cobra_tank needs proper think() method.

changing the height of objects in the editor needs fixing.

where comes the window for ambient sound-editing from?
Found.

object display window in editor (on the pick-objects list) needs fixing.
Looks acceptable.

what can be done with sprites?

Selecting objects during game doesn't work. Must look at g1_render.current_selectable_list.
golg__controller Line 415.
Fixed, works very fine.

Must add a function to change the players ai.
Done. The ai of each player can now be set in the player params dialogue in the editor
The different ais need fixing, though.

Moving the mouse to the edge of the controller - window should look around
in strategic and action modes.
Implemented in a very simple way. Doesn't work yet in strategy mode.


Remarks about i4-class-hirarchy
-------------------------------

Window-Management

i4_style_manager_class (mit its static instance i4_style_man) handles
all window styles. The styles are of type i4_graphical_style_class 
(An almost pure virtual abstract class). 
That one is a child of i4_init_class.
Currently, i4_mwm_style_class is the only implementation of 
i4_graphical_style_class and therefore it defines the look of all 
windows. 

The window-manager itself is a child of i4_event_handler.

i4_event_handler_class
	...
	i4_window_class
		i4_mwm_....
		i4_parent_window_class
			...
			i4_window_manager_class

Object-Handling
Ableitungen von g1_object_class


People that need to be mentioned in the credits:
------------------------------------------------
(add yourself if you want to one or more groups)

Coding:    Jonathan Clark
		   Jan Jasinski
		   Ferret Malicious
		   Patrick Grawehr

Music:    -Hunting the Enemy - MP3 
           Classical, VideoGame
  
           Year:1998 - From the Album: Four 
        
	       Ratings
           May be freely distributed without profit; may be played on radio; may not be sold

	  -others: Crack dot com

Sfx:      -Crack dot com

Levels:   -test: crack dot com
          -egypt: originally crack dot com, upgraded by Patrick Grawehr
	      -snow: originally crack dot com, upgraded by Patrick Grawehr

Objects:   Crack dot com

Textures: -most: Crack dot com
          -some free sources on the net, like 3dcafe (www.3dcafe.com)
		  -data becker "media pool" cd.

Bitmaps:  -Crack dot com
	  -Patrick Grawehr
      

Beta Testing: 
	      -Edward Downling

Web Server: 
		  -Christea Virgil Ionut


Special thanks to Jonathan Clark and his Crack Dot Com team for relasing the 
source code of this great game into public.
Unfortunatelly, I was unable to find out the names of the other members of
the original Crack dot com team. If anyone knows, please add them above.

Release History:
----------------

V1.0.9.2
	(common)
	Subversion on qbic is operating fully now. See the changelogs
	there for details.
	It's now possible to control every single unit by hand. 
	Improved the border frame window.
	Fixed some old and anoying bugs. 

V1.0.8.4
	(common)
	Lots of cleanups. 
	Made the golgotha source code including the version control
	available on qbic.vipnet.ro. 

V1.0.8.3
	(FreeBSD)
	Golgotha compiles and runs under freebsd. Checked with gcc-3.3.
      Does *not* compile with gcc-2.95 (the default on bsd) because
      that one can't properly handle CR/LF linefeeds.
	(common)
	The maxtool can now create and render an octree from an object. 
	  Saving and loading is not yet implemented. 

V1.0.8.2
	(common)
	Lisp symbols can now be attached properties. The commands are 
	  (put 'symbol 'name 'value) and (get 'symbol 'name)
	Fixed several minor bugs all over the place.
	To avoid problems in networking, all object types are now sorted
	  by name before instantiating the objects themselves.
	(linux)
	Fixed some minor compiler-related inconsistencies to get the
	  code to compile under linux again.
	The network code is running under linux now. Windows and linux
	  version talk to each other (with the limitation that the network
	  protocol of golg is incomplete and has bugs)

V1.0.8.1
    (common)
	Redesigned the lisp memory management code: All global memory 
	  limitations are gone. (Except for the 4GB mark, of course)
	Fixed a minor bug in the lisp code that could lead to inconsistent
	  data handling.
	Lisp dialogs now properly display the name of boxed classes.
	Fixed some other bugs in the lisp code and the garbage collector.
	Added a feature to change the cloud texture. (should do more on the sky)

V1.0.8.0
	(windows)
	Golgotha builds again under VC6, including the DirectX9 support.
	The DX9 Mouse is almost functional. Since dx9 uses page flipping
	  again, some minor glitches might occur. 
	(common)
	New feature added: Zooming. Can only be used manually now. Press 
	  Alt-F7 to see the camera window. Change the scale_x and scale_y
	  parameters to zoom. Larger values bring you nearer to the target.
	  Values smaller than one currently result in missing polygons
	  and on values larger than one, the detail level is not increased
	  for far objects (that should become visible if zooming).
	  Using non-square values (scale_x and scale_y differ) will yield 
	  a very funny picture...
	Fixed a small bug that made some dialog windows a bit dangerous to use: 
	  The yes and no buttons had the same action.

V1.0.7.0
	(windows)
	Added a new rendering device driver for DirectX9. This one is now the default 
	  and will always be used if available. The usage of DirectX9 avoids
	  some known bugs of DX5 and might increase performance on newer systems.
	The Mouse cursor is for dx9 is not yet functional, gotta do this soon.
	A setup script has been generated and can be used now.
	I'm now using Visual Studio.NET, so exspect additional workspace files.
	Golgotha also builds on VS.NET 2003, after some minor modifications.
    (common)
        The Cobra Tank lisp implementation is functional. 	
	Fixed a few flaws of the level editor. 
    
V1.0.6.1
    (common)
	The software renderer now uses true alpha scanline drawing as default.
	Improved lisp object creation functions. (needs further work)
	(unix)
	The Settings are now correctly saved and loaded from golgotha.ini.
	Removed all STL code from app_cdatafile.
	Added new Makefiles for other plattforms/compilers.
	Software rendering with an X11 type display now works. 
	  (add a line "display=X Windows" in golgotha.ini)
	The unix builds now also have a useful error-window.
	
V1.0.6.0
	(common)
	Further improved collision handling.
	Improved the enemy stank ai a tiny bit, such that he doesn't get stuck so quickly.
	Fixed a few bugs in the texture management.
	Cleanup in arch.h.
	Perhaps the sky is now finally looking kinda like it should?
	Objects can now be added using plain lisp code only. 
	Implemented a kind of 3d-logo programming to learn about vector translations and 
	  other matrix-vector operations in 3d space. 
	Fixed a bug in the A* map solver that caused stanks to take bad paths. Unfortunatelly,
	  A* doesn't seem to be really much faster that breath first search in most cases,
	  perhaps we need some improvement here.
	(new)
	Golgotha compiles and runs on SUN Sparc Solaris and Silicon Graphics SGI with OpenGL support.
	Golgotha now has the software renderer builtin again. Works under windows
	  in either directx mode or plain gdi mode. Unix not tested yet. 
	(unix only)
	Fixed a bug that caused the ai to fail on big endian machines.
	The settings are now saved to a file called golgotha.ini. 
	
V1.0.5.6
	(common)
	Improved collision handling (still not fast and precise enough)
	The statistics window now correctly displays the number of 
	  elapsed frames and the number of calculated ai-steps. 
	Fixed a bug that prevented the placement of some objects at most
	  locations in the editor.
	The radar can now be correctly hid in the ai-window. An enemy stank
	  no more uncovers it. 

V1.0.5.5
	(Windows only)
	Continued working on the network code.
	Fixed the "Win2000/XP show no sensefull error messages" bug.
	(common)
	Added support for special unit commands. The models for the buttons still need
	  to be written and the button placement is bad.

V1.0.5.4
	(Windows only)
	Partial network support added. Still need to implement all the load() methods.
	Fixed a few strange bugs in the math library.
	Did a few tests with stereo-vision support. 

V1.0.5.3
	(common)
	Huge cleanups in the texture management. Now all textures load directly
	  from the textures folder. No more g_compressed folders needed. 
	Lots of bugs fixed that became visible with the above change.
	The tex_chache.dat file now also contains lowmips for 32 bit texture modes.
	I hope I haven't broken anything that worked before. (Did some tests, 
	  but there might still be unhandled cases)
	Hint: After updating to this version, you can delete all g_compressed and
	  g_decompressed* folders. They will be rebuilt if required.

V1.0.5.2
	(Windows only)
	Optimized the asynchronous jpg texture loader.
	Golgotha now supports 32 Bit texture modes. This requires a screen resolution
	  with 24 or 32 bit color depth. Hint 1: Some older graphics cards might not 
	  support hardware acceleration in these modes, so the performance can drop 
	  significantly. Hint 2: There's no texture cache for 32 Bit textures yet,
	  so loading can take quite long.
	You find the new settings on the graphics tab in the options.
	Better support for different software renderer types.
	(Linux only)
	The higher texture levels now load correctly.
	File load debug statements added.
	Fixed a strange bug with multiple render windows in the editor.
	Fixed a shutdown error.
	The button images now display correct collors.
	Fixed the render-window-overlaps-menus-and-windows bug. 
	(common)
	Added set and get texture methods to texture manager.
	The async readers now use 2 different queues depending on priorities of 
	  the request.
	Image32, Image16, Image24: Improved copy() method.
	Image8, i4_pal_man: Fixed a nasty but that caused palettes to get screwed up
	  esp. under linux.
	The stank now has a different model for the top if viewed from outside.
	The sky and the clouds now look _much_ better. Choosing the sky now works.
      Need still changing that model.
	Fixed a memory leak in maxtool.
	Added fn to update the lod - texture (not yet working properly under linux)
	Fixed some bugs in the transportation simulation.
	Minor performance improvements.

V1.0.5.1
	Golgotha is compiling, linking and running again under linux!!!
	Just use the included "Makefile" to get it done. You need to have
	the "mesa" and "mesa-devel" (or similar) opengl support package installed.
	I'm using gcc V2.95 on mandrake 8.1.
	Further improved the transportation simulation.
	Fixed some minor bugs.
	
V1.0.5.0
	I'm using VSS now. The Complete what's new feature is now included in a file
	  called ssreport.txt.
	Improved the selecting and comanding of units.
	Units can now be pointed at to show their remaining health.
	Implementing a transportation simulation on golg, for my studies.
	Increased the flexibility of most data structures to support a larger
	  amount of objects on the map. (must still be improved)
	
V1.0.4.1
	New Objects: Lawfirm, armor (no visible effect yet)
	Supergun accepts user targets.
	Most units can now be selected and manually commanded.
	Fixed another bunch of memory leaks in the object management.
	In action mode, you can now move the cursor to the edge of the
	  screen to turn faster.

V1.0.4.0
	Got the following objects to work: Bomber, Rocket tank, tank buster, 
	  Jet, Bridger and the supergun.
	Fixed a bug in the texture management, that caused some textures to
	  get screwed up on DDERR_SURFACELOST.
	Hope to have fixed a bug to avoid a fatal error loading lowest 
	  miplevels. 
	Default position for the statistics window (FPS, Polys) is now top
	  right corner of screen instead of center.
	Remark: Release build is about 5 times faster than debug build.
	  I reach about 20 to 27 FPS on my Celeron-433.

V1.0.3.6
	Fixed a bunch of memory leaks.
	Added new editor mode to find out infos about already placed tiles.
	Tile picker now shows the name of a texture.
	Partial implementation of lisp bignums working. (addition, subtraction, 
	  multiplication). Use a "b" after the operation to use the bignum-version.
	  I.e (+b 999999999999 88888888888888)
	  Try (! 100) *g*
	Added some other lisp system functions.

V1.0.3.5
	Removed main__win32_self_modify from project. (Not needed)
	Implemented possiblity for playback of avi files.
	Fixed a bug introduced with the changes to the sound-engine.
	Updated AI-Window with a few new modes. 
	  Now three different path-solving algorithms are available:
	  1. Breadth_first_map_solver: Finds the shortest path for a given
	  grade (uses blocking indications on the map)
	  2. Astar_map_solver: Finds shortest path using some info
	  on the map (I think this one is usable only for supertanks
	  or airplanes)
	  3. Breadth_first_graph_solver: Uses a precompiled graph to find
	  a path (requires that the map contains one, test.level doesn't)
	Checkboxes and Radiobuttons are now available as new controls.
	A group box for radiobuttons that ensures exactly one is choosen
	  is also available.

V1.0.3.4
	Implemented the possiblity to use modal dialog boxes. Create them using
	  style->create_modal_window(). Any mouse-down events outside the
	  modal window are ignored. The key-manager is inactive, too.
	  Modal windows immediatelly get the fucus when opened and cannot loose it,
	  except if another modal window is opened on top of them.
	  Still missing: Some feedback to the user when clicking outside
	  of modal windows. Not doing this now, as the soundman needs work
	  anyway. Should do that in one part.
	Removed os-dependent code from golg__sound_man.cpp. Now uses 
	  i4_signal_object for synchronisation.
	Extended capablilities of singal-objects.
	Removed obsolete reference to a3d.lib and a3d.dll.
	All error or warning messages are now broadcasted through the kernel.
	Found a problem with the map. If we cannot load some of the texture files
	  ("Texture %s not found in texture cache...") the index of the map
	  texture doesn't get recalced and therefore is invalid (reverts to
	  white).
	  Workaround (not fix): Check that the scm-file doesn't contain 
	  inexistent textures.
	Found the Bug that causes reloading of textures to fail: memory_images
	  (the map) are not conserved upon reopen() and load_textures().
	  This bug has been fixed.
	The sky still needs fixing. Not doing this now, as it will be entirelly
	  rewritten.
	There seems to be a bug in the sound engine when no sound is requested
	  or forced (i.e Winamp running). We run out of mem?!

V1.0.3.3
	Restoring all the surfaces and textures after DDERR_SURFACELOST seems 
	  to work.
	Reloading the textures still has trouble (max_view_distance becomes
	  important then, this needs fixing)
	The Target of the enemy supertank is no more shown in action mode.
	Resizing the main window in windowed mode works for action and strategy modes
	The radar no more uses G1_RADAR_INTERLACED.
	Added new error-dialogue.
	The border frame now displays the active ammo types. (needs change 
	  though, as sometimes, supertank updates are on the spot, other 
	  methods don't change until the next rebuild)
	Other updates I don't remember.

V1.0.3.2
	Fixed a bunch of bugs, i.e. Texture loading, Object Picker class
	Added many editing options to Maxtool
	Combined res/ and resource/ directories in resource/ directory.
	Enemy is using his SuperTank again
	Fixed draw_3d_line() and Maxtool to prevent overwritting of windows.
	Added old ai code

V1.0.3.1
	Fixed full-screen-mode (no more flickering, but a bit slower)
	Options fixed.
	The lisp-engine has been updated. Most features are implemented, including:
		-Lambda functions
		-defun operators
		-mapcar and other functions that take functions as operands
		-many system functions such as arithmetics, stacks, internals
		-backquote corrected
	Implementing the missing CLISP-functions is just a matter of time.
	Started to update the maxtool to become a fully usable 3D-Object editor.
	Implemented a command line utility to enter lisp commands. 
		Press F11 to see the window. Press ALT-F11 to open the debug window
		which takes the replies. This will change soon.
	Small change to the initialisation sequence.


V1.0.2.2
	Included the Maxtool in the Golgotha Main Executable. This simplifies compilling
		and debugging. Press F10 to get there.
	Fixed a bunch of memory-leaks.
	Alpha mode fully working.
	Some additions to the Option-Settings. Rendering Quality can be choosen.
	Fixed a bug in the texture manager that caused an overload of the disk cache and 
		used up far to much memory.
	A rendering device can now have multiple texture managers attached to it.
	The Texture manager can now reload it's textures without the need to 
		reregister them. This helps in situations, where the tman needs to reload the 
		textures (i.e Maxtool) but the main textures shan't change.
	Out of Memory conditions are now reported to the user before the process quits.
	Implemented i4_char_send_event_class: Should be used to send ascii data to windows
		where translations are welcome. (The textinput-window is currently the
		only one to use it)
	Fixed a bug that caused the kernel to loose events when a WM_KEYUP message 
		corresponding to some WM_KEYDOWN event got lost due to the window losing focus.
	An intro-image is now displayed.


V1.0.2.1 
	Reimplemented Extras/AI Menu for old Maps
	JPG-Textures are now run-time decompressed
		(Software Renderer has not been adapted yet, so doesn't work right now)
	32Bit-Color Depth Mode partially works (no map, textures still 16 bit)
	Improved Options, Graphics Settings are now usefull
	More Progress bars visible
	Fixed some of the textures with color key. Alpha still needs some work.
	Implemented i4_temp_file_class
	Implemented reverse-lookup of texture ids
	Some minor changes to the memory-subsystem. This mainly has the purpose
		of using the default c++ new operator instead of I4_MALLOC to get
		more memory.
	Added some system-functions from abuse to the lisp-subsystem, needs
		much more to get it completed
	Landscape textures are preloaded during map-rendering.

V1.0.0.1 
	Fixed Unit doesn't see first waypoint bug
	Implemented searching the CD for files
	Reactivated the menu.
	Implemented Options to set Resolution and Sound
	Fixed Critical Sections
	Fixed Soundman-Poller (now Real-Time-Capable)
	Fixed the scrolling in-game text window.

V1.0.0.0 Original Version as published on www.jonathanclark.com/crack.com/golgotha
