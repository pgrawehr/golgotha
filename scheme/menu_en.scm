;; Editor menu declaration for english.

;; Translation instructions :
;; Translate File/Edit/View/Window/Map/Terrain, etc.
;; Translate the first string (everything in quotes), do not translate the second string.
;; Do not modify the third string (if any) either.
;; 
;; How it works: A submenu will be added with (add_sub_menu name <entries>)
;; where each entry is either a dash ("-"), which corresponds to a
;; separator line or a list (display_name command active_command).
;; The display name is a string which will be shown to the user.  
;; The command is the lisp-command that will be executed when the
;; entry is selected (The xxx/xxx kind of strings are all declared in the
;; editor itself). The third part is optional and might be used to name
;; a function that will be executed each time the menu is shown to 
;; enable or disable the entry. The has_map shown here returns false
;; if no map is loaded (and therefore most editor commands can't be used)

;;            Translate                 Don't Translate
;;            this colum                this colum
;----------------------------------------------------------------
(add_sub_menu "File"
              ("New..."                 "File/New")
              -
              ("Save"                   "File/Save" "has_map")
              ("Save As..."             "File/Save As" "has_map")
              ("Open..."                "File/Open")
              -
              ("Load DLL..."            "File/Open DLL")
              ("Execute Lisp-Code..."   "File/Open Anim")
              ("Preferences..."         "global_options")
              -
              ("Exit"                   "File/Exit"))


(add_sub_menu "Edit"
              ("Undo"                   "Edit/Undo" "has_map")
              ("Redo"                   "Edit/Redo" "has_map")
              ("Enable/Disable Undo"    "set_undo")
              -
;              ("Cut"                    "Edit/Cut")
;              ("Copy"                   "Edit/Copy")
;              ("Paste"                  "Edit/Paste")
              ("Toggle Menu"            "Edit/Toggle Menu")
              -
              ("No Snap"                "Edit/No Snap")
              ("Snap Cell Center"       "Edit/Snap Cell Center")
              ("Snap Cell Origin"       "Edit/Snap Cell Origin"))


(add_sub_menu "View"
              ("Wireframe"              "View/Wireframe")
              ("Textured"               "View/Textured")
              ("Solid Colors"           "View/Solid")
              ("Toggle Texture Loading" "View/Toggle Texture Loading")
              -
              ("1 View"                 "View/1 View")
              ("4 Views"                "View/4 Views"))

(add_sub_menu "Tools"
              ("Cutscenes"              "Tools/Scenes" "has_map")
              ("Radar"                  "Tools/Radar" "has_map")
              ("Profile"                "Tools/Profile")
              ("Lisp console"           "Tools/Lisp Interaction")
              ("Debug"                  "Tools/Debug")
              ("AI"                     "Tools/AI" "has_map")
              -
              ("Object list"            "Tools/Objects" "has_map")
              ("Tile list"              "Tools/Tiles" "has_map"))


(add_sub_menu "Map"
              ("Simulate Tick"          "Map/Simulate Tick" "has_map")
              ("Resize"                 "Map/Resize" "has_map")
              ("Rotate 90"              "Map/Rotate 90" "has_map")
              ("Change Sky"             "Map/Change Sky" "has_map")
              ("SCM File"               "Map/SCM File" "has_map")
              -
              ("Reload Map"             "reload_level")
              -
              ("Fog All"                "fog_map" "has_map")
              ("UnFog All"              "unfog_map" "has_map")
              -
              ("Dump Map (debug)"       "dump_level")
              ("Recalculate"            "Map/Recalculate" "has_map")
              ("Delete Selected"        "Map/Delete Selected" "has_map")
              -
              ("Edit Map Vars"          "edit_level_vars" "has_map")
              ("Add Cloud Shadow"       "add_cloud_shadow" "has_map")
              ("Remove Cloud Shadow"    "remove_cloud_shadow" "has_map")
              )


(add_sub_menu "Objects"
              ("Select Game Pieces"     "Objects/Select Game Pieces" "has_map")
              ("Select Similar"         "Objects/Select Similar" "has_map")
              ("Floor Selected"         "Map/Floor Selected" "has_map")
              ("Ceil Selected"          "Map/Ceil Selected" "has_map")
              ("Drop Selected"          "Objects/Drop Selected" "has_map")
              ("Full Health"            "full_health" "has_map")
              ("Set Health"             "set_health" "has_map")
              -
              ("Select Rest of Path"    "select_restof_path" "has_map")
              ("Join Selected"          "join_path_ends" "has_map")
              ("UnJoin Selected"        "unjoin_path_ends" "has_map")
              ("Insert Path Node"       "insert_path_object" "has_map") 
              ("Alle Wege loeschen"     "remove_all_paths" "has_map")
              ("Knoten und Wege laden"  "load_from_transims")
              -
              ("Toggle Path Display"    "toggle_show_list")
              ("Place Camera"           "place_camera" "has_map")
              )


(add_sub_menu "Teams"
              ("No Tinting"             "tint_none")
              ("Tint team polys"        "tint_polys")
              ("Tint all polys"         "tint_all")
              -
              ("Nuetral Team"           "team_0")
              ("Green Team   (allies)"  "team_1")
              ("Red Team     (axis)"    "team_2")
              ("Blue Team    (axis)"    "team_3")
              ("Yellow Team  (axis)"    "team_4")
              -
              ("Edit Nuetral Team"      "edit_team_0")
              ("Edit Green"             "edit_team_1")
              ("Edit Red"               "edit_team_2")
              ("Edit Blue"              "edit_team_3")
              ("Edit Yellow"            "edit_team_4")
              )


(add_sub_menu "Terrain"
              ("Select All"             "Terrain/Select All" "has_map")
              ("Flatten Selected"       "Terrain/Flatten Selected" "has_map")
              ("Smooth Selected"        "Terrain/Smooth Selected" "has_map")
                                                      
              ("Add Noise to Selected"  "Terrain/Add Noise to Selected" "has_map")
;              ("Load Heights from map"   "Terrain/Load Heights from map" "has_map")
              -
              ("Load Image Heightmap"   "Terrain/Load Image Heightmap")
              ("Save Image Heightmap"   "Terrain/Save Image Heightmap"))

(add_sub_menu "Sound"
              ("Pick Song"              "edit_music")
              ("Make Sfx List"          "list_sfx")
              -
              ("Music Volume Up"        "music_up")
              ("Music Volume Down"      "music_down"))

              
              

;; when adding editor functionality, the follow are available through the li system

;; (editor_undo int) 
;;; saves off undo information about sections of the map you are about to change
;;; if the sections are omitted, then the entire map is saved


;; (redraw_all)
;;; redraws all windows

;; (redraw)
;; redraws the current active window

;; (editor_changed)  
;;; if you changed the map and want the user to save before quiting

