

;; Translation instructions :
;; Translate File/Edit/View/Window/Map/Terrain, etc.
;; Translate the first string (everything in quotes), do not translate the second string
;; pair

;;            Translate                 Don't Translate
;;            this colum                this colum
;----------------------------------------------------------------
(add_sub_menu "Datei"
              ("Neu..."                    "File/New")
              -
              ("Speichern"              "File/Save" "has_map")
              ("Speichern unter..."     "File/Save As" "has_map")
              ("Oeffnen..."                   "File/Open")
              -
              ("Lade DLL..."               "File/Open DLL")
              ("Lisp-Code ausfuehren..."    "File/Open Anim")
              ("Einstellungen..."           "global_options")
              -
              ("Beenden"                   "File/Exit"))


(add_sub_menu "Bearbeiten"
              ("Rueckgaengig"                   "Edit/Undo" "has_map")
              ("Wiederholen"                   "Edit/Redo" "has_map")
              ("Rueckgangig aktiv/inaktiv"     "set_undo")
              -
;              ("Ausschneiden"                    "Edit/Cut")
;              ("Kopieren"                   "Edit/Copy")
;              ("Einfuegen"                  "Edit/Paste")
              ("Menu anzeigen/verbergen"            "Edit/Toggle Menu")
              -
              ("Nicht einrasten"                "Edit/No Snap" "has_map")
              ("Rasten an Zellenmitte"       "Edit/Snap Cell Center" "has_map")
              ("Rasten an Zellenursprung"       "Edit/Snap Cell Origin" "has_map"))


(add_sub_menu "Ansicht"
              ("Drahtgitter"              "View/Wireframe")
              ("Texturiert"               "View/Textured")
              ("Einfarbige Flaechen"           "View/Solid")
              ("Texturen laden ein/aus" "View/Toggle Texture Loading")
              -
              ("1 Fenster"                 "View/1 View")
              ("4 Fenster"                "View/4 Views"))

(add_sub_menu "Extras"
              ("Zwischensequenzen"                 "Tools/Scenes" "has_map")
              ("Radar"                  "Tools/Radar" "has_map")
              ("Profilanalyse"                "Tools/Profile")
	          ("Lisp Interpreter"       "Tools/Lisp Interaction")
              ("Debug"                  "Tools/Debug")
              ("AI"                     "Tools/AI" "has_map")
              -
              ("Objekte"                "Tools/Objects" "has_map")
              ("Zellentexturen"         "Tools/Tiles" "has_map"))

(add_sub_menu "Karte"
              ("Simuliere Zeitschritt"          "Map/Simulate Tick" "has_map")
              ("Groesse aendern"                 "Map/Resize" "has_map")
              ("90 Grad rotieren"              "Map/Rotate 90" "has_map")
              ("Himmel aendern"             "Map/Change Sky" "has_map")
              ("SCM-Datei"               "Map/SCM File" "has_map")
              -
              ("Karte neu laden"             "reload_level")
              ("LOD Textur neu rechnen"   "update_lod_texture" "has_map")
              -
              ("Mehr Nebel"                "fog_map")
              ("Weniger Nebel"              "unfog_map")
              -
              ("Drucke Karte (debug)"       "dump_level")
              ("Neu berechnen"            "Map/Recalculate")
              ("Ausgewaehltes entfernen"        "Map/Delete Selected" "has_map")
              -
              ("Kartenparameter bearbeiten"          "edit_level_vars")
              ("Wolkenschatten einfuegen"       "add_cloud_shadow")
              ("Wolkenschatten entfernen"    "remove_cloud_shadow")
              )

(add_sub_menu "Objekte"
              ("Spielobjekte waehlen"     "Objects/Select Game Pieces" "has_map")
              ("Aehnliche waehlen"         "Objects/Select Similar" "has_map")
              ("Boden waehlen"         "Map/Floor Selected" "has_map")
              ("Himmel waehlen"          "Map/Ceil Selected" "has_map")
              ("Gewaehltes entfernen"          "Objects/Drop Selected" "has_map")
              ("Gesundheit wiederherstellen"            "full_health" "has_map")
              ("Gesundheit setzen"             "set_health" "has_map")
              -
              ("Rest des Weges waehlen"    "select_restof_path" "has_map")
              ("Wege zusammenfuegen"          "join_path_ends" "has_map")
              ("Wege trennen"        "unjoin_path_ends" "has_map")
              ("Wegpunkt einfuegen"       "insert_path_object" "has_map") 
              ("Alle Wege loeschen"       "remove_all_paths" "has_map")
              ("Knoten und Wege laden"    "load_from_transims")
              -
              ("Wegpunkte ein/ausblenden"    "toggle_show_list")
              ("Kamera platzieren"           "place_camera" "has_map")
              )


(add_sub_menu "Teams"
              ("No Tinting"             "tint_none")
              ("Tint team polys"        "tint_polys")
              ("Tint all polys"         "tint_all")
              -
              ("Neutral Team"           "team_0")
              ("Green Team   (allies)"  "team_1")
              ("Red Team     (axis)"    "team_2")
              ("Blue Team    (axis)"    "team_3")
              ("Yellow Team  (axis)"    "team_4")
              -
              ("Edit Neutral Team"      "edit_team_0")
              ("Edit Green"             "edit_team_1")
              ("Edit Red"               "edit_team_2")
              ("Edit Blue"              "edit_team_3")
              ("Edit Yellow"            "edit_team_4")
              )


(add_sub_menu "Land"
              ("Alles waelen"             "Terrain/Select All" "has_map")
              ("Gewaehltes abflachen"       "Terrain/Flatten Selected" "has_map")
              ("Gewaehltes abrunden"        "Terrain/Smooth Selected" "has_map")
                                                      
              ("Gewaehltes kantiger machen"  "Terrain/Add Noise to Selected" "has_map")
;              ("Load Heights from map"   "Terrain/Load Heights from map" "has_map")
              -
              ("Load Image Heightmap"   "Terrain/Load Image Heightmap")
              ("Save Image Heightmap"   "Terrain/Save Image Heightmap"))

(add_sub_menu "Sound"
              ("Pick Song"              "edit_music")
              ("Make Sfx List"          "list_sfx")
              -
              ("Musiklautstaerke erhoehen"        "music_up")
              ("Musiklautstaerke reduzieren"      "music_down"))

              
              

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

