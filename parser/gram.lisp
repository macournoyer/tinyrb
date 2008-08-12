;; Copyright (c) 2007 Markus Liedl; See the file LICENSE; 

;; Hi-lock: (("^[a-zA-Z0-9-_]+" (0 (quote hi-red-b) t)))

;; (defun rb-load-gram () (ec-compile-grammar "gram.lisp"))
;; (add-hook 'after-save-hook 'rb-load-gram t t)

program
    (new-mark-scope compstmt)
    

uchar    	 (range ?A ?Z)
lchar    	 (or (range ?a ?z) "_")
digit    	 (range ?0 ?9)
octdigit 	 (range ?0 ?7)
bindigit 	 (or "0" "1")
hexdigit 	 (or (range ?0 ?9) (range ?a ?f) (range ?A ?Z))
non-control-char (range ?  ?~)

wordchar1 	 (or uchar lchar)
wordchar  	 (or uchar lchar digit)
wordchars 	 (string (rep (yield wordchar)))
uword     	 (string (seq (yield uchar) (yield wordchars)))
lword     	 (string (seq (yield lchar) (yield wordchars)))
word     	 (string (seq (yield wordchar1) (yield wordchars)))


term
    (seq h0 (or ";" nl) s0)

terms
    (seq term opt-terms)

opt-terms
    (rep (one-of " \t\n\r;"))

bodystmt
    (seq (@ compstmt)
	 (opt (@ (or (make rescues-else @ rescues else)
		     (make rescues @ rescues))))
	 (opt (@ (make ensure @ ensure))))

rescues
    (make cons
	  (make resc
		(seq "rescue" (except wordchar) h0 (opt exceptions))
		(opt h0 "=>" s0 lhs1)
		(seq then compstmt))
	  (opt rescues))

exceptions
    (make cons ldefined (opt comma exceptions))

ensure
    (seq "ensure" (except wordchar) compstmt)

compstmt
    (seq opt-terms (@ (opt stmts)) opt-terms)

stmts
    (or (seq (or (@ stmt-postcomment)
		 (seq (@ stmt) terms))
	     (@ (make progn (make cons @ many-stmts))))
	(seq (@ (or stmt-postcomment stmt)) opt-terms))

many-stmts
    (or (seq (or (1 stmt-postcomment)
		 (seq (1 stmt) terms))
	     (make cons 1 (opt many-stmts)))
	(make cons stmt (make nil)))

stmt-postcomment
    (make postcomment stmt term-comment-lines)

term-comment-lines
    (seq h0 (opt ";") (@ comment-lines) opt-terms)

stmt
    (location
      (or (make comment comment-lines (seq nl h0 stmt0))
	  unassociated-comment
	  stmt0))
   

stmt0
    (seq (@ stmt1)
	 (rep h0
	      (@ (or (seq "if" (except wordchar) s0c (make rif @ boolean))
		     (seq "unless" (except wordchar) s0c (make runless @ boolean))
		     (seq "while" (except wordchar) s0c (make rwhile @ boolean))
		     (seq "until" (except wordchar) s0c (make runtil @ boolean))
		     (seq "rescue" (except wordchar) s0c (make rescue @ boolean))))))

stmt1
    (or alias
	undef
	global-begin
	global-end
	masgn
	boolean)

alias-op
    (seq (opt ":") operation2)

alias
    (seq "alias" (except wordchar) s0
	 (or (make alias alias-op (seq s0 alias-op))
	     (make valias
		   gvar
		   (seq s0 valias-target))))

valias-target
    (or gvar
	(seq s0 
	     (or special-variables
		 back-ref
		 (error nth-ref
			"can't make alias for the number variables"))))



undef
    (seq "undef" (except wordchar) (make undef undef-list))

undef-list
    (make cons undef-part (opt comma undef-list))

undef-part
    (seq s0 (opt ":") operation2)


global-begin
    (seq "BEGIN" (except wordchar)
	 (make begin begin-end-block))

global-end
    (seq "END" (except wordchar)
	 (make end begin-end-block))

begin-end-block
    (seq s0 "{" s0
	 (new-mark-scope (@ compstmt))
	 s0 "}")



boolean
    (seq (@ not)
 	 (rep (@ (or (seq h0 "and" (except wordchar) s0c (make and @ not))
 		     (seq h0 "or" (except wordchar) s0c (make or @ not))))))

not
    (or (seq "not" (except wordchar) s0c (make not not))
	command)

command
    (or commandxx
	casg
	defined)

commandxx
    (or (location (seq "!" (make opnot commandxx)))
	commandx)

commandx
    (or yieldc
	superc
	return
	breakc
	nextc
	command1
	command2)

command1
    (seq (1 operation-na)
	 h
	 (make self-cmd 1 cmdargs-block))


command2
    (seq (1 primary) h0
	 (until (seq h0 (or "." "::") s0
		     (2 operation2-na) h
		     (@ (make cmd 1 2 cmdargs-block)))
		(seq h0
		     (1 (location
			 (or (seq (or "." "::") s0
				  (make send 1
					operation2
					(opt paren-arguments-block)))
			     (make aref 1 array)))))))


