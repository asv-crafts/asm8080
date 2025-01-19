
; simple source without a sense. just to show 
; syntax and features of the assembler

	org 1200h
start:
	lxi b, 2 + 2 * 2   ; guess the result ;)
	lxi d, (2+2) * 2   ; did you suppose this?

; pretend, that we are doing something
	mvi a, 2
	mvi b, 3
	lxi h, message
label:
	add b
	add a
	sub m
	call emptyfunc
	ret

; =====================
; really empty function
; =====================
emptyfunc:
	ret

ds	5                  ; reserve 5 bytes, fill with zeros
ds  5, 77h             ; reserve 5 bytes, will with 77h value
db	55h
dw	aa55h
message: db	"Hello, world!", 0

#include "data.asm"   ; inline source from specified file

end
