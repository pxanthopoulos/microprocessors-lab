.include "m328PBdef.inc"

.org 0x0
    rjmp reset
.org 0x4
    rjmp ISR1
.org 0x1A
    rjmp timer1
    
reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPL, r24
    
    ldi r26,(1<<TOIE1)	    ;enable timer1 overflow interrupt
    sts TIMSK1, r26
    
    ldi r26, (1 << ISC11) | (1 << ISC10)
    sts EICRA, r26
    
    ldi r26, (1 << INT1)
    out EIMSK, r26
			;INT1 is enabled at rising edge   
    ldi r26 ,(1<<CS12) | (0<<CS11) | (1<<CS10) ; CK/1024
    sts TCCR1B, r26 
    
    sei
    
init:    
    ser r26		;PORT B is set as output
    out DDRB, r26
    clr r26
    out DDRC, r26	;PORT C is set as input
    
loop:
    in r20, PINC	;wait for PC5 to be pressed
    sbrc r20, 5		;sbrs for sim, sbrc for lab
    rjmp loop
    
loop1:
    in r20, PINC	;wait for PC5 to be released
    sbrs r20, 5		;sbrc for sim, sbrs for lab
    rjmp loop1
    
    ldi r26, HIGH(3036)    ;reset TCNT1
    sts TCNT1H, r26
    ldi r26, LOW(3036)
    sts TCNT1L, r26 
    
    ser r26		    ;turn on all leds for 500ms
    out PORTB, r26
    
    ldi r24, LOW(16*500)
    ldi r25, HIGH(16*500)
    rcall delay_mS
    
    ldi r26, 0x01	    ;turn on only LSB
    out PORTB, r26

    rjmp loop
    
    
    
ISR1:
    push r24
    push r25
    in r26, SREG
    push r26
    
debounce:		    ;debounce avoidance algorithm (given)
    ldi r26, (1 << INTF1)
    out EIFR, r26
    
    ldi r24, low(16*5)
    ldi r25, high(16*5)
    rcall delay_mS
    
    in r26, EIFR
    lsr r26
    lsr r26
    brcs debounce
    
    ldi r26, HIGH(3036)    ;reset TCNT1
    sts TCNT1H, r26
    ldi r26, LOW(3036)
    sts TCNT1L, r26
    
    ser r26		    ;turn on all leds for 500ms
    out PORTB, r26
    
    ldi r24, LOW(16*500)
    ldi r25, HIGH(16*500)
    rcall delay_mS
    
    ldi r26, 0x01	    ;turn on only LSB
    out PORTB, r26
    
    pop r26
    out SREG, r26
    pop r25
    pop r24
    reti
    
    
    
timer1:
    in r26, SREG
    push r26
    
    clr r26
    out PORTB, r26
    
    pop r26
    out SREG, r26
    reti
    
    
    
    
delay_mS:	    ;delay of 1000*DEL_NU_X + 6 cycles
    ldi r23, 249     ;default is 249, for simulator set to 15
loop_inn:
    dec r23
    nop
    brne loop_inn
    
    sbiw r24, 1
    brne delay_mS
    
    ret