yieldc
    (seq "yield" (except wordchar) h0 (make yieldc cmdargs-opt-block))

superc
    (seq "super" (except wordchar) h0 (make csuper cmdargs-opt-block))

return
    (seq "return" (except wordchar) h0
;;	 (make return cmdargs1))
	 (make return cmd2args))
;;	 (make return cmdargs))
;; 	 (or (make return mrhs1)
;; 	     (make return1 arg)
;; 	     (make return-nil)))

breakc
    (seq "break" (except wordchar) h0
	 (make break (opt cmdargs)))

nextc
    (seq "next" (except wordchar) h0
	 (make next (opt cmdargs)))

cmdargs-block
    (or (seq (except "<<")
	     (@ (make cons commandx (make nil))))
	(seq (@ cmdargs)
	     (opt (@ (make extra @ block)))))

cmdargs-opt-block
    (or (make extra (opt cmdargs) block)
	cmdargs)

cmdargs
    (enter-mode cmdargs
      (make cons cmdarg-first (opt comma cmdargs1)))

cmdargs1
    (make cons cmdarg (opt comma cmdargs1))

cmdarg-first
    (enter-mode cmdarg-first
		cmdarg)

cmdarg
    (enter-mode no-do-block
      arg)



cmd2args
    (enter-mode cmdargs
      (make cons cmd2arg-first (opt comma cmd2args1)))

cmd2args1
    (make cons arg (opt comma cmd2args1))

cmd2arg-first
    (enter-mode cmdarg-first
		arg)



arg
    (or (make make-hash hash-assocs)
	splat-arg
	(seq "&" s0 (make block ldefined)))

splat-arg
    (or ldefined
	splat-arg1)

splat-arg1
    (seq "*" s0 (make splat ldefined))

ldefined
    (location defined)

masgn
    (seq (1 mlhs1)
	 h0 "=" (except "~") s0
	 (or (make masgn 1
		   (or mrhs1 (make cons splat-arg1 (make nil))))
	     (make masgn1 1 command)))

mlhs1
    (or (make cons
	      (or mlhsp mlhsx-splat)
	      (opt comma (opt mlhs)))
	(make cons
	      mlhsx
	      (seq comma (opt mlhs))))

mlhs 
    (make cons
	  (or mlhsp
	      mlhsx)
	  (opt comma (opt mlhs)))

mlhsp
    (seq h0 "("
	 s0 (@ (make enter mlhs))
	 h0 ")")

mlhsx
    (or lhs1
	mlhsx-splat)

mlhsx-splat
    (seq "*" s0 (or (make splat lhs1)
		    (make anon-splat)))

mrhs1
    (make cons
	  mrhsx
	  (seq comma mrhs))

mrhs
    (make cons
	  mrhsx
	  (opt comma mrhs))

mrhsx
    arg


defined
    (or defined1
	assignment)

defined1
    (seq "defined?" s0
	 (except "(")
	 (make definedp assignment))

rop
    (or asg defined1)

assignment
    (or asg
	rescue-mod)

casg
    (seq (1 lhs1) h0
	 (or (seq "="    (except (one-of "~="))
		         s0c (make assign 1 asgvalue))
	     (seq "&&="  s0c (make and-assign 1 command))
	     (seq "||="  s0c (make or-assign 1 command))
	     (seq "+="   s0c (make add-assign 1 command))
	     (seq "-="   s0c (make sub-assign 1 command))
	     (seq "*="   s0c (make mul-assign 1 command))
	     (seq "/="   s0c (make div-assign 1 command))
	     (seq "%="   s0c (make rem-assign 1 command))
	     (seq "**="  s0c (make pow-assign 1 command))
	     (seq "<<="  s0c (make shl-assign 1 command))
	     (seq ">>="  s0c (make shr-assign 1 command))
	     (seq "|="   s0c (make bitor-assign 1 command))
	     (seq "^="   s0c (make bitxor-assign 1 command))
	     (seq "&="   s0c (make bitand-assign 1 command))))

asg
    (seq (1 lhs1) h0
	 (or (seq "="    (except (one-of "~="))
		         s0c (make assign 1 ldefined))
	     (seq "&&="  s0c (make and-assign 1 ldefined))
	     (seq "||="  s0c (make or-assign 1 ldefined))
	     (seq "+="   s0c (make add-assign 1 ldefined))
	     (seq "-="   s0c (make sub-assign 1 ldefined))
	     (seq "*="   s0c (make mul-assign 1 ldefined))
	     (seq "/="   s0c (make div-assign 1 ldefined))
	     (seq "%="   s0c (make rem-assign 1 ldefined))
	     (seq "**="  s0c (make pow-assign 1 ldefined))
	     (seq "<<="  s0c (make shl-assign 1 ldefined))
	     (seq ">>="  s0c (make shr-assign 1 ldefined))
	     (seq "|="   s0c (make bitor-assign 1 ldefined))
	     (seq "^="   s0c (make bitxor-assign 1 ldefined))
	     (seq "&="   s0c (make bitand-assign 1 ldefined))))

