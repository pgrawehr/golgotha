(add_sub_menu "File"
              ("New"         "create_plane")
              ("Open..."     "open_model")
              ("Save"        "save_model")
              ("Save As..."  "saveas_model")
              -
              ("Preferences..." "global_options")
              ("Run IVCON..." "Run_IVCON")
              -
              ("Quit"     "File/Exit"))


(add_sub_menu "Edit"
              ("Undo"                  "undo")
              ("Redo"                  "redo")
              -
              ("Recenter View"         "recenter")
              ("Navigate Mode"         "navigate")
              ("Pan Mode"              "pan")
              ("Translate Mode"        "translate_point")
              -
              ("Update Model Textures" "update_textures")
;;              ("Update All Textures"   "update_all_textures")
	      ("Reload model-textures" "reload_max_textures")
	      ("Reload main textures"  "reload_main_textures")
	      ("Close Maxtool"         "Maxtool/Toggle Menu")
              )


(add_sub_menu "Selection"
              ("Select None"           "select_none")
              ("Select All"            "select_all")
              ("Select Similar"        "select_similar")
              ("Drag Select"           "drag_select")
              -
              ("Select Faces"          "navigate")
              ("Select Points"         "translate_point")
              ("Pan Mode"	       "pan")
              ("Recalculate normals"   "m1_recalc")
	      -
	      ("Join Texture Coords"   "join_coords")
	      ("Add a Side"	       "add_quad")
              ("Add a Vertex"	       "add_vertex")
              ("Delete Vertex"         "delete_vertex")
              ("Edit Mount-Points"     "m1_edit_mount_points")
              ;;This moves the entire object relative to the origin. 
              ("Resize object"         "m1_scale_object")
              ("Move object"           "m1_move_object")
              )


(add_sub_menu "Animation"
              ("Advance one frame"  "frame_advance")
              ("Back one frame"     "frame_back")
              ("Rewind to first frame"   "frame_rewind")
              ("Add a frame"      "frame_add")
              ("Remove current frame"   "frame_remove")
              ("Run current animation"  "toggle_animation")
              ("Get info on object" "max_get_object_info")
              -
              ("Advance one animation" "animation_advance")
              ("Back one animation" "animation_back")
              ("Rewind to default animation" "animation_rewind")
              ("Add an animation" "add_animation")
              ("Delete current animation" "delete_animation")
              ("Rename current animation" "rename_animation")
              )


(add_sub_menu "Faces"
              ("Add Face"              "add_quad")
              ("Delete Faces"          "delete_sel")
              -
              ("Swap Face Numbers"     "swap_polynums")
              ("Edit Special"          "edit_special")
              ("Flip Normal"           "flip_normal_sel")
              ("Draw bothsided"        "toggle_bothsided_sel")
              ("Default Coordinates"   "default_coords")
              ("Tint Selected"         "tint_sel")
              ("Untint Selected"       "untint_sel")
              -
              ("Distribute Texture"    "distribute_sel")
              ("Rotate Textures"       "rotate_texture_sel")
              ("Reverse Textures"      "reverse_texture_sel")
              ("Animate Textures"      "animate_texture_sel")
              ("Pan Textures"          "pan_texture_sel")
              -
              ("Dump Polys to SCM"     "dump_polys")
              )
              
              

(add_sub_menu "Options"
              ("No tint"         "no_tint")
              ("Tint Team 1"     "m1_team_1")
              ("Tint Team 2"     "m1_team_2")
              ("Tint Team 3"     "m1_team_3")
              ("Tint Team 4"     "m1_team_4")
              -
              ("Shading"         "toggle_shading")
              -
              ("Toggle Axis"     "axis_toggle")
              ("Wire Frame"      "wireframe")
              -
              ("Toggle Texture Names" "toggle_names")
              ("Toggle Face Numbers" "toggle_numbers")
              ("Toggle Vertex Numbers" "toggle_vnumbers")	      
              ("Show Coordinate Origin" "toggle_origin")
              ("Toggle Orphaning Vertices" "toggle_orphans")
              ("Toggle Octree cubes" "toggle_octree")
              -
              ("Build octree" "build_octree")
              ("Remove octree" "remove_octree")
              )

(add_sub_menu "Background"
              ("Black"           "back_black")
              ("Red"             "back_red")
              ("White"           "back_white")
              ("Blue"            "back_blue")
              ("Dark Blue"       "back_darkblue")
              ("Green"           "back_green")
              )
