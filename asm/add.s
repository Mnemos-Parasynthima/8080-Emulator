
; begin program here
				; some comment starting later on in the line
CONST equ 5

_start: mvi A 1h
			  mvi B 0ah
				mvi C CONST
			  add B ; add uses register A (accumulator)