lhs1
    (or sends1-lhs ;; todo: don't allow code like `a.b(2,3) = 7'
	variable
	new-lvar
	toplevel-constant)

new-lvar
    (seq non-keyword
         (@ (make lvar (mark lword)))
	 (except h0 (or "." "[")))

asgvalue
    (or commandxx
	asgargs)

asgargs
    (or (make make-array (make cons arg (seq comma asgargs1)))
	defined)

asgargs1
    (make cons arg (opt h0 comma asgargs1))



rescue-mod
    (seq (@ short-if) h0
	 (opt (not-in-mode cmdargs
		(seq "rescue" (except wordchar) s0c
		     (@ (make rescue @ boolean))))))

short-if
    (seq (@ range) h0
	 (opt "?" s0c (1 ldefined) s0c
	      ":" (except ":") s0c (2 ldefined)
	      (@ (make short-if @ 1 2))))

range
    (seq (@ orop) h0
	 (opt (@ (or (seq "..." s0c (make rangeexc @ (or rop orop)))
		     (seq ".." s0c (make rangeinc @ (or rop orop)))))))
   

orop
    (seq (@ andop) h0
	 (rep (@ (seq "||" s0c (make orop @ (or rop andop))))))
   

andop
    (seq (@ compare) h0
	 (rep (@ (seq "&&" s0c (make andop @ (or rop compare))))))

compare
    (seq (@ relop) h0
	 (opt (@ (or (seq "<=>" s0c (make cmp @ (or rop relop)))
		     (seq "===" s0c (make eqq @ (or rop relop)))
		     (seq "==" s0c (make eq @ (or rop relop)))
		     (seq "!=" s0c (make neq @ (or rop relop)))
		     (seq "=~" s0c (make match @ (or rop relop)))
		     (seq "!~" s0c (make nmatch @ (or rop relop)))))))

relop
    (seq (@ bitor) h0
	 (rep (@ (or (seq "<=" s0c (make lte @ (or rop relop)))
		     (seq "<" s0c (make lt @ (or rop relop)))
		     (seq ">=" s0c (make gte @ (or rop relop)))
		     (seq ">" s0c (make gt @ (or rop relop)))))))

bitor
    (seq (@ bitand) h0
	 (rep (@ (or (not-in-mode block-formal-default
		       (seq "|" s0c (make bitor @ (or rop bitand))))
		     (seq "^" s0c (make bitxor @ (or rop bitand)))))))

bitand
    (seq (@ shift) h0
	 (rep (@ (seq "&" s0c (make bitand @ (or rop shift))))))

shift
    (seq (@ add) h0
	 (rep (@ (or (seq "<<" s0c (make shl @ (or rop add)))
		     (seq ">>" s0c (make shr @ (or rop add)))))))

add
    (seq (@ mul) h0
	 (rep (@ (or (seq "+" s0c (make add @ (or rop mul)))
		     (seq "-" s0c (make sub @ (or rop mul)))))))

mul
    (seq (@ negated) h0
	 (rep (@ (or (seq "*" s0c (make mul @ (or rop negated)))
		     (seq "/" s0c (make div @ (or rop negated)))
		     (seq "%" s0c (make rem @ (or rop negated)))))))

uop-space
    (opt (not-in-mode cmdarg-first s0c))

negated
    (or (location (seq "-" (except (range ?0 ?9))
		       uop-space
		       (make neg (or rop negated))))
	pow)
   

pow
    (seq (@ unaries) h0
	 (opt "**" s0c (@ (make pow @ (or rop negated)))))
   

unaries
    (or (location (or (seq "!" s0c (make opnot (or rop unaries)))
		      (seq "+" uop-space (make uplus (or rop unaries)))
		      (seq "~" s0c (make optilde (or rop unaries)))))
	sends)
   

sends
    (or	sends1
	primary)

;;     (seq (@ primary) h0
;; 	 (rep h0
;; 	      (@ (or (seq (or "." "::") s0
;; 			  (or (make send @
;; 				    operationl
;; 				    (opt paren-arguments-block))
;; 			      (make send @
;; 				    uword
;; 				    paren-arguments-block)
;; 			      (make colon2 @ uword)))
;; 		     (make aref @ aref-array)))))

;; location?
sends1
    (seq (@ primary)
	 (rep1 h0
	       (@ (or (seq (or "." "::") s0c
			   (or (make send @
				     operationl
				     (opt paren-arguments-block))
			       (make send @
				     uword
				     paren-arguments-block)
			       (make colon2 @ uword)))
		      (make aref @ aref-array)))))
   

