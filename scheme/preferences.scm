
;Upgrade this as soon as the lisp engine is complete
(defconstant major_version 0)
(defconstant minor_version 50)

(defconstant array-rank-limit 65535)

;Emacs compatibility and external system functions
(defun eval-when-compile (x) nil) ;;just ignore this
(defun require (x) nil)
(defun make-vector (length object) (make-array length :initial-element object))
(defun aset (arr index object) (setf (aref arr index) object))
(defun concat(a b) (concatenate 'string a b))
(defun 1+ (x) (+ x 1))
(defun 1- (x) (- x 1))
(defun insert (x) (print x));; inserts into current buffer, I suppose
(defun interactive () nil)
;;Let's see wheter we need special buffers for such stuff.
;;We could probably assign st to the (window-local) variable window_identifier
(defun switch-to-buffer (st) (print st))
(defun text-mode () nil)
;;Defines keyboard-interactive commands. Should probably really implement
;;this somehow (but more game-like than for emacs)
(defun use-local-map (map-to-use) nil)
(defun turn-on-auto-fill () nil)


;some special list-processing functions
(defun revappend (x y) (append (reverse x) y)) ;;implement the easy way
(defun rplaca (x y) (setf (car x) y))
(defun rplacd (x y) (setf (cdr x) y))
(defun prog2 (a &rest r) (progn a (prog1 (car r))));;test if r is set correctly

;a bit math
(defun ! (x) (if (<=b x 1) 1 (*b x (! (-b x 1))))) ; try to calculate (! 1000) and watch for the speeeed!!!
(defun fact (x) (if (<=b x 1) 1 (*b x (fact (-b x 1)))))
(defconstant pi 3.14159265359)
(setf 2pi (* 2.0 pi))

(defconstant I4_READ 1)
(defconstant I4_WRITE 2)
(defconstant I4_APPEND 4)
(defconstant I4_NO_BUFFER 8)
(defconstant I4_SUPPORT_ASYNC 16)

;helper functions
(defun open (filename &key ((:direction dir) :input) ((:element-type el) 'byte) ((:if-exists ife) nil) ((:if-does-not-exist ifne) nil)
	((:external-format ext) nil))
	"Open: Opens a file. Only the :directon keyword is recognized right now"
	(let (mode) 
		(setf mode (if (eq dir :input) I4_READ (if (eq dir :output) I4_WRITE (if (eq dir :io)
		(or I4_READ I4_WRITE) I4_READ))))
		(new file filename mode)
	)
)

(defun close (hfile)
	"Close: Closes a given file"
	(close-file hfile)
)

;;The following definitions come from abuse - just copied for simplicity
;;PG: I was quite surprised that golg ate them just right away
(defun select_place (x place) 
  (- (/ x place) (* (/ x (* place 10)) 10)))


(defun dig2char (x) 
  (code-char (+ x (char-code "0"))))


;; this creates a list of dpaint numbered antimation from a base name
;; i.e. (seq "hi" 2 5)  -> '("hi0002.pcx" "hi0003.pcx" "hi0004.pcx" "hi0005.pcx")

;; will take into acount reverse sequences

(defun seq (name first last)
  (if (<= first last)
      
(forward-seq name first last)
   
(reverse-seq name first last))
)


(defun forward-seq (name first last) 
  
(if (> first last) 
      nil 
    
(cons (concatenate 'string name (digstr first 4) ".pcx") 
	  
(forward-seq name (+ 1 first) last))))


(defun reverse-seq (name last first) 
 
(if (< last first) 
      nil 
    
(cons (concatenate 'string name (digstr last 4) ".pcx") 
	 
(reverse-seq name (- last 1) first))))

(defun rep (name count)
  
(if (eq count 0)
      nil
    (cons name (rep name (- count 1)))))




;; appends something to the end of a list
(defun app (head tail) (if (null head) tail 
(cons (car head) (app (cdr head) tail))))




