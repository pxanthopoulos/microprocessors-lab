.include "m328PBdef.inc"
    
.org 0x0
    rjmp reset
    
reset:
    ldi r24, low(RAMEND)
    out SPL,r24
    ldi r24, high(RAMEND)
    out SPH,r24
    
    ldi r24, (1 << WGM10) | (1 << COM1A1)
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << CS11)
    sts TCCR1B, r24
    
    ldi zh, high(table*2)
    ldi zl, low(table*2)
    adiw zl, 6			
    
    lpm r23, z
    sts OCR1AL, r23		;DC initalized at 50%
    
    clr r24
    out DDRD, r24		;set D as input
    
    ser r24
    out DDRB, r24
    
    ldi r21, 7			;counter
    
main:
    in r25, PIND		;check if PD1 was pressed
    lsr r25
    lsr r25
    brcc wait_for_PD1		;brcs for sim, brcc for lab
    
    lsr r25			;check if PD2 was pressed
    brcc wait_for_PD2		;brcs for sim, brcc for lab

    rjmp main			;if none, check again
    
    
wait_for_PD1:			;wait for PD1 to be released
    in r25, PIND		;check if PD1 was released
    lsr r25
    lsr r25
    brcs inc_DC			;brcc for sim, brcs for lab
    rjmp wait_for_PD1		;if not, check again
    
wait_for_PD2:			;wait for PD2 to be released
    in r25, PIND		;check if PD2 was released
    lsr r25
    lsr r25
    lsr r25
    brcs dec_DC			;brcc for sim, brcs for lab
    rjmp wait_for_PD2		;if not, check again
    
    
inc_DC:
    ldi r24, low(16*10)		;debounce avoidance
    ldi r25, high(16*10)
    rcall delay_mS
    
    cpi r21, 13
    breq end_inc_DC
    inc r21
    adiw z, 1
    lpm r23, z
    sts OCR1AL, r23
end_inc_DC:
    rjmp main
    
dec_DC:
    ldi r24, low(16*10)		;debounce avoidance
    ldi r25, high(16*10)
    rcall delay_mS
    
    cpi r21, 1
    breq end_dec_DC
    dec r21
    sbiw z, 1
    lpm r23, z
    sts OCR1AL, r23
end_dec_DC:
    rjmp main
    


delay_mS:	    ;delay of 1000*DEL_NU_X + 6 cycles
    ldi r23, 249     ;default is 249, for simulator set to 15
loop_inn:
    dec r23
    nop
    brne loop_inn
    
    sbiw r24, 1
    brne delay_mS
    
    ret
    
    
    
.cseg  
table:
.DW 0x1A05, 0x422E, 0x6B57, 0x9480, 0xBDA8, 0xE6D1, 0x00FA