sends1-lhs
    (seq (@ primary)
	 (rep1 h0
	       (@ (or (seq (or "." "::") s0c
			   (or (make send @
				     operationl-na
				     (opt paren-arguments-block))
			       (make send @
				     uword
				     paren-arguments-block)
			       (make colon2 @ uword)))
		      (make aref @ aref-array)))))

aref-array
    (seq (@ array)
	 (opt h0 (@ (make extra @ block))))


paren-arguments
    (leave-mode cmdarg-first
      (leave-mode cmdargs
	(seq "(" (@ arguments) s0 ")" 
	     h0 (opt (@ (make extra @ block))))))

paren-arguments-block
    (seq h0
	 (or paren-arguments
	     (make extra (make nil) block)))

array
    (seq "[" (@ arguments) (opt comma) s0c "]")

arguments
     (seq s0c
	 (@ (or (make cons commandx (make nil)) ;; todo: with warning
		(opt (make cons arg opt-comma-arguments))))
	 s0c)

opt-comma-arguments
    (opt s0c
	 comma
	 s0c
	 (make cons arg opt-comma-arguments))

primary
    (location
     (or (leave-mode block-formal-default
	   (leave-mode no-do-block
	     (leave-mode cmdargs
	       (or string-regexp-words
		   primary1
		   primary2))))
	 self-send))
   

primary1
    (leave-mode cmdarg-first
       (or literal
	   here-doc
	   yield
	   super
	   break
	   next
	   redo
	   retry
	   (seq "return" (make return (make nil)))
	   for-in
	   case
	   variable
	   defclass
	   defmodule
	   defmethod
	   begin-end
	   parenthesized
	   paren-definedp))

primary2
    (not-in-mode cmdarg-first
      (or if
	  unless
	  while
	  until))

yield
    (seq "yield" (or (make yield paren-arguments-block)
		     (seq (except wordchar) (make yield0))))

super
    (seq "super" (or (make super paren-arguments-block)
		     (seq (except wordchar) (make zsuper))))


break
    (seq "break" (except wordchar)
	 (make zbreak))

next
    (seq "next" (except wordchar)
	 (make znext))

redo
    (seq "redo" (except wordchar)
	 (make redo))

retry
    (seq "retry" (except wordchar)
	 (make retry))

if
    (seq "if" (except wordchar)
	 (@ if2)
	 "end" (except wordchar))

if2
    (seq s0 (make if
		  boolean
		  (seq then compstmt)
		  (opt iftail)))

iftail
    (or (location (seq "elsif" (except wordchar) if2))
	else)

else
    (location (seq dropped-comment s0
		   "else" (except wordchar) dropped-comment s0
		   compstmt))

then
    (seq dropped-comment h0
	 (or (seq then1 dropped-comment (opt term))
	     (seq term (opt then1))
	     (seq ":" dropped-comment))
	 s0)

then1
    (seq "then" (except wordchar))

unless
    (seq "unless" (except wordchar) s0
	 (@ (make unless
		  boolean
		  (seq then compstmt)
		  (opt else)))
	 "end" (except wordchar))
		  

while
    (seq "while" (except wordchar) s0
	 (@ (make while boolean-no-do do-compstmt-end)))

until
    (seq "until" (except wordchar) s0
	 (@ (make until boolean-no-do do-compstmt-end)))

for-in
    (seq "for" (except wordchar) s0
	 (make for-in
	       mlhs-opt-comma
	       (seq h0 "in" (except wordchar) s0
		    boolean-no-do)
	       do-compstmt-end))

mlhs-opt-comma
    (seq (@ mlhs) (opt h0 ","))

boolean-no-do
    (seq (@ (location
	     (enter-mode no-do-block
	       boolean)))
	 dropped-comment)

do-compstmt-end
    (seq do (@ compstmt) "end" (except wordchar))

do
    (seq h0
	 (or term ":" "do")
	 s0)

case
    (seq "case" (except wordchar) s0c
	 (@ (make case (opt (location boolean))
		       (seq s0c when)
		       (opt s0 else)))
	 s0 "end" (except wordchar))

when
    (seq opt-terms dropped-comment opt-terms
	 "when" (except wordchar)
	 s0
	 (1 arguments-splat)
	 then
	 (make cons (make when 1 compstmt) (opt when)))


arguments-splat
    (make cons
	  splat-arg
	  (opt s0 comma (opt arguments-splat)))

self-send
    (or (make self-send
	      (seq non-keyword (or fid lword))
	      (opt paren-arguments-block))
	(make self-send uword paren-arguments-block))

non-keyword
    (except (or (seq (or "if" "unless" "until" "for" "in"
			 "while" "and" "or" "not" "begin"
			 "then" "else" "elsif" "end"
			 "case" "when"
			 "class" "module" "def" "do"
			 "rescue" "ensure"
			 "yield" "return" "yield" "super" "next")
		     non-word-char)
		"defined?"))

non-word-char
    (one-of "\n\r\t \"#$%&'()*+,-./:;<=>@[\\]^`{|}~")


operation
    (seq non-keyword
	 (or fid
	     lword
	     uword))

