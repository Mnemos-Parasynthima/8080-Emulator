
; begin program here
				; some comment starting later on in the line
CONST   equ     5
		ARH equ 2
	ach equ 3

start: mvi A,1h
			  mvi B,0ah+1h
				mvi C,CONST					
				mvi D,ARH      
				mvi h,ach shr 3
			  add B ; add uses register A (accumulator)
end:
				hlt



; mvi 7, 1h		0b00111110__00000001
; mvi 0, 0bH	0b00000110__00001011
; mvi 1, 5		0b00001110__00000101
; mvi 2, 2		0b00010110__00000010
; mvi 4, 0		0b00100110__00000000
; add 0				0b10000000
; hlt					0b01110110





; SHA equ (CHACH * 9) + CHA
; CHACH equ LAGA - 2
; CHA equ LAGA / 3

; ABBG equ 5

; mvi H,LAGA + 6

; LAGA equ ABBG + 2



; LAB equ 5
; LAB0: jmp LAB

; LAB: equ 5
; LA jmp LAB

; jmp SET