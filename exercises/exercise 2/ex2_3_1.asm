.include "m328PBdef.inc"
    
.equ FOSC_MHZ=16
.equ DEL_1=3500
.equ DEL_NU1 = FOSC_MHZ*DEL_1
.equ DEL_2=500
.equ DEL_NU2 = FOSC_MHZ*DEL_2

.org 0x0
    rjmp reset
.org 0x4
    rjmp ISR1
    
reset:
    ldi r24, low(RAMEND)
    out SPL,r24
    ldi r24, high(RAMEND)
    out SPH,r24
    
    ldi r24, (1 << ISC11) | (1 << ISC10)
    sts EICRA, r24
    
    ldi r24, (1 << INT1)
    out EIMSK, r24
			;INT1 is enabled at rising edge
    sei
    
init:    
    ser r26		;PORT B is set as output
    out DDRB, r26
    
loop:
    ldi r24, low(DEL_NU1)   ;main loop is: turn off PORTB every 3.5 sec
    ldi r25, high(DEL_NU1)  ;if an interrupt comes, it adds 0.5 sec delay and
    rcall delay_mS	    ;renews the 3.5 sec counter
    
    clr r26
    out PORTB, r26

    rjmp loop
   
  
    
delay_mS:	    ;delay of 1000*DEL_NU_X + 6 cycles
    ldi r23, 249     ;default is 249, for simulator set to 15
loop_inn:
    dec r23
    nop
    brne loop_inn
    
    sbiw r24, 1
    brne delay_mS
    
    ret
    
    
    
ISR1:
    in r24, SREG	    ;save only SREG because r25 and r24 are renewed
    push r24
    
debounce:		    ;debounce avoidance algorithm (given)
    ldi r24, (1 << INTF1)
    out EIFR, r24
    
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    
    in r24, EIFR
    lsr r24
    lsr r24
    brcs debounce

    ser r24
    out PORTB, r24
    
    ldi r24, low(DEL_NU2)
    ldi r25, high(DEL_NU2)
    rcall delay_mS
    
    ldi r24, 0x01
    out PORTB, r24
    
end:   
    pop r24
    out SREG, r24
    
    ldi r24, low(DEL_NU1)	;renew counter
    ldi r25, high(DEL_NU1)
    
    reti