;; operation; no assign
operation-na
    (seq non-keyword
	 (or fid-na
	     lword
	     uword))

operation2
    (or fid
	lword
	uword
	aref-assign
	operator)

aref-assign
    (seq (@ "[]=") (except "="))

operation2-na
    (or fid-na
        lword
	uword
	operator)

operationl
    (or fid lword operator)


operationl-na
    (or fid-na lword operator)

fid
    (string (seq (yield word)
		 (or (yield "?")
		     (seq (1 "!") (except "=") (yield 1))
		     (seq (1 "=")
			  (except (one-of ">=~"))
			  (yield 1)))))

fid-na
    (string (seq (yield word)
		 (or (yield "?")
		     (seq (1 "!") (except "=") (yield 1)))))

operator
    (or "-@" "+@" "~"
	"+" "-" "**" "*" "/" "%" "<<" ">>" "&" "|" "^"
	"<=>" "<=" "<" ">=" ">"
	"===" "==" "=~"
	"[]")


begin-end
    (seq "begin" (except wordchar) s0
	 (@ bodystmt)
	 "end")

parenthesized
    (seq "(" s0 (@ compstmt) ")")


paren-definedp
    (seq "defined?" s0 (make paren-definedp parenthesized))


literal
    (or number
	(make make-array array)
	symbol
	hash-literal
	char-literal)

char-literal
    (seq "?" (or (seq "\\" (or backslash-char
			       (error (opt) "bad backslash")))
		 (make char (range ?! ?\xff))))  ;; any byte from 33 upto 255 

backslash-char
    (or (make backslash-char (one-of "abefnrstv\\"))
	(seq "x" (make hexchar hex-integer-max-two-digits))
	(make octchar octal-integer-max-three-digits)
	ctrl-meta-char-literal)

ctrl-meta-char-literal
    (or (seq (or "C-" "c") (make ctrl non-control-char-literal))
	(seq "M-\\C-"  (make meta-ctrl non-control-char-literal))
	(seq "M-"  (make meta non-control-char-literal)))

non-control-char-literal
    (make char non-control-char)

octal-integer-max-three-digits
    (string (yield octdigit)
	    (opt (yield octdigit)
		 (opt (yield octdigit))))

hex-integer-max-two-digits
    (string (yield hexdigit)
	    (opt (yield hexdigit)))

number
    (or (seq "-" (make neg number1))
	number1)
   

number1
    (or hex-literal
	bin-literal
	oct-literal
	float-literal
	dec-literal)

hex-literal
    (seq "0x" (make hexint
		    (string (yield hexdigit)
			    (rep (opt (yield "_"))
				 (yield hexdigit)))))

dec-literal
    (make int dec-digits)
   

dec-digits
    (string (yield digit)
	    (rep (opt (yield "_"))
		 (yield digit)))

oct-literal
    (seq "0" (make octint
		   (string (yield octdigit)
			   (rep (opt (yield "_"))
				(yield octdigit)))))

bin-literal
    (seq "0b" (make binint
		    (string (yield bindigit)
			    (rep (opt (yield "_"))
				 (yield bindigit)))))

float-literal
    (make float
	  (string (yield dec-digits)
		  (yield ".")
		  (yield dec-digits)
		  (opt (yield "e")
		       (opt (yield "-"))
		       (yield dec-digits))))

symbol
    (seq ":"
	 (or (make symbol
		   (or ivarname
		       cvarname
		       gvarname
		       operation2
		       symbol-sv))
	     (make symbol-from-string strings)))

symbol-sv
    (string
      (seq (yield "$")
	   (yield (one-of "!@~=/\\,;.<>_0*$?:\"&`'+123456789"))))

hash-literal
    (seq "{" s0c
	 (enter-mode hash-comma
	   (@ (make make-hash hash-assocs0)))
	 s0c (opt comma) s0c "}")

hash-assocs0
    (opt hash-assocs)

hash-assocs
    (make cons
	  (or (seq (1 word)
		   s0c ":" (except ":") s0c
		   (make assoc (make symbol 1) ldefined))
	      (seq (1 ldefined)
		   s0c
		   (or "=>"
		       (only-in-mode hash-comma ","))
		   s0c
		   (make assoc 1 ldefined)))
	  (opt s0c comma s0c hash-assocs))

variable
    (or known-lvar
	ivar
	gvar
	special-variables
	constant
	toplevel-constant
	cvar
	nil
	self
	true
	false
	__line__
	__file__)

known-lvar
    (seq (@ (make lvar (is-marked lword)))
	 (except (or "?" "!" (seq h0 "("))))

nil
    (seq "nil" (except wordchar) (make rbnil))

self
    (seq "self" (except wordchar) (make self))

true
    (seq "true" (except wordchar) (make true))

false
    (seq "false" (except wordchar) (make false))

__line__
    (seq "__LINE__" (except wordchar) (make __line__))

__file__
    (seq "__FILE__" (except wordchar) (make __file__))

