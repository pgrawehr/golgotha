# Key name                         Command                      Contexts
#                                  Usually a lisp call          anything can be mentioned here
#max number of different contexts: 32
#max number of defined keys: unlimited
#Modifiers (shift, alt, Ctrl) can either be used individually (i.e Ctrl to shoot in action mode)
#or as real modifier (i.e Alt+S to save), but not both at the same time in the same mode.
#----------------------------------------------------------------------
(def_key  "Escape"                 "quit_demo"                  action strategy follow menu)
(def_key  "Left"                   "Move Left"                  action follow)
(def_key  "Right"                  "Move Right"                 action follow)
(def_key  "Up"                     "Move Forward"               action follow)
(def_key  "Down"                   "Move Backward"              action follow)
(def_key  "Del"                    "Move Left"                  action follow strategy)
(def_key  "PageDown"               "Move Right"                 action follow strategy)       
(def_key  "Home"                   "Move Forward"               action follow strategy)       
(def_key  "End"                    "Move Backward"              action follow strategy)

(def_key  "Shift+Left"             "Rotate Left"                strategy)
(def_key  "Shift+Right"            "Rotate Right"               strategy)
(def_key  "Shift+Up"               "Pan Up"                     strategy)
(def_key  "Shift+Down"             "Pan Down"                   strategy)
(def_key  "Alt+Up"                 "Rotate Up"                  strategy)
(def_key  "Alt+Down"               "Rotate Down"                strategy)
(def_key  "Alt+Left"               "Tilt Left"                  strategy)
(def_key  "Alt+Right"              "Tilt Right"                 strategy)
(def_key  "Left"                   "Move Left"                   strategy)
(def_key  "Right"                  "Move Right"                  strategy)
(def_key  "Up"                     "Move Forward"                strategy)
(def_key  "Down"                   "Move Backward"               strategy)

#(def_key  "Alt+Up"                 "Pan Up"                     strategy)
#(def_key  "Alt+Down"               "Pan Down"                   strategy)       

(def_key  "Space"                  "Main Gun"                   action follow)
(def_key  "Pad 0"                  "Main Gun"                   action follow)
(def_key  "Enter"                  "Missiles"                   action follow)
(def_key  "Pad ."                  "Chain Gun"                  action follow)                
(def_key  "Pad ."                  "Nothing" 	                strategy)                
(def_key  "Ctrl"                   "Chain Gun"                  action follow)
#the following is used for selections in strategy mode
#(def_key  "Ctrl"                   "Nothing"                    strategy) 
(def_key  "Alt+S"		   "File/Quicksave"		action follow strategy)
(def_key  "Alt+L"		   "File/Quickload"		action follow strategy)

(def_key  ","                      "Strafe Left"                action follow)                
(def_key  "Insert"                 "Strafe Left"                action follow)                
(def_key  "."                      "Strafe Right"               action follow)                
(def_key  "PageUp"                 "Strafe Right"               action follow)
(def_key  "Shift+Up"               "Strafe Up"                  action follow)
(def_key  "Shift+Down"             "Strafe Down"                action follow)
(def_key  "Shift+Left"             "Strafe Left"                action follow)
(def_key  "Shift+Right"            "Strafe Right"               action follow)
              
(def_key  "Tab"                    "strategy_toggle"            follow strategy action)
(def_key  "Backspace"              "strategy_toggle"            follow strategy action)

(def_key  "C"                      "camera_mode"                action)
(def_key  "Y"                      "Zoom In"                    follow)
(def_key  "X"                      "Zoom Out"                   follow)
(def_key  "C"                      "Zoom Up"                    follow)
(def_key  "V"                      "Zoom Down"                  follow)
(def_key  "B"                      "Zoom Left"                  follow)
(def_key  "N"                      "Zoom Right"                 follow)

(def_key  "P"                      "Pause"                      follow action strategy editor)
(def_key  "G"                      "toggle_rendering"           editor)

(def_key  "<"                     "Strafe Mode"                action follow)                
                                                                                       
(def_key  "Alt+F4"                 "File/Exit"                  editor action strategy follow menu)
(def_key  "Alt+F9"                      "music_down"                 action strategy follow editor menu maxtool)
(def_key  "Alt+F10"                      "music_up"                   action strategy follow editor menu maxtool)

(def_key  "0"                      "upgrade_level_0"            action strategy follow)
(def_key  "1"                      "upgrade_level_1"            action strategy follow)
(def_key  "2"                      "upgrade_level_2"            action strategy follow)

(def_key  "["                      "shrink_screen"              action strategy follow)
(def_key  "]"                      "grow_screen"                action strategy follow)
(def_key  "m"                      "toggle_follow_mode"         action follow)
                                                                                       
# Gifts to trey                    command                                                     
#--------------------------------------------------------------------                 
(def_key  "'"                      "Move Forward"               action follow strategy)
(def_key  "/"                      "Move Backward"              action follow strategy)
#need the modifiers for selections
#(def_key  "Shift"                  "Main Gun"                   action follow strategy)       
                                                                                       
                                                                                       
#Build Keys                         command                                                     
#--------------------------------------------------------------------                 

