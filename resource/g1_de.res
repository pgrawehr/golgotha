// Please place all strings need translation in the TRANSLATE section


// **************************** DONT TRANSLATE *************************************

/u/ /u/
/tmp/ /tmp/
x:/ x:/
X:/ X:/
c:/ c:/

-debug true

sys_text_size = 3096000
vid_text_size = 3096000

linux_dll_dir     "linux_dll"
win32_dll_dir     "win32_dll"
debug_dll_substr  "_debug.dll"
dll_extension     "dll"
dll_fullname      "%S/%S"

object_logo_fmt   "bitmaps/logos/%s_logo.jpg"
tdir      "c:/tmp/ctext"
tdir_file "c:/tmp/ctext/tdir.dat"

pcx_sequence    = "%S%S%03d.pcx"
screen_shot_str = "shot%03d.tga"

s_dir	textures
encoding_jpg           "JPG-Datei %s wird gepackt"
copying_textures       "Kopiere Texturen"
loading_textures       "Lade Texturen"
building_texture_cache "Erstelle Texturcache"
updating_texture_cache "Aktualisiere Texturcache"
saving_level           "Speichere Karte"
checking_times         "Pruefe Zeiten..."
max_texture_size 256

movie_option "-movie"

Human = "Human"
"Ai Joe" = "Ai Joe"
"Ai Jim" = "Ai Jim"


fps = "Frames per second"

map1       = maps/map1.pcx

net_cancel = " Cancel "
net_cancel_x = 285
net_cancel_y = 49

net_joined_x = 490
net_joined_y = 106



start_net_game = "     Begin      "
start_server   = " Start New Game "
net_main_menu =  " Return to Menu "
stat_win_title  = "Statistiken"


sky_model    = x:/jc/gdata/sky/sky1.gmod
title_screen = bitmaps/title_screen.jpg
title_buts = {
  bitmaps/menu/lit_return.jpg     198 17
  bitmaps/menu/lit_newgame.jpg    248 88
  bitmaps/menu/lit_load.jpg       280 144
  bitmaps/menu/lit_save.jpg       282 193
  bitmaps/menu/lit_network.jpg    213 303
  bitmaps/menu/lit_options.jpg    256 353
  bitmaps/menu/lit_quit.jpg       282 414
}


net_bg_image      = bitmaps/network/net1.jpg
server_bg_image   = bitmaps/network/net2.jpg
client_wait_image = bitmaps/network/net3.jpg
help_screen       = bitmaps/help.jpg
youwin_screen     = bitmaps/youwin.jpg
youlose_screen    = bitmaps/youlose.jpg

plot_screen       = bitmaps/plot.jpg
startup_screen    = bitmaps/startup.jpg
options_background = bitmaps/options/topbar.tga

cd_test_file = i:/movies.mix


constants   "resource/constants.res"
defaults    "resource/objects.res"
editor_strs "resource/editor.res"
cell_strs   "resource/cells.res"
keys_res_file "resource/keys.res"

remap_pal "tint"


bomb_coming "WARNING!  Bomb Truck Approaching"
switch_to_action  "Press TAB or BACKSPACE to switch back to Action Mode"
switch_to_strategy "Press TAB or BACKSPACE for Strategy Mode"

/////////////// Debug / temp strings
cs_pos "%d %d %d %d"
tmp_savename = "test.level"
play_savename = "play.start"
sfx_fmt = "sfx/%x.wav"

// ****************************** TRANSLATE ****************************************

cheats "Cheats"
build_context_help =
{
  "Baue SuperTank"             // 0
  "Baue Engineer Truck"        // 1
  "Baue Missile Truck"         // 2
  "Baue Panzer"           // 3
  "Fixme!"                      // 4
  "Baue Kamikaze Trike"        // 5
  "Baue Electro Car"           // 6
  "Fixme!"                      // 7
  "Fixme!"                      // 8
  "Baue Jet Bomber"            // 9
  "Baue Attack Helikopter"     // 10
}

rendering_map                    = "Rendere Karte"
options_context_help             = "Detaileinstellungen"
vis_low_help                     = "Schlechte Sicht"
vis_med_help                     = "Mittlere Sicht"
vis_hi_help                      = "Gute Sicht"
normal_pixel_help                = "Normaler Pixel Modus"
double_pixel_help                = "Doppelter Pixel Modus"
interlace_pixel_help             = "Interlaced TV-Modus"
3d/2d_sound_help                 = "3D Sound / 2D Sound"
shadows_help                     = "Schalte Schatten an/aus"
gamespeed_help                   = "Aendere Spieltempo"
initialisation			 = "Initialisierung, bitte warten..."
mount_window_title = "Aufhaengepunkte bearbeiten"
mountpoint_add = "Punkt hinzufuegen"
mountpoint_edit = "Punkt bearbeiten"
mountpoint_remove = "Punkt entfernen"

pad_type_bank     "  Bank Takeover  "
pad_type_lawfirm  "Law firm Takeover"
pad_type_base     "  Base Takeover  "
takeover_context  "takeover_pad : %S"


cell_matchup       =  "Could not matchup cell name %S, check ${cell_strs}"
file_missing       =  "File missing : %S"
bad_texture        =  "Bad file format or file corrupted : %S"
not_32             =  "Texture file is not 32 bit color : %S"
no_create          =  "Unable to create file %S"
bad_anim           =  "FLC file bad : %S"
model_num          =  "Models in resource file = %d, models game is expecting = %d"
sprite_num         =  "Sprites in resource file = %d, sprites game is expecting = %d"
anim_missing       =  "Animations missing from %S"
anim_missing_name  =  "  Animation #%d, name = %s"
old_model_file     =  "Model file %S out of date, please re-fly"
make_crit_graph    = "Connecting network nodes"