cvar
    (make cvar cvarname)

cvarname
    (string (seq (yield "@@") (yield word)))

ivar
    (make ivar ivarname)

ivarname
    (string (seq (yield "@") (yield word)))

gvar
    (make gvar gvarname)

gvarname
    (string (seq (yield "$")
		 (or (yield word)
		     (seq (yield "-")
			  (yield (or uchar lchar))))))

special-variables
     (seq "$"
	 (or (seq "!" (make sv-exception))
	     (seq "@" (make sv-backtrace))
	     (seq "~" (make sv-match-info))

	     (seq "=" (make sv-case-insensitive))
	     (seq "/" (make sv-input-record-separator))
	     (seq "\\" (make sv-output-record-separator))
	     (seq "," (make sv-output-field-separator))
	     (seq ";" (make sv-string-split-separator))

	     (seq "." (make sv-current-input-line))
	     (seq "<" (make sv-complete-input))
	     (seq ">" (make sv-print-default-output))
	     (seq "_" (make sv-input-last-line))
	     (seq "0" (make sv-script-name))
	     (seq "*" (make sv-command-line-arguments))
	     (seq "$" (make sv-interpreter-pid))
	     (seq "?" (make sv-child-status))
	     (seq ":" (make sv-load-path))

	     (seq "\"" (make sv-loaded-modules))
	     
	     back-ref-0
	     nth-ref-0))

back-ref
    (seq "$" back-ref-0)

back-ref-0
    (or (seq "&" (make sv-matched-string))
	(seq "`" (make sv-match-lefthand))
	(seq "'" (make sv-match-righthand))
	(seq "+" (make sv-match-last-bracket)))

nth-ref-0
    (make sv-match-group dec-digits)

nth-ref
    (seq "$" nth-ref-0)

constant
    (seq (@ (make constant uword))
	 (except h0 "("))

toplevel-constant
    (seq "::" (@ (make toplevel-constant uword)))

defclass
    (seq "class" (except wordchar) s0
	 (new-mark-scope (@ (or defsclass defclass1)))
	 s0 "end")

defclass1
    (make class
	  cpath       ;; todo: ensure form (constant ...) or (colon2 ...)
	  superclass
	  bodystmt)

defmodule
    (seq "module" (except wordchar) s0
	 (@ (make module
		  cpath
		  (seq dropped-comment terms
		       (new-mark-scope bodystmt))))
	 s0 "end")

cpath
    sends

cname
    (seq (error lword "class/module name must be constant")
	 uword)

superclass
    (or (seq dropped-comment term (@ (make nil)))
	(seq h0 "<" (@ boolean-opt-terms) dropped-comment))

defsclass
    (seq "<<"
	 (make sclass
	       boolean-opt-terms
	       (seq dropped-comment bodystmt)))

boolean-opt-terms
    (seq s0 (@ boolean) opt-terms)

defmethod
    (seq "def" (except wordchar) s0
	 (@ (or defmethod1
		def-singleton-method))
	 s0 "end" (except wordchar))

defop
    (seq (@ operation2) h0)

defmethod1
    (seq (1 defop)
	 (new-mark-scope
	    (@ (make def 1 defformals bodystmt))))

def-singleton-method
    (seq (1 (or parenthesized
		variable))
	 (or "::" ".")
	 (2 defop)
	 (new-mark-scope
	    (@ (make defs 1 2 defformals bodystmt))))

defformals
    (or (seq dropped-comment terms (@ (make nil)))
	(seq "(" s0 (@ (opt mformals)) s0 ")" dropped-comment)
	(seq (@ (opt mformals)) dropped-comment terms))

mformals
    (make cons mformal (opt comma mformals))

mformal
    (or splat-formal
	blockvar
	mformal1)

mformal1
    (seq (@ (mark lword))
	 (opt h0 "=" s0 (@ (make optarg @ defined))))

splat-formal
    (seq "*" (or (make splat (mark lword))
		 (make anon-splat)))

blockvar
    (seq "&" (make blockvar (mark lword)))

block
    (leave-mode cmdargs
      (leave-mode cmdarg-first
	 (location
	   (or brace-block
	       (not-in-mode no-do-block
		 do-block)))))

brace-block
    (seq "{" s0c
	 (add-mark-scope
	    (@ (make brace-block (opt bpformals) compstmt)))
	 s0 "}")

do-block
    (seq "do" (except wordchar) s0c
	 (add-mark-scope
	   (@ (make do-block (opt bpformals) compstmt)))
	 "end" (except wordchar))

bpformals
    (seq "|" s0 (@ (opt bformals)) s0 (opt "," s0) "|")

bformals
    (make cons bformal (opt s0 comma bformals))

bformal
    (or (@ splat-formal)
	(@ blockvar)
	(@ mlhsp)
	(@ ivar)
	(seq (mark (@ lword))
	     (opt s0 "=" s0
		  (@ (make optarg @
			   (enter-mode block-formal-default
			     ldefined))))))