#(def_key  "0"                      "team_0")
#(def_key  "1"                      "team_1")
#(def_key  "2"                      "team_2")
#(def_key  "3"                      "team_3")
#(def_key  "4"                      "team_4")
#
#
#(def_key  "Alt+0"                  "tint_none")
#(def_key  "Alt+1"                  "tint_polys")
#(def_key  "Alt+2"                  "tint_all")

  
(def_key  "Q"                      "Build stank"                action follow strategy)       
(def_key  "Shift+Q"                "Goto_stank"                 action follow strategy)
(def_key  "W"                      "Build peon_tank"            action follow strategy)       
(def_key  "E"                      "Build trike"                action follow strategy)       
(def_key  "R"                      "Build rocket_tank"          action follow strategy)       
(def_key  "T"                      "Build electric_car"         action follow strategy)       
(def_key  "Z"                      "Build tank_buster"          action follow strategy)       
(def_key  "U"                      "Build jet"                  action follow strategy)       
(def_key  "I"                      "Build bomber"               action follow strategy)       
                                                                                       
(def_key  "A"                      "Build bridger"              action follow strategy)       
(def_key  "S"                      "Build helicopter"           action follow strategy)       
(def_key  "D"                      "Build jet"                  action follow strategy)       
(def_key  "F"                      "Build engineer"             action follow strategy) 
(def_key  "G"                      "Build bomb_truck"           action follow strategy)     

(def_key  "H"                      "Tools/Radar"                action follow strategy editor)      
        
(def_key "Alt+J"                       "File/Open Anim"             action follow strategy editor)

# Debugging keys
# -----------------------------------------------------------------
(def_key  "O"                      "Toggle Stats"                action follow strategy editor maxtool)

(def_key "Alt+Q"		   "global_options"              action follow strategy
editor maxtool menu)

(def_key  "F1"                     "Show_Help"                   action follow strategy
menu)
(def_key  "F2"                     "Screen Shot"                 action follow strategy editor maxtool menu)

(def_key  "F3"                     "play_demo"                   action follow strategy)
(def_key  "F4"                     "record_toggle"               action follow strategy)

(def_key  "F5"                     "Map/Change Sky"              editor follow strategy action)
(def_key  "F6"                     "Edit/Toggle Menu"            action follow strategy editor menu)
(def_key  "F10"			   "Maxtool/Toggle Menu"	 action follow strategy maxtool menu)
(def_key  "Alt+F12"		   "Hide_Main_Menu"		 editor maxtool)
(def_key  "F7"                     "Tools/Profile"               action follow strategy editor maxtool menu)
(def_key  "F8"                     "Cheat Menu"                  action follow strategy editor)
(def_key  "F9"                     "edit_music"                  action follow strategy editor maxtool)
(def_key  "Alt+F7"                 "edit_camera"                 action follow strategy editor)
(def_key  "F11"			   "Tools/Lisp Interaction"      action follow strategy editor maxtool menu)
(def_key  "Alt+F11"		   "Tools/Debug"		 action follow strategy editor maxtool menu)
(def_key  "F12"                    "File/Save As"                editor)




# Editor keys
# ------------------------------------------------------------------

(def_key  "Left"                   "Pan Left"                   editor)
(def_key  "Right"                  "Pan Right"                  editor)
(def_key  "Up"                     "Pan Forward"                editor)
(def_key  "Down"                   "Pan Backward"               editor)

#(def_key  "Del"                    "Pan Left"                   editor)
#(def_key  "PageDown"               "Pan Right"                  editor)
#(def_key  "Home"                   "Pan Forward"                editor)
#(def_key  "End"                    "Pan Backward"               editor)

(def_key  "Del"                    "Map/Delete Selected"        editor)
#The following should be the same in editor and in strategy mode.
#This leaves Ctrl as lone fire key for action mode.
(def_key  "Shift+Left"             "Rotate Left"                editor)
(def_key  "Shift+Right"            "Rotate Right"               editor)
(def_key  "Shift+Up"               "Pan Up"                     editor)
(def_key  "Shift+Down"             "Pan Down"                   editor)
(def_key  "Alt+Up"                 "Rotate Up"                  editor)
(def_key  "Alt+Down"               "Rotate Down"                editor)
(def_key  "Alt+Left"               "Tilt Left"                  editor)
(def_key  "Alt+Right"              "Tilt Right"                 editor)


(def_key  "Ctrl+N"                 "File/New"                   editor)                       
(def_key  "Ctrl+S"                 "File/Save"                  editor)                       

