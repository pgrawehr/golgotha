//This file is the main source for precompiled headers
//This has no great advantage in compilling speed 
//versus the need to recompile everything when one of
//the include files changes.
#ifndef __PCH_H
#define __PCH_H

#include <stdlib.h>
#include <string.h>
#ifdef _WINDOWS
//Those are needed only for windows
//Stdafx includes windows.h
#include "stdafx.h"
#include <ddraw.h>
//#include <d3d.h>
#endif
#include "time/profile.h"
//#include "window/wmanager.h"
//#include "map.h"
//#include "tile.h"
#include "app/app.h"
//#include "controller.h"
//#include "input.h"
//#include "objs/map_piece.h"
//#include "player.h"
#include "math/random.h"
#include "resources.h"
//#include "menu.h"
//#include "border_frame.h"
//#include "sound/sfx_id.h"
//#include "remove_man.h"
//#include "saver.h"
//#include "mess_id.h"
//#include "sound_man.h"
#include "time/time.h"
#include "time/timedev.h"
//#include "g1_speed.h"
//#include "cwin_man.h"
//#include "m_flow.h"
//#include "g1_render.h"
//#include "objs/defaults.h"
//#include "net/startup.h"       // g1 stuff for start network menus
//#include "network/net_prot.h"  // so we can choose a protocol when we start
//#include "render\r1_api.h"
//#include "g1_texture_id.h"
//#include "statistics.h"
//#include "map_man.h"
//#include "render\tmanage.h"
//#include "level_load.h"
//#include "lisp/lisp.h"
//#include "make_tlist.h"
//#include "gui/image_win.h"
//#include "font/anti_prop.h"
//#include "lisp/li_init.h"
//#include "tick_count.h"
//#include "map_view.h"
//#include "image_man.h"
//#include "options.h"

//#include "editor/editor.h"
//#include "file/file.h"
#endif