comma
    (seq h0 "," dropped-comment s0)


dropped-comment
    (opt h0 comment-line)

unassociated-comment
    (or (seq (@ (make unassociated-comment comment-lines))
	     ;;"\n" h0) ;; followed by an empty line
                      ;; (or end-of-file ...)
	     )
	(@ begin-end-comment))


comment-lines
    (make cons comment-line (opt nl h0 comment-lines))

comment-line
    (seq "#" h0
	 (string (rep (seq (yield comment-char)))))

comment-char
    (or (range ?  ?\xff) "\t")

begin-end-comment
    (seq "=begin" h0
	 (@ (make begin-end-comment
		  (string
		   (rep (except nl "=end")
			(yield (or (range ?  ?~) "\t" nl))))))
	 nl
	 "=end")


string-regexp-words
    (or (seq "/" re)
	strings
	(seq "`" system)
	(seq "%"
	     (or (make lqstring
		       (seq "q" sqstring-balanced))
		 (make uqstring
		       (or (seq "Q"
				(or string-balanced
				    string-balanced-spc))
			   string-balanced))
		 (seq "w" (enter-mode words
			    (make lwords sqstring-balanced)))
		 (seq "W" (enter-mode words
			    (make uwords string-balanced)))
		 (seq "r" re-balanced)
		 (seq "x" (make xsystem string-balanced)))))

strings
    (or (seq "\"" (make string string1))
	(seq "'" (make sqstring sqstring1)))


string1
    (or (seq "\"" (opt (seq h0 "\"" string1)))
	(make cons
	      (or string2
		  (seq "\\" backslash-char)
		  string-expr)
	      string1))

string2
    (string (rep1 (except (or "\""
			      (seq "#" (one-of "{@$"))
			      (seq "\\" (one-of "abefnrstvx0123456789cCM"))))
		  (or (seq "\\" nl)
		      (yield (or (seq "\\" non-control-char)
				 non-control-char-tab-nl)))))



non-control-char-tab-nl
    (or (range ?  ?\xff)  ;; any byte from 32 upto 255 
	"\n"
	"\r"
	"\t")

sqstring1
    (or (seq "'" (opt (seq h0 "'" sqstring1)))
	(make cons sqstring2 sqstring1))

sqstring2
    (string (rep1 (except "'")
		  (yield (or (seq "\\" (one-of "'\\"))
			     non-control-char-tab-nl))))

string-balanced
    (or string-balanced1
	string-balanced-any)

string-balanced1
    (or (call string1-balanced "(" (return ")"))
	(call string1-balanced "<" (return ">"))
	(call string1-balanced "{" (return "}"))
	(call string1-balanced "[" (return "]")))

string-balanced-any
    (call string1-balanced
	   (1 (or (one-of "!\"#$%&')*+,-./:;<>?@\\]^_`|}~")
		  (not-in-mode cmdarg-first "=")))
	   1)

string-balanced-spc
    (call string1-balanced
	  (1 " ")
	  1)

(string1-balanced open close)
    (or (seq (again (arg close)) (make nil))
	(make cons
	      (or (call string2-balanced (arg open) (arg close))
		  (seq "\\" backslash-char)
		  (only-in-mode words (seq s (make wordsep)))
		  string-expr
		  (call string1-balanced-sub (arg open) (arg close)))
	      (call string1-balanced (arg open) (arg close))))

(string1-balanced-sub open close)
    (make substring
	  (again (arg open))
	  (call string1-balanced (arg open) (arg close))
	  (arg close))


(string2-balanced open close)
    (string (rep1 (except (or (again (arg close))
			      (again (arg open))
			      (seq "#" (one-of "{@$"))
			      (seq "\\" (one-of "abefnrstvx0123456789cCM"))
			      (only-in-mode words s)))
		  (or (seq "\\" (1 nl) (opt (only-in-mode words
					      (yield 1))))
		      (yield (or (seq "\\" (or non-control-char
					       (again (arg close))))
				 non-control-char-tab-nl)))))

system
    (make system 
	  (call string1-balanced
		(return "`")
		(return "`")))


sqstring-balanced
    (or (call sqstring1-balanced "(" (return ")"))
	(call sqstring1-balanced "<" (return ">"))
	(call sqstring1-balanced "{" (return "}"))
	(call sqstring1-balanced "[" (return "]"))
	bsqstring-any)

bsqstring-any
    (call sqstring1-balanced
	  (1 (one-of " !\"#$%&')*+,-./:;<=>?@\\]^_`|}~"))
	  1)

(sqstring1-balanced open close)
    (or (seq (again (arg close)) (make nil))
	(make cons
	      (or (call sqstring2-balanced (arg open) (arg close))
		  (only-in-mode words (seq s (make wordsep)))
		  (call sqstring1-balanced-sub (arg open) (arg close)))
	      (call sqstring1-balanced (arg open) (arg close))))

(sqstring1-balanced-sub open close)
    (make substring
	  (again (arg open))
	  (call sqstring1-balanced (arg open) (arg close))
	  (arg close))


