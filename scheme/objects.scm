; This file contains lisp object definitions. They can be changed at runtime
; Warning: Cannot def_object before models are registered

;The object_type_vars and the map_piece_type vars are newer directly instantiated
;since there are no objects of this actual type. 
; The actual references are copied in when instantiating (in case of lisp-objects only, others
; don't make sense, it seems)
;g1_ref as parameter bellow means that a li_g1_ref object with the global id of the 
; class is passed. The this-reference can therefore be obtained by calling li_g1_ref::get(li_first(o,env),env)->value()
; from within c++ code. 
;The signature of this function is the same wheter you implement one of these or 
; you want to call the parent.
(def_class object_type_vars nil
  ;g1_ref
  (think nil)
  (think_parent nil)
  ;g1_ref, other.g1_ref, hitpoints, damage_dir*
  (damage nil)
  (damage_parent nil)
  ;g1_ref (should return non-0 upon success)
  (occupy_location nil)
  (occupy_location_parent nil)
  ;g1_ref
  (unoccupy_location nil)
  (unoccupy_location_parent nil)
  ;g1_ref (unimplemented, needs complicated data structures to proceed)
  (draw nil)
  (draw_parent nil)
  ;g1_ref, other.g1_ref, hitpoints
  (notify_damage nil)
  (notify_damage_parent nil)
  ;First param: g1_ref, second param: symbol of message, third param: list of parameters
  (message nil)
  (message_parent nil)
  ;g1_ref, x, y, (using_path is always 0)
  (deploy_to nil)
  (deploy_to_parent nil)
  ;g1_ref, new_player_num
  (change_player_num nil)
  (change_player_num_parent nil)
  ;unimplemented
  (enter_range nil)
  (enter_range_parent nil)
  ;g1_ref, this_flags, this_player_num, other_g1_ref, other_flags, other_player_num
  ;The flags are the known combinations of g1_object_class::flags values.
  ;Here, mainly the HIT_XXX and the GROUND, UNDERWATER, AERIAL flags are interesting
  ;Should return t if the given object can be attacked (doesn't need to check the
  ;distance to it, only wheter it's of the right team and we can shoot at it).
  ;If calling the parent function, only other_g1_ref is actually examined. 
  (can_attack nil)
  (can_attack_parent nil)
)
; Releated global functions:
; (sync g1_ref) - required in special cases to get the position in the c++ fields right before 
;                 calling a system function (like occupy_location)
; (request_think g1_ref) 
;		- Request a rethink (if not present in think, the object thinks only once, unless
;                 some notification wakes it up.
;                 Note: The implementation of this function allows several calls to rethink without 
;                 performance degradation.  
; (request_remove g1_ref) 
;               - Get deleted from the map.
; (fire g1_ref target_ref pos dir weapon)
;               - Fire a bullet
;                 g1_ref: self reference
;                 target: reference to target. Use nil to fire at the default target. If no 
;		    default is known, will fire just straight away. Consider that many weapons
;		    don't need to know the target (because they fly straight ahead anyway)
;                 pos: Local(!) position of where the bullet comes out (i.e the end of the cannon) *
;                 dir: Direction of where the bullet goes in local coordinates* (i.e 1 0 0 to fire
;                   straight ahead)
;		  weapon: Name of the weapon to use (nil to use default, if any)
;		  Returns: t if fired, nil if not (one reason might be that the fire rate would be 
;                   to high)
; (decrease_fire_delay g1_ref)
;		- Call this if you don't use the g1_map_piece_class::think() method before firing. 
; (find_targets g1_ref)
;               - Look for targets (in the range defined by the object defaults)
;		  Returns: the reference to the target.
; (max_health g1_ref) 
;               - Returns the configured maximum health of this object.
; (object_flags g1_ref) 
;               - returns the flag word of the given object.
; (object_owner g1_ref)
;               - returns the player number of the given object.
; (object_pos g1_ref)
;               - returns the position vector of the object
; (object_orientation g1_ref)
;               - returns a (theta, pitch, roll) vector of the given object. 
; (objects_in_range g1_ref range [object_flags_mask] [type_flags_mask])
;               - Returns a list of objects that meet the given conditions and
;                 are inside a circle around the given object.
;                 The flags default to anything. 
;                 At most 1024 objects are returned.
;                 Hint: The current object may also be in the list.
; (get_mini_object_pos g1_ref num_of_mini_object)
;               - Returns a list(!) with the following properties of the mini:
;                 x, y, h, offset*, rotation* (all relative to this object)
; (set_mini_object_pos g1_ref num_of_mini_object new_data)
;               - Sets the new position of the mini-object. 
;                 Takes the same list that the get method returns as third argument.
;                 Hint: Use (setf (nth x list) newvalue) to update a single element of
;                 a list. 
; (set_mini_object_model ...) (get_mini_object_model) 
;               - same as above, but with animation number, frame number, model number
;                 and lod model number.
; (create_object name location* orientation*)
;               - Creates an instance of object "name" at location "location" with 
;                 orientation "orientation" (theta, pitch, roll). 
;                 The object is inserted into the map (using occupy_location) but
;                 request_think has not been called yet. 
;                 Returns the object reference.
;                 Hint: Objects that require special setup functions called might not
;                 properly work like this (unless you can access the other objects lisp
;                 variables) 
; (local_player) 
;               - Returns the number of the local player.
; (player_variables player_num)
;               - Returns the player variable class for the given player.
;                 This class is of type player_vars (start.scm). 
;
; Hint: The best way of debugging lisp code is currently by using print in appropriate
; places. However, this can drastically reduce performance!                 
;
; (*) a vector (internal type li_vect, external name "vector")

(def_class map_piece_type_vars object_type_vars
)

(def_class cobra_tank_type_vars map_piece_type_vars
  (smallrange 7.0)
  (smallweapon 'chain_gun)
)

(def_class cobra_lisp1_type_vars map_piece_type_vars
)

(def_object armor 
  (model_name "shield_building")
  (mini_object (offset 0 0 0) (model_name "shield_building_shield"))
  (type_flags editor_selectable) ;;type flags (for the defining class)
  (object_flags targetable ground blocking) ;object flags (for the object itself)
  (occupy_location armor_building_occupy_location) ;a function
  (unoccupy_location armor_building_unoccupy_location) ; some more functions 
  (think armor_building_think) ;implementation is in c here.
  (change_player_num armor_building_change_player_num)
  )

(defun easy_cobra_thinker (glob_id)
  (print glob_id)
  (print "xpos")
  (print xpos)
  (print "ypos")
  (print ypos)
  ;(print this) ;this has only a value if a variable class for the object exists
  		; (in this case cobra_lisp1_vars)
  ;(print this_type)
  ;(print " think_parent is")
  ;(print (member this_type 'think_parent))
  ((member this_type 'think_parent) glob_id)
  (if (< health (max_health glob_id)) (setf health (+ health 1)))
  (fire glob_id nil (new vector 0 0 0.5) (new vector 1 0 0) nil)
  (request_think glob_id)
)

(setf minuspi (- 0.0 pi))
; rotates the angle "from" to "to" by at most "speed"
(defun rotate (from to speed)
  (let  ((dangle (- to from)))
  	(if (> dangle pi) (setf dangle (- dangle 2pi)))
  	(if (< dangle minuspi) (setf dangle (+ dangle 2pi)))
  	(if (> dangle speed) (setf dangle speed))
  	(if (< dangle (- 0.0 speed)) (setf dangle (- 0.0 speed)))
  	(+ from dangle) ; the last statement calculates the return value
  )
)

(defun norm_angle (angle)
	(let* ( (tangle angle) )
	(if (> angle 2pi)
		(setf tangle (- angle 2pi))
	)
	(if (< angle 0.0)
		(setf tangle (+ angle 2pi))
	)
	;(print "normalized angle" tangle)
	tangle
	)
)	
  
(def_class cobra_tank_vars nil
   (smalltarget (new object_ref))
   (smallfire_delay 30)
   (experience 0)
)

(defun cobra_thinker (glob_id)
  ((member this_type 'think_parent) glob_id)
  (if (< health (max_health glob_id)) (setf health (+ health 1)))
  (if (not (equal attack_target null_object_ref))
     (progn (let* ((v (object_pos attack_target)) (tx (vector_element 'x v))
     		(ty (vector_element 'y v)) (xdiff) (ydiff) (dist) (xdiff2) (ydiff2)) 
     		(setf xdiff (- xpos tx))
     		(setf ydiff (- ypos ty))
     		(setf xdiff2 (* xdiff xdiff))
     		(setf ydiff2 (* ydiff ydiff))
     		(setf dist (sqrt (+ xdiff2 ydiff2)))
     		; If it's near enough fire with the chain gun at it
     		(if (and (< dist (member this_type 'smallrange) ) 
     		         (equal (member this 'smalltarget) null_object_ref)
     		         (not (equal (member this 'smalltarget) attack_target))
     		         )
     			(progn 
     			(setf (member this 'smalltarget) attack_target)
     			(setf attack_target null_object_ref)
     			)
     			;else
     			;if the direction of the main gun is ok, we can fire
     			(let ((angle (atan2 ydiff xdiff)) (mini_pos) (angle) (mini_rot_vect) (mini_pos2))
     			    ;(setf theta (rotate theta angle 0.2))
     			    ;(fire glob_id nil (new vector 0.5 0 0.1) (new vector 1 0 0) nil)
     			    (setf mini_pos (get_mini_object_pos glob_id 0))
     			    (setf angle (norm_angle (+ (- (atan2 ydiff xdiff) theta) pi)) )
     			    ;(setf angle (- pi theta))
     			    ;(print "bigbefore: " mini_pos angle)
     			    (setf mini_rot_vect (new vector (vector_element 'x (nth 4 mini_pos)) (vector_element 'y (nth 4 mini_pos)) 
     			  	(norm_angle (rotate (vector_element 'z (nth 4 mini_pos)) angle 0.2))))
     			    (setf (nth 4 mini_pos) 
     			  	mini_rot_vect)
     			    (setf mini_pos2 (get_mini_object_pos glob_id 1))
     			    (setf (nth 4 mini_pos2) mini_rot_vect)
     			    ;(print "bigafter: " mini_pos)
     			    (set_mini_object_pos glob_id 0 mini_pos)
     			    (set_mini_object_pos glob_id 1 mini_pos2) 
     			    (fire glob_id nil (new vector 0 0 0.2) 
     			      (new vector (cos (vector_element 'z (nth 4 mini_pos2))) 
     			      (sin (vector_element 'z (nth 4 mini_pos2))) 0) nil);
     			    
     			)
     		)
   	    ) ;let
     ) ;progn
  ); if
  (if (not (equal (member this 'smalltarget) null_object_ref))
     (progn (let ((v) (tx)
     		(ty) (xdiff) (ydiff) (xdiff2) (ydiff2) (dist)) 
     		(setf v (object_pos (member this 'smalltarget)))
     		(setf tx (vector_element 'x v))
     		(setf ty (vector_element 'y v))
     		(setf xdiff (- xpos tx))
     		(setf ydiff (- ypos ty))
     		(setf xdiff2 (* xdiff xdiff))
     		(setf ydiff2 (* ydiff ydiff))
     		(setf dist (sqrt (+ xdiff2 ydiff2 1)))
     		;if it goes beyond the inner range, stop firing
     		(if (> dist (member this_type 'smallrange) ) 
     			(progn 
     			(setf (member this 'smalltarget) null_object_ref)
     			)
     			;otherwise turn the top tower and fire
     			(let ((mini_pos) (angle))
     			  (setf mini_pos (get_mini_object_pos glob_id 2))
     			  (setf angle (norm_angle (- (+ (atan2 ydiff xdiff) pi) theta)) )
     			  ;(setf angle (- pi theta))
     			  ;(print "before: " mini_pos angle theta)
     			  (setf (nth 4 mini_pos) 
     			  	(new vector (vector_element 'x (nth 4 mini_pos)) (vector_element 'y (nth 4 mini_pos)) 
     			  	(norm_angle (rotate (vector_element 'z (nth 4 mini_pos)) angle 0.2))))
     			  ;(print "after: " mini_pos)
     			  (set_mini_object_pos glob_id 2 mini_pos) 
     			  (fire glob_id nil (new vector 0 0 0.3) 
     			    (new vector (cos (vector_element 'z (nth 4 mini_pos))) 
     			    (sin (vector_element 'z (nth 4 mini_pos))) 0) 'chain_gun);
     			    ;last vector is often (0 0 0) which doesn't make sense.
     			)
     		) ;if
     	   ) ;let
     ) ;progn
   
     )
  (request_think glob_id)
 )
 
(defun cobra_notify_damage (glob_id other_glob_id hp)
  (setf (member this 'experience) (+ (member this 'experience) hp))
  ((member this_type 'notify_damage_parent) glob_id other_glob_id hp)
)

  
; First try: The most trivial implementation of a new object.  
(def_movable_object cobra_lisp1
  (model_name "cobra")
  (type_flags editor_selectable movable)
  (object_flags targetable ground blocking dangerous selectable hit_ground hit_underwater)
  (think easy_cobra_thinker)
)

;Second try: The most advanced implementation of a new object.  
(def_movable_object cobra_tank
  (model_name "cobra_body")
  (mini_object (model_name "cobra_main_barrel") (offset 0 0 0.1))
  (mini_object (model_name "cobra_main_turret") (offset 0 0 0.1))
  (mini_object (model_name "cobra_mini_turret") (offset 0 0 0.17))
  (type_flags editor_selectable movable)
  (object_flags targetable ground blocking dangerous selectable hit_ground hit_aerial hit_underwater)
  (think cobra_thinker)
  (notify_damage cobra_notify_damage)
)

