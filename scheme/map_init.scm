;;default name of this file is "scheme/map_init.scm"

;;This file is executed each time a map is reloaded 
;;(actually just before the <levelname>.scm file is executed)

;;it is used to set defaults that might (but not need to) be overwritten
;;in the level.scm file.

; Hint: The loose function is always evaluated before the winning
; function, so if i.e the player destroys the designated target but dies
; from the explosion himself, he has lost.
; Return t if the local player has lost.
(defun loose_function (tick local_player) nil)
; Return t if the local player has won the level. 
; Return a string with the next level if you want.
; Hint: The following doesn't work, since the first string in the body
; will be interpreted as documentation value.
; (defun winning_function (tick local_player) "the_next.level")
(defun winning_function (tick local_player) nil)

(map_height_restore)
(setf world_scaling 1.0)
(setf deterministic nil)

(setf texture_format "textures/%s.tga")
(setf building_format "objects/%s.gmod")
(setf object_format "objects/%s.gmod")



;;(setf day_and_night (new day_night_change nil))