(sqstring2-balanced open close)
    (string (rep1 (except (or (again (arg close))
			      (again (arg open))
			      (only-in-mode words s)))
		  (yield (or (seq "\\" (or "\\"
					   (again (arg close))
					   " "
					   nl))
			     non-control-char-tab-nl))))


string-expr
    (seq "#"
	 (or string-block-expr
	     ivar
	     cvar
	     gvar
	     back-ref
	     nth-ref))

string-block-expr
     (seq "{"
	  (@ (leave-mode cmdarg-first
	       (leave-mode cmdargs
	         compstmt)))
	  "}")


here-doc
    (seq "<<"
	 (or (seq "-"
		  (enter-mode here-doc-space
		    here-doc-x))
	     here-doc-x))

here-doc-x
    (or dq-here-doc sq-here-doc)

dq-here-doc
    (seq (or (seq "\"" (1 dq-here-doc-limiter) "\"")
	     (1 here-doc-limiter))
	 (make here-doc 1
	       (postpone-rest-of-line (call dq-here-doc-1 1))))

dq-here-doc-limiter
    (string (rep1 (seq (except "\"")
		       (yield non-control-char-tab-nl))))

here-doc-limiter
    (string (rep1 (yield wordchar)))

(dq-here-doc-1 limiter)
    (or (seq nl (opt (only-in-mode here-doc-space hx))
	     (again (arg limiter)) nl
	     (make nil))
	(make cons
	      (or (call dq-here-doc-2 (arg limiter))
		  (seq "\\" backslash-char)
		  string-expr)
	      (call dq-here-doc-1 (arg limiter))))

(dq-here-doc-2 limiter)
    (string (rep1 (except (or (seq nl (opt (only-in-mode here-doc-space hx))
				   (again (arg limiter)) nl)
			      (seq "#" (one-of "{@$"))
			      (seq "\\" (one-of "abefnrstvx0123456789cCM"))))
		  (or (seq "\\" nl)
		      (yield (or (seq "\\" (one-of "#\"\\"))
				 non-control-char-tab-nl)))))


sq-here-doc
    (seq (seq "'" (1 sq-here-doc-limiter) "'")
	 (@ (make sq-here-doc
		  1
		  (postpone-rest-of-line
		   (string (until (seq nl
				       (opt (only-in-mode here-doc-space hx))
				       (again 1)
				       nl)
				  (yield non-control-char-tab-nl)))))))

sq-here-doc-limiter
    (string (rep1 (seq (except "'")
		       (yield non-control-char-tab-nl))))



re
    (seq can-re-start-with-space
	 (make re 
	       re1
	       re-flags))

can-re-start-with-space
    (or (not-in-mode cmdarg-first (seq))
	(except (or " " nl)))

re-flags
    (opt (string (rep1 (yield (range ?a ?z)))))

re1
    (or (seq "/" (make nil))
	(make cons
	      (or re2 string-expr)
	      re1))

re2
    (string (rep1 (except (or "/" (seq "#" (one-of "{@$"))))
		  (or (seq "\\"
			   (or nl
			       (yield "/")
			       (seq "\\" (yield (return "\\\\")))
			       (seq "#" (yield (return "\\#")))))
		      (yield non-control-char-tab-nl))))



re-balanced
    (make re rebal0 re-flags)

rebal0
    (or (call rebal1 "(" (return ")"))
	(call rebal1 "<" (return ">"))
	(call rebal1 "{" (return "}"))
	(call rebal1 "[" (return "]"))
	rebal-any)

rebal-any
    (call rebal1
	  (1 (one-of " !\"#$%&')*+,-./:;<=>?@\\]^_`|}~"))
	  1)

(rebal1 open close)
    (or (seq (again (arg close)) (make nil))
	(make cons
	      (or (call rebal2 (arg open) (arg close))
		  string-expr
		  (call rebal1-sub (arg open) (arg close)))
	      (call rebal1 (arg open) (arg close))))

(rebal1-sub open close)
    (make substring
	  (again (arg open))
	  (call rebal1 (arg open) (arg close))
	  (arg close))


(rebal2 open close)
    (string (rep1 (except (or (again (arg close))
			      (again (arg open))
			      (seq "#" (one-of "{@$"))))
		  (or (seq "\\" nl)
		      (seq (yield "\\")
			   (yield non-control-char-tab-nl))
		      (yield non-control-char-tab-nl))))




;; arbitrary many spaces; also newline
s0
    (rep (one-of " \n\r\t"))

;; at least one space; also newline
s
    (rep1 (one-of " \n\r\t"))

;; horizontal space
h0
    (rep hspace)

;; horizontal space
h
    (rep1 hspace)

hspace
    (or (one-of " \t")
	(seq "\\" nl))

hx
    (rep (one-of " \t"))


s0c
    (rep (or s
	     comment-line
	     (seq "\\" nl)))

nl
    (or "\r\n" "\r" "\n")