up-to-date       "texture up-to-date %S"
updating_texture "updating texture %S"
best_pal         "forming palette from %S"
updating_anim    "updating animation %S"
remaping_texture "remaping texture %S"
no_texture       "Face in model %S has no texture"
bad_normal       "Can't calc normal for face in %S"

out_of_nodes     "Out of graph node memory"
out_of_edges     "Out of graph edge memory"

main_menu_options = { "   Start    "
                      " Networking "
                      "   Quit     " }

bad_file_version "File version not supported"

detail_options = { "Wireframe"
                   "Textured "
                   "White Light "
                   "320 X 200"
                   "400 X 400"
                   "640 X 480" }

load_build "loading building %S"

songs = {
}

sfx_fmt "sfx/%x.wav"
creating_radar_view "Creating Radar View"
recalcing_graph = "Recalculating path solving information"
make_critical_map = "Recalculating AI information" // AI = artificial intelligence

options_buttons = {
}

game_command_error = "Couldn't find command [%S]"
game_commands =
{
  "Left"
  "Right"
  "Forward"
  "Backward"
  "Main Gun"
  "Missiles"
  "Chain Gun"
  "Strafe Left"
  "Strafe Right"
  "Strafe Mode"

  "Map Toggle"
  "Build Blank"
  "Build Commander"
  "Build Peon"
  "Build Trike"
  "Build Rocket Tank"
  "Build Electric Car"
  "Build Helicopter"
  "Build Bomber"
  "Build Engineer"

  "Group 0"
  "Group 1"
  "Group 2"
  "Group 3"
  "Group 4"
  "Group 5"
  "Group 6"
  "Group 7"
  "Group 8"
  "Group 9"
}

game_key_error = "Couldn't find key [%S]"
game_keys = {
//Primary Controls         command
  "Left"                   "Left"
  "Right"                  "Right"
  "Up"                     "Forward"
  "Down"                   "Backward"
  "Space"                  "Main Gun"
  "Enter"                  "Missiles"
  "Left Ctrl"              "Chain Gun"
  "Right Ctrl"             "Chain Gun"
  ","                      "Strafe Left"
  "."                      "Strafe Right"
  "Left Alt"               "Strafe Mode"
  "Right Alt"              "Strafe Mode"
  "Tab"                    "Map Toggle"

//Secondary Controls        command
//--------------------------------------------------------------
  "Del"                    "Left"
  "PageDown"               "Right"
  "Home"                   "Forward"
  "End"                    "Backward"
  "Pad 0"                  "Main Gun"
//"Enter"                  "Missiles"    //repeated from primary
  "Pad 1"                  "Chain Gun"
  "Insert"                 "Strafe Left"
  "PageUp"                 "Strafe Right"
  "\\"                     "Strafe Mode"
  "Backspace"              "Map Toggle"

//Tertiary Controls        command
//--------------------------------------------------------------
  "Pad 4"                  "Left"
  "Pad 6"                  "Right"
  "Pad 8"                  "Forward"
  "Pad 5"                  "Backward"
  "Pad 2"                  "Backward"
//"Pad 0"                  "Main Gun"    //repeated from secondary
  "Pad Enter"              "Missiles"
  "Pad +"                  "Chain Gun"
  "Pad 7"                  "Strafe Left"
  "Pad 9"                  "Strafe Right"

//Gifts to trey            command
//--------------------------------------------------------------
  "'"                      "Forward"
  "/"                      "Backward"
  "Left Shift"             "Main Gun"
  "Right Shift"            "Main Gun"

//Build Keys               command
//--------------------------------------------------------------
  "`"                      "Build Blank"
  "Q"                      "Build Commander"
  "W"                      "Build Peon"
  "E"                      "Build Trike"
  "R"                      "Build Rocket Tank"
  "T"                      "Build Electric Car"
  "Y"                      "Build Helicopter"
  "U"                      "Build Bomber"
  "I"                      "Build Engineer"

  "A"                      "Build Electric Car"
  "S"                      "Build Helicopter"
  "D"                      "Build Bomber"
  "F"                      "Build Engineer"

// Group Keys
  "1"                      "Group 0"
  "2"                      "Group 1"
  "3"                      "Group 2"
  "4"                      "Group 3"
  "5"                      "Group 4"
  "6"                      "Group 5"
  "7"                      "Group 6"
  "8"                      "Group 7"
  "9"                      "Group 8"
  "0"                      "Group 9"
}

sfx_obj_fmt = "Sound '%S', max_volume=%d, hear_dist=%d, ticks=%d"


factory_pad   "Factory Pad"
base_launcher "Base Launcher"
tank_buster   "Tank Buster"
helicopter    "Attack Helicopter"
electric_car  "Electric Car"
rocket_tank   "Rocket Tank"
jet           "Jet Bomber"
trike         "Kamikaze Trike"
peon_tank     "Peon Tank"
turret        "Gun Turret"
sfx_obj       "Sound Player"
lightbulb     "Light Caster"
repairer      "Repair Station"
airbase       "Air Base Station"
bank          "Bank building"
tower_electric "Electric Tower"
tower_missile "Missile Tower"

loading_models "Lade Modelle"
scanning_models "Modelle werden abgesucht"



