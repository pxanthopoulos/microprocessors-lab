.include "m328PBdef.inc"
    
.equ FOSC_MHZ=16
.equ DEL_mS=600
.equ DEL_NU = FOSC_MHZ*DEL_mS

.org 0x0
    rjmp reset
.org 0x2
    rjmp ISR0
    
reset:
    ldi r24, low(RAMEND)
    out SPL,r24
    ldi r24, high(RAMEND)
    out SPH,r24
    
    ldi r24, (1 << ISC01) | (1 << ISC00)
    sts EICRA, r24
    
    ldi r24, (1 << INT0)
    out EIMSK, r24	;INT0 is enabled at rising edge
			
    sei
    
init:    
    ser r26
    out DDRC, r26	;PORT C as output
    clr r26    
    out DDRB, r26	;PORT B	as input
    
    
loop1:
    clr r26		;counter code (given)
   
loop2:
    out PORTC, r26
    
    ldi r24, low(DEL_NU)
    ldi r25, high(DEL_NU)
    rcall delay_mS
    
    inc r26
    
    cpi r26, 32
    breq loop1
    rjmp loop2
    
    
    
delay_mS:	    ;delay of 1000*DEL_NU + 6 cycles
    ldi r23, 249    ;default is 249, for simulator set to 10
loop_inn:
    dec r23
    nop
    brne loop_inn
    
    sbiw r24, 1
    brne delay_mS
    
    ret
    
    
    
ISR0:
    push r25		    ;save r24, r25 because we use them later
    push r24
    in r24, SREG	    ;save SREG
    push r24
    
debounce:		    ;debounce avoidance algorithm (given)
    ldi r24, (1 << INTF0)
    out EIFR, r24
    
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    
    in r24, EIFR
    lsr r24
    brcs debounce

start:
    in r22, PINB
    clr r21	    ;counter
    
    sbrs r22, 0	    ;sbrc for simulator, sbrs for lab
    inc r21
    
    sbrs r22, 1
    inc r21
    
    sbrs r22, 2
    inc r21
    
    sbrs r22, 3
    inc r21
    
    sbrs r22, 4
    inc r21
    
    sbrs r22, 5
    inc r21
    
    clr r22
    
loop:
    cpi r21, 0x00
    breq output
    
    bclr 0
    lsl r22
    ori r22, 0x01
    
    dec r21
    rjmp loop
    
output:
    out PORTC, r22
    
    ldi r24, low(16*1000)	;the output stays visible for 1 sec
    ldi r25, high(16*1000)	;for validation purposes
    rcall delay_mS
    
end:   
    pop r24		;return saved values
    out SREG, r24
    pop r24
    pop r25
    
    reti