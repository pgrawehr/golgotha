(setf normal_unlimited '(list_box normal unlimited))


(def_class player_vars nil
  (num_points        0)
  (num_stank_deaths  0)
  (num_stank_lives   3)

  (income_multipler  1)       ; computer players get an advantage
  (income_rate       0)
  (money             10000)

  (continue          0)       ; after stank dies
  (team_flag "team_flag")
  (ai        "human" '(list_box "ai_jim" "ai_builder" "ai_formation_jim" "ai_joe" "human" "ai_neutral"))
  (damage_multiplier 1.0)
)

(setf radar_darkness 1.0)   ; 0-1 scales the birghtness of the radar map
(setf editor_pan_accel 0.05)
(setf editor_rotate_accel 0.01)  ; in radians
(setf strategy_pan_accel 0.05)
(setf strategy_rotate_accel 0.01) ; not used since not possible in strategy mode



; **************************** Border frame config *******************************
(setf strategy_camera_border 7)


  
; defines which weapons go with which upgrade levels and the border image to use
; weapons are defined in scheme/balance.scm
(setf default_frame "bitmaps/stank/def_frame.jpg")
(setf upgrade_levels
;     '(("bitmaps/stank/frame_level_0.jpg" guided_missile heavy_rocket  bolt kevlar)))
     '(("bitmaps/stank/frame_level_0.jpg" b120mm heavy_rocket    chain_gun kevlar)
       ("bitmaps/stank/frame_level_1.jpg" acid   vortex_missile  chain_gun reactive)
       ("bitmaps/stank/frame_level_2.jpg" napalm nuke_missile    chain_gun titanium)))

(setf upgrade_colors 
      '(0xffffff     ; no stank - white
        0x005fff     ; cyan - level 0
        0xff00       ; green - level 1 (acid)
        0xff0000))    ; red  - level 2  (fire)

; locations relative to status bar top left graphic
;action_mode_locations
;although this might look strange, the filename is here used as an identifier
(setf bitmaps/stank/status_stank_level_0.jpg
      '((276   10)  ; lives
        (-10   500) ; money
        (92    34)  ; main
        (341   37)  ; missiles
        (369   6)   ; chain
        (92    6))) ; health
        
(setf bitmaps/stank/status_stank_level_1.jpg
      '((276   10)  ; lives
        (-10   500) ; money
        (92    34)  ; main
        (341   37)  ; missiles
        (369   6)   ; chain
        (92    6))) ; health

(setf bitmaps/stank/status_stank_level_2.jpg
      '((276   10)  ; lives
        (-10   500) ; money
        (92    34)  ; main
        (341   37)  ; missiles
        (369   6)   ; chain
        (92    6))) ; health
        
(setf bitmaps/stank/status_default.jpg
      '((276   10)  ; lives
        (276   30))) ; money

; locations relative to strategy-bar top left graphic
; only lives and money are really visible in strategy mode. 
(setf strategy_mode_locations
      '((7 172)     ; lives
        (99 126)    ; money
        (37 270)    ; main
        (100 299)   ; missiles
        (101 218)   ; chain
        (102 218))) ; health -was (37 218)


; *************************** Preferences ******************************
(setf team_tinting       nil)
(setf team_icons         nil)
(setf allow_follow_mode  nil)


;(set_default_ai "ai_builder")
(set_default_ai "ai_jim")


(setf watch_camera_accel 0.01)
(setf watch_camera_max_speed  0.3)
(setf watch_camera_turn_speed 0.04)

(setf font "bitmaps/golgotha_font.tga")


(def_class music_class nil
  (songs  "none" '(list_box 

                   ; ************ music list ***********
                   "music/splash_screen_opus_22khz.wav"

                   "music/Cairo_Egypt.wav"
                   "music/rome_italy_22khz.wav"
		   "music/finnland.wav"
		   "music/zurich_switzerland_22khz.wav"
		   "music/Al_Basrah_22khz.wav"
		   "music/Greece.wav"
		   "music/helsinki_sweden.wav"
		   "music/Jerusalem_Israel.wav"
		   "music/Munich_Germany.wav"
		   "music/nakhayb_iraq.wav"
		   "music/naples_italy.wav"


;                   "music/frankfurt_germany_22khz.wav"
                   "music/norway_nephelim_battle.wav"
                   "music/rome_italy_22khz.wav"
		   "music/snow_level_22khz.wav"
		   "music/turin_italy.wav"
;                   "music/roselyn_chapel_scotland_ii_22khz.wav"
;                   "music/naples_italy.wav"
;                   "music/turin_italy_22khz.wav"
                   "music/vienna_austria.wav"
                   ; ************************************
                   ))
  (volume 63))

(setf music (new music_class))

(setf enemy_buildable  '(peon_tank bomb_truck engineer))
;;(setf player_buildable '(peon_tank bomb_truck moneyplane stank engineer))
(setf player_buildable '(peon_tank bomb_truck moneyplane stank engineer electric_car 
  helicopter jet bomber trike tank_buster rocket_tank bridger cobra_tank))

(load "scheme/classes.scm")

;(set_default_level "/u/ddt/src/dlev/d.level")


(setf mouse_left_button "Main Gun")
(setf mouse_right_button "Chain Gun")




