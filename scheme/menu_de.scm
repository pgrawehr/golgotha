

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
              ("Speichern"              "File/Save")
              ("Speichern unter..."     "File/Save As")
              ("Oeffnen..."                   "File/Open")
              -
              ("Lade DLL..."               "File/Open DLL")
              ("Lisp-Code ausfuehren..."    "File/Open Anim")
              ("Einstellungen..."           "global_options")
              -
              ("Beenden"                   "File/Exit"))


(add_sub_menu "Bearbeiten"
              ("Rueckgaengig"                   "Edit/Undo")
              ("Wiederholen"                   "Edit/Redo")
              ("Rueckgangig aktiv/inaktiv"     "set_undo")
              -
;              ("Ausschneiden"                    "Edit/Cut")
;              ("Kopieren"                   "Edit/Copy")
;              ("Einfuegen"                  "Edit/Paste")
              ("Menu anzeigen/verbergen"            "Edit/Toggle Menu")
              -
              ("Nicht einrasten"                "Edit/No Snap")
              ("Rasten an Zellenmitte"       "Edit/Snap Cell Center")
              ("Rasten an Zellenursprung"       "Edit/Snap Cell Origin"))


(add_sub_menu "Ansicht"
              ("Drahtgitter"              "View/Wireframe")
              ("Texturiert"               "View/Textured")
              ("Einfarbige Flaechen"           "View/Solid")
              ("Texturen laden ein/aus" "View/Toggle Texture Loading")
              -
              ("1 Fenster"                 "View/1 View")
              ("4 Fenster"                "View/4 Views"))

(add_sub_menu "Extras"
              ("Zwischensequenzen"                 "Tools/Scenes")
              ("Radar"                  "Tools/Radar")
              ("Profilanalyse"                "Tools/Profile")
	      ("Lisp Interpreter"       "Tools/Lisp Interaction")
              ("Debug"                  "Tools/Debug")
              ("AI"                     "Tools/AI")
              -
              ("Objekte"                "Tools/Objects")
              ("Zellentexturen"         "Tools/Tiles"))

(add_sub_menu "Karte"
              ("Simuliere Zeitschritt"          "Map/Simulate Tick")
              ("Groesse aendern"                 "Map/Resize")
              ("90 Grad rotieren"              "Map/Rotate 90")
              ("Himmel aendern"             "Map/Change Sky")
              ("SCM-Datei"               "Map/SCM File")
              -
              ("Karte neu laden"             "reload_level")
              ("LOD Textur neu rechnen"   "update_lod_texture")
              -
              ("Mehr Nebel"                "fog_map")
              ("Weniger Nebel"              "unfog_map")
              -
              ("Drucke Karte (debug)"       "dump_level")
              ("Neu berechnen"            "Map/Recalculate")
              ("Ausgewaehltes entfernen"        "Map/Delete Selected")
              -
              ("Kartenparameter bearbeiten"          "edit_level_vars")
              ("Wolkenschatten einfuegen"       "add_cloud_shadow")
              ("Wolkenschatten entfernen"    "remove_cloud_shadow")
              )

(add_sub_menu "Objekte"
              ("Spielobjekte waehlen"     "Objects/Select Game Pieces")
              ("Aehnliche waehlen"         "Objects/Select Similar")
              ("Boden waehlen"         "Map/Floor Selected")
              ("Himmel waehlen"          "Map/Ceil Selected")
              ("Gewaehltes entfernen"          "Objects/Drop Selected")
              ("Gesundheit wiederherstellen"            "full_health")
              ("Gesundheit setzen"             "set_health")
              -
              ("Rest des Weges waehlen"    "select_restof_path")
              ("Wege zusammenfuegen"          "join_path_ends")
              ("Wege trennen"        "unjoin_path_ends")
              ("Wegpunkt einfuegen"       "insert_path_object") 
              ("Alle Wege loeschen"       "remove_all_paths")
              ("Knoten und Wege laden"    "load_from_transims")
              -
              ("Wegpunkte ein/ausblenden"    "toggle_show_list")
              ("Kamera platzieren"           "place_camera")
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
              ("Alles waelen"             "Terrain/Select All")
              ("Gewaehltes abflachen"       "Terrain/Flatten Selected")
              ("Gewaehltes abrunden"        "Terrain/Smooth Selected")
                                                      
              ("Gewaehltes kantiger machen"  "Terrain/Add Noise to Selected")
;              ("Load Heights from map"   "Terrain/Load Heights from map")
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

