(def_class damager_vars nil
  (attach_pos  (new vector 0 0 0))
  (smoke_type  'acid '(list_box acid napalm))
  (ticks_to_think  1) ; after visible, think for 2 seconds
  (damage_per_tick 0)
  (person_being_damaged (new object_ref))
  (person_giving_damage (new object_ref))
)

(def_class armor_vars nil
  (damage_multiplier_fraction 0.2)
  (spin_speed 1.0)
  )



(def_class bird_vars nil
  (flapping 'yes '(list_box yes no))
  (ticks_to_think 10)
  (flap_theta 0.1)
  (dest_x     10.0)
  (dest_y     10.0)
  )
  
(def_class gunport_vars nil
  (explode_health -20)
  (fire_delay 20)
  (total_burst 1)
  (burst_delay 5)
  (fire_type 'bolt '(list_box bolt b120mm acid napalm chaingun beam plasma heavy_rocket vortex_missile nuke_missile))
  )
  
(def_class bird_eagle_vars bird_vars
  (pad       0)
  )

(def_class goal_vars nil
  (trigger_objects (new object_ref_list))
  (think_delay 10)
  (ticks_to_think 2)
  (range 5.0)
  )

(def_class takeover_pad_vars nil
  (trigger_objects  (new object_ref_list))
  (trigger_message  'on)
  (takeover_objects (new object_ref_list))
  (turret           (new object_ref))
  )

(def_class bank_vars nil
  (can_build  '(moneyplane))
  (ticks_till_next_deploy   0)
  (reset_time_in_ticks      50)
  (path_color               0)
  (start                    (new object_ref))
  (moneyplanes     1)
  (time            0)
  (income_time     500)
  (crates          0)
  (crate_value     2000)
  (crate_capacity  10)
  (crate           (new object_ref)))

(def_class moneycrate_vars nil
  (vspeed          0.0)
  (crate_value     100))

(def_class moneyplane_vars nil
  (crate                    (new object_ref))
  (mode            0))

(def_class lawfirm_vars nil
                (income_rate        3000)    ; how much they are sueing for
                (commision           0.5)    ; lawyers get %50 of income from other players
                (counter             0)      ; ticks left till current law-suite   
                (reset_time          150)     ; ticks till next law-suite
                )
             
; This is an example of a type class declaration. It must end with "_type_vars" and is automatically
; bound to the type with the given name.
(def_class chain_gun_type_vars nil
  (texture_name              "blue_flare")
  (texture_size              0.2))


(def_class director_vars nil
  ; use an objects as the location to send people to.  Randly picks one
  (deploy_to            (new object_ref_list)) 

  (range_when_deployed  2.0)                ; when an object is this close, send it to destination
  (current_state       'on                  ; when off it will not deploy
                       '(list_box on off))
  (on_message          'on)                 ; when we get this message we turn on
  (off_message         'off)
  (who_to_send         'team '(list_box team enemy anyone))

  
  ; list of nearby objects we've already sent so we don't send them again,
  ; or if we turned off, then a list of objects to send when we turn on
  (nearby_objects              (new object_ref_list))

  ; objects should be sent next tick.
  (objects_to_send_next_tick    (new object_ref_list))
)

(def_class trigger_vars nil
  (range_when_activated   3.0     '(list_box 1.0 2.0 3.0 4.0 5.0 6.0))
  ;;(range_when_deactivated 4.0     '(list_box 1.0 2.0 3.0 4.0 5.0 6.0))
  (team_can_trigger        'everybody '(list_box everybody friend enemy))
  (units_can_trigger        'anyone '(list_box anyone stank
  					engineer bomb_truck ))
  (objects_to_trigger    (new object_ref_list))
  (send_on_enter       'enter) ;; use none to send no message
  (send_on_leave       'leave) ;; use none to send no message
  (objects_in_range      (new object_ref_list))
  ;(current_state         'on  '(list_box on off))
  (check_time            10) ;; check interval
  (check_cur_time        0)  ;; current interval value.
)

(def_class switch_vars trigger_vars
  (current_state        'on '(list_box on off))
  (sendon_on           'on)
  (sendon_off          'off)
)

(def_class toggable_switch_vars switch_vars
;; for this type, current_state determines wheter switching is
;; possible at all.
  (toggle_state        'off '(list_box on off))
)

(def_class extended_trigger trigger_vars
)

(def_class garage_vars nil
  (can_build  '(electric_car peon_tank engineer 
                trike tank_buster rocket_tank bomb_truck bridger cobra_tank))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (path_color               0x7f7f7f)
  (selected_path_color      0xffffff)
  (start                    (new object_ref)))

(def_class airbase_vars nil
  (can_build  '(helicopter jet bomber))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (traffic_height           1.5)   ; height of all flying objects objects 
  (path_color               0x7f007f)
  (selected_path_color      0xff00ff)
  (start                    (new object_ref)))

(def_class mainbasepad_vars nil
  (can_build  '(stank))
  (ticks_till_next_deploy    0)
  (reset_time_in_ticks      10)
  (path_color               0x7f7f7f)
  (selected_path_color      0xffffff)
  (start                    (new object_ref)))

(def_class path_object_vars nil
  (warning_level      0   '(list_box 0 1 2 3 4 5 6 7 8 9))
  (bridgeable_spot    'no '(list_box no yes already_attached))    
  (active   'on       '(list_box on off))     ; is this node turned on?
  (controlled_objects (new object_ref_list))) ; objects taken over by engineers & other specials
  
(def_class road_object_vars path_object_vars
;; just inherit, but add nothing new
)

(def_class secret_hider_vars nil
  (grab_height    'no '(list_box no yes))
  (grab_textures  'no '(list_box no yes)))

(def_class field_camera_vars nil
  (name  "unknown_camera"))

(def_class camera_params nil
  (x 0.0)
  (y 0.0)
  (z 0.0)
  (rotatex 0.0)
  (rotatey 0.0)
  (rotatez 0.0)
  (scalex 1.0)
  (scaley 1.0)
  (scalez 1.0)
  )

(def_class bridger_vars nil
  (marker_attached_to (new object_ref))
)

(def_class guided_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class buster_rocket_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class heavy_rocket_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class vortex_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))

(def_class nuke_missile_vars nil
  (smoke_trail    (new object_ref))
  (who_fired_me   (new object_ref))
  (track_object   (new object_ref))
  (fuel           0.0)
  (velocity       (new vector)))


(def_class crate_vars nil
  (type   'money   '(list_box health bullet missile chain_gun money))
  (amount 'small    '(list_box small large))
  (yvel    0.0)
  (ticks_left -1))
  
(def_class supergun_vars nil
  (active 'yes '(list_box yes no))
  (user_selectable 'yes '(list_box yes no))
)

(def_class convoy_vars nil
  (units (new object_ref_list))
  (user_formation 'none '(list_box none arrow box rectangle line row))
  
)


(def_class cloud_color nil
  (red   1.0)
  (green 1.0)
  (blue  1.0)
  (alpha 1.0)
  (texture_name "cloud2")
)

(def_class level_vars nil
;  (bottom_cloud_layer    (new cloud_color 0.8 0.6 0.4 1.0))
;  (top_cloud_layer (new cloud_color 1.0 0.8 0.6 0.75)) 

  (bottom_cloud_layer    (new cloud_color 1.0 1.0 1.0 1.0))
  (top_cloud_layer (new cloud_color 0.8 0.8 0.8 0.75)) 
  (path_binding 'default_bound 
  '(list_box force_bound default_bound force_unbound default_unbound))
  (path_finding 'any 
  '(list_box any path_only breadth_solver astar_solver sight_only graph_solver))
  (command_all t)
  (traffic_sim nil)
;;  (traffic_sim_keepasp t);;Keep the aspect ratio of the nodes in the map?
  (traffic_sim_links "")
  (traffic_sim_nodes "")
  (traffic_sim_vehicles "");; will sometime be the output
  (traffic_sim_acts "")
  (model_scaling 0.1)
  ;;(world_scaling 1.0)  ;; set the average of scaling in x and y direction
  ;;should be used in calculating speeds, link lengths and other things
  ;;changed to a global variable since it is needed before the map vars are loaded
)

;;Information: The day has 60*60*24=86400 seconds.
;;since a second has 10 ticks, the whole day takes 864000 ticks. 
;;if you increase the advance_ticks value, day and night change faster
;;i.e. 10 will make the day only 2.4 hours instead of 24h.
(def_class day_night_change_vars nil
  (active nil)
  (daysky (new sky))
  (nightsky (new sky))
  (advance_ticks 1) ;; Default increment per tick (= real time)
  ;; time==0 means dawn
  (current_time 0);; start simulation at dawn
  (sync_with_realtime nil)
  (darkness_factor 1.0) ;; ratio from maximum to minimum brightness
  ;; the sky is brightest at noon (with daysky) and at midnight (nightsky)
  
)