(def_key  "Ctrl+D"                 "File/Open DLL"              editor)  
                     

                                                                                       
(def_key  "Ctrl+Z"                 "Edit/Undo"                  editor)                       
(def_key  "Ctrl+A"                 "Edit/Redo"                  editor)
(def_key  "Ctrl+X"                 "Edit/Cut"                   editor)
(def_key  "Ctrl+C"                 "Edit/Copy"                  editor)
(def_key  "Ctrl+V"                 "Edit/Paste"                 editor)


(def_key  "Alt+1"                  "View/1 View"                editor)  
(def_key  "Alt+4"                  "View/4 Views"               editor)  
(def_key  "Ctrl+6"                 "View/Wireframe"             editor)
(def_key  "Ctrl+7"                 "View/Textured"              editor)

(def_key  "Ctrl+F9"                "View/Toggle Wireframe"      editor)
(def_key  "Ctrl+F10"               "View/Toggle Textured"       editor)


(def_key  "Alt+O"                  "Tools/Objects"              editor)
(def_key  "Alt+T"                  "Tools/Tiles"                editor)
(def_key  "Alt+R"                  "Tools/Radar"                editor)

(def_key  "Ctrl+T"                 "Map/Simulate Tick"          editor)

(def_key  "Ctrl+Q"                 "full_reload_level"          editor)
(def_key  "Ctrl+R"                 "reload_level"               editor)


(def_key  "Alt+L"             "Terrain/Recalc Light"       editor)
(def_key  "Alt+M"             "Terrain/Merge"              editor)
(def_key  "Alt+F"             "Terrain/Flatten Selected"   editor)
(def_key  "Alt+S"             "Terrain/Smooth Selected"    editor)
(def_key  "Alt+N"             "Terrain/Add Noise to Selected" editor)

(def_key  "Ctrl+O"                 "Objects/Select Game Pieces" editor)                       


(def_key  "Alt+Space"              "edit_selected"              editor)
(def_key  "Alt+X"                  "add_link"                   editor)
(def_key  "Alt+Z"                  "remove_link"                editor)

(def_key  "Alt+'"                  "read-eval"                  editor follow strategy action)
(def_key  ","                      "move_selected_down"         editor)
(def_key  "."                      "move_selected_up"           editor)

(def_key  "Alt+["                  "fix_previous_link"          editor)
(def_key  "Alt+]"                  "fix_forward_link"           editor)
(def_key  "Alt+P"                  "fix_path_link"              editor)

# Keys for Maxtool context
# Key name                         Command                      Contexts
#----------------------------------------------------------------------
(def_key  "Ctrl+O"                 "open_model"                 maxtool)
(def_key  "Ctrl+S"                 "save_model"                 maxtool)
(def_key  "Ctrl+A"                 "saveas_model"               maxtool)
(def_key  "Alt+F4"                 "File/Exit"                  maxtool)
(def_key  "Alt+W"                  "wireframe"                  maxtool)
(def_key  "Alt+T"                  "tint_sel"                   maxtool)
(def_key  "Ctrl+T"                 "untint_sel"                 maxtool)
(def_key  "Ctrl+F"                 "flip_normal_sel"            maxtool)
(def_key  "Ctrl+R"                 "rotate_texture_sel"         maxtool)
(def_key  "Ctrl+D"                 "drag_select"                maxtool)


(def_key  "Alt+0"                  "no_tint"                    maxtool)
(def_key  "Alt+1"                  "m1_team_1"                     maxtool)
(def_key  "Alt+2"                  "m1_team_2"                     maxtool)
(def_key  "Alt+3"                  "m1_team_3"                     maxtool)
(def_key  "Alt+4"                  "m1_team_4"                     maxtool)

(def_key  "Ctrl+0"                 "back_black"                 maxtool)
(def_key  "Ctrl+1"                 "back_red"                   maxtool)
(def_key  "Ctrl+2"                 "back_white"                 maxtool)
(def_key  "Ctrl+3"                 "back_blue"                  maxtool)
(def_key  "Ctrl+4"                 "back_darkblue"              maxtool)
(def_key  "Ctrl+5"                 "back_green"                 maxtool)

(def_key  "Alt+A"                  "axis_toggle"                maxtool)
(def_key  "Alt+C"                  "recenter"                   maxtool)

(def_key  "Del"                    "delete_sel"                 maxtool)
(def_key  "Escape"                 "select_none"                maxtool)


(def_key  "Alt+`"                  "read-eval"                  maxtool)
(def_key  "Alt+;"                  "test"                       maxtool)

(def_key  "Ctrl+Y"                 "translate_point"            maxtool)
(def_key  "Space"                  "navigate"                   maxtool)
(def_key  "Ctrl+Space"             "pan"                        maxtool)

(def_key  "Alt+M"                  "frame_rewind"               maxtool)
(def_key  "Alt+,"                  "frame_back"                 maxtool)
(def_key  "Alt+."                  "frame_advance"              maxtool)
(def_key  "Alt+/"                  "toggle_animation"           maxtool)
(def_key  "Alt+U"                  "update_textures"            maxtool)
#(def_key  "Ctrl+U"                 "update_all_textures"        maxtool)
