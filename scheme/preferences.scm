
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


