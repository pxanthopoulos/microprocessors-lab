.include "m328PBdef.inc"

.org 0x0
    rjmp reset
    
reset:
    ldi r24, low(RAMEND)
    out SPL,r24
    ldi r24, high(RAMEND)
    out SPH,r24
    
    ser r24
    out DDRB, r24
    
    clr r24
    out DDRD, r24	    ;set PORTD as input
    
    ;WGM1[3:0] = 0xE for fast PWM, bottom = 0, top = ICR1
    ;CS1[2:0] = 0x0 for timer stop, 0x2 for prescaler 8 (default)
    ;COM1A[1:0] = 0 for no output, 0x2 for non inverting fast PWM

    
    
none:			    ;no waveform output but keep checking continuously
    ldi r24, 0x00	    ;reset timer
    sts TCNT1H, r24
    sts TCNT1L, r24
    
    ldi r24, (1 << WGM11)   ;no waveform and timer is stopped
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << WGM13 )
    sts TCCR1B, r24

recheck:
    in r24, PIND
    
    sbrs r24, 0	    ;sbrc for sim, sbrs for lab
    jmp PD0_125	    
    
    sbrs r24, 1	    ;sbrc for sim, sbrs for lab
    jmp PD1_250
    
    sbrs r24, 2	    ;sbrc for sim, sbrs for lab
    jmp PD2_500
    
    sbrs r24, 3	    ;sbrc for sim, sbrs for lab
    jmp PD3_1000
    
    rjmp recheck
    
    
    
PD0_125:
    ldi r24, (1 << WGM11) | (1 << COM1A1) ;fast PWM at 125Hz, non inverted, 50%DC, prescaler 8 
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << WGM13 ) | (1 << CS11)
    sts TCCR1B, r24
    
    ldi r24, 0x3E
    ldi r25, 0x7F
    sts ICR1H, r24
    sts ICR1L, r25	    ;ICR1 at 15999 for 125Hz
    
    ldi r24, 0x1F
    ldi r25, 0x40
    sts OCR1AH, r24
    sts OCR1AL, r25	    ;OCR1A at 8000 for 50% DC
    
release_PD0:		    ;wait for PD0 to be released
    in r24, PIND
    lsr r24
    brcc release_PD0	    ;if not yet released, check again
			    ;brcs for sim, brcc for lab
    jmp none		    ;if released, stop waveform
    
    
    
PD1_250:
    ldi r24, (1 << WGM11) | (1 << COM1A1) ;fast PWM at 125Hz, non inverted, 50%DC, prescaler 8 
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << WGM13 ) | (1 << CS11)
    sts TCCR1B, r24
    
    ldi r24, 0x1F
    ldi r25, 0x3F
    sts ICR1H, r24
    sts ICR1L, r25	    ;ICR1 at 7999 for 125Hz
    
    ldi r24, 0x0F
    ldi r25, 0xA0
    sts OCR1AH, r24
    sts OCR1AL, r25	    ;OCR1A at 4000 for 50% DC
    
release_PD1:		    ;wait for PD1 to be released
    in r24, PIND
    lsr r24
    lsr r24
    brcc release_PD1	    ;if not yet released, check again
			    ;brcs for sim, brcc for lab
    jmp none		    ;if released, stop waveform
    
    
    
PD2_500:
    ldi r24, (1 << WGM11) | (1 << COM1A1) ;fast PWM at 125Hz, non inverted, 50%DC, prescaler 8 
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << WGM13 ) | (1 << CS11)
    sts TCCR1B, r24
    
    ldi r24, 0x0F
    ldi r25, 0x9F
    sts ICR1H, r24
    sts ICR1L, r25	    ;ICR1 at 3999 for 125Hz
    
    ldi r24, 0x07
    ldi r25, 0xD0
    sts OCR1AH, r24
    sts OCR1AL, r25	    ;OCR1A at 2000 for 50% DC
    
release_PD2:		    ;wait for PD2 to be released
    in r24, PIND
    lsr r24
    lsr r24
    lsr r24
    brcc release_PD2	    ;if not yet released, check again
			    ;brcs for sim, brcc for lab
    jmp none		    ;if released, stop waveform
    
    
    
PD3_1000:
    ldi r24, (1 << WGM11) | (1 << COM1A1) ;fast PWM at 125Hz, non inverted, 50%DC, prescaler 8 
    sts TCCR1A, r24
    
    ldi r24, (1 << WGM12) | (1 << WGM13 ) | (1 << CS11)
    sts TCCR1B, r24
    
    ldi r24, 0x07
    ldi r25, 0xCF
    sts ICR1H, r24
    sts ICR1L, r25	    ;ICR1 at 1999 for 125Hz
    
    ldi r24, 0x03
    ldi r25, 0xE8
    sts OCR1AH, r24
    sts OCR1AL, r25	    ;OCR1A at 1000 for 50% DC
    
release_PD3:		    ;wait for PD3 to be released
    in r24, PIND
    lsr r24
    lsr r24
    lsr r24
    lsr r24
    brcc release_PD3	    ;if not yet released, check again
			    ;brcs for sim, brcc for lab
    jmp none		    ;if released, stop waveform