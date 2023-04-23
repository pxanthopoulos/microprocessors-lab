.include "m328PBdef.inc"
    
.def temp = r16
.def prev_state = r17	;prev_state is 0x00 if clear, prev_state is 0x01 if gas detected
			;useful for skipping lcd_init and output
.def cnt = r18
.def ADC_L = r21
.def ADC_H = r22
    
.org 0x00
    jmp reset

.org 0x2A
    jmp adc_ready
    
reset:
    ldi temp, high(RAMEND)
    out SPH, temp
    ldi temp, low(RAMEND)
    out SPL,temp
    
    ldi temp, 0xFF
    out DDRB, temp
    out DDRD, temp
    ldi temp, 0x00
    out DDRC, temp	;Set PORTC as input
    
    ; REFSn[1:0]=01 => select Vref=5V, MUXn[4:0]=0011 => select ADC3(pin PC3),
    ; ADLAR=0 => right adjust the ADC result
    ldi temp, (1 << REFS0) | (1 << MUX1) | (1 << MUX0)
    sts ADMUX, temp
    
    ; ADEN=1 => ADC Enable, ADCS=0 => No Conversion yet
    ; ADIE=1 => enable adc interrupt, ADPS[2:0]=111 => fADC=16MHz/128=125KHz
    ; ADATE = 1, for free running
    ldi temp, (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADATE)
    sts ADCSRA, temp
    
    ldi temp, 0x00
    sts ADCSRB, temp

    ldi temp, 0xFF
    ldi r18, (1 << ADC3D)   ;r18 is used as dummy var
    eor temp, r18   
    sts DIDR0, temp	    ;disable all other ADC inputs
    
    sei			    ;enable all interrupts
    
    lds temp, ADCSRA	    ;start first conversion
    ori temp, (1<<ADSC)
    sts ADCSRA, temp
      
    ldi prev_state, 0x00	    ;original state is clear
    rcall print_clear
    
main:   
    rjmp main
    
adc_ready:
    lds ADC_L, ADCL
    lds ADC_H, ADCH
    
    cpi ADC_H, 0x00
    breq lo1
    rjmp next1
lo1:			;ADC_H = 0x00
    cpi ADC_L, 0xCD
    brlo level1		;ADC_H = 0x00 and ADC_L < 0xCD
    rjmp level2		;ADC_H = 0x00 and ADC_L >= 0xCD
next1:
    cpi ADC_H, 0x01	
    breq lo2
    rjmp next2
lo2:			;ADC_H = 0x01
    cpi ADC_L, 0x87	 
    brlo level2		;ADC_H = 0x01 and ADC_L < 0x87
    rjmp level3		;ADC_H = 0x01 and ADC_L >= 0x87
next2:
    cpi ADC_H, 0x02
    breq lo3
    rjmp next3
lo3:			;ADC_H = 0x02
    cpi ADC_L, 0x40
    brlo level3		;ADC_H = 0x02 and ADC_L < 0x40
    cpi ADC_L, 0xF9
    brlo level4		;ADC_H = 0x02 and ADC_L < 0xF9 (also ADC_L >= 0x40
    rjmp level5		;ADC_H = 0x02 and ADC_L >= 0xF9
next3:			;ADC_H must be 0x03
    cpi ADC_L, 0xB3
    brlo level5		;ADC_H = 0x03 and ADC_L < 0xB3
    rjmp level6		;ADC_H = 0x03 and ADC_L >= 0xB3

level1:
    ldi temp, 0x01
    out PORTB, temp
    sbrc prev_state, 0	;if previous state was clear, dont output
    rcall print_clear
    cbr prev_state, 1	;set previous state
    rcall delay
    
level2:
    ldi temp, 0x03
    rcall blink
    sbrs prev_state, 0	;if previous state was gas detected, dont output
    rcall print_gas_detected
    sbr prev_state, 1
    rcall delay
    
level3:
    ldi temp, 0x07
    rcall blink
    sbrs prev_state, 0
    rcall print_gas_detected
    sbr prev_state, 1
    rcall delay
    
level4:
    ldi temp, 0x0F
    rcall blink
    sbrs prev_state, 0
    rcall print_gas_detected
    sbr prev_state, 1
    rcall delay
    
level5:
    ldi temp, 0x1F
    rcall blink
    sbrs prev_state, 0
    rcall print_gas_detected
    sbr prev_state, 1
    rcall delay
    
level6:
    ldi temp, 0x3F
    rcall blink
    sbrs prev_state, 0
    rcall print_gas_detected
    sbr prev_state, 1
    rcall delay
    
blink:
    cpi cnt, 0x04
    brlo lower
    rjmp higher
lower:
    out PORTB, temp
    inc cnt
    rjmp end1
higher:
    ldi temp, 0x00
    out PORTB, temp
    inc cnt
end1:
    sbrc cnt, 3
    ldi cnt, 0x00
    ret
    
delay:
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    rcall wait_x_msec
    reti
    
print_clear:
    rcall lcd_init
    ldi r24, low(16*2)
    ldi r25, high(16*2)
    rcall wait_x_msec
    
    ldi r24, 'C'
    rcall lcd_data
    
    ldi r24, 'L'
    rcall lcd_data
    
    ldi r24, 'E'
    rcall lcd_data
    
    ldi r24, 'A'
    rcall lcd_data
    
    ldi r24, 'R'
    rcall lcd_data
    
    ret
    
print_gas_detected:
    rcall lcd_init
    ldi r24, low(16*2)
    ldi r25, high(16*2)
    rcall wait_x_msec
    
    ldi r24, 'G'
    rcall lcd_data
    
    ldi r24, 'A'
    rcall lcd_data
    
    ldi r24, 'S'
    rcall lcd_data
    
    ldi r24, ' '
    rcall lcd_data
    
    ldi r24, 'D'
    rcall lcd_data
    
    ldi r24, 'E'
    rcall lcd_data
    
    ldi r24, 'T'
    rcall lcd_data
    
    ldi r24, 'E'
    rcall lcd_data
    
    ldi r24, 'C'
    rcall lcd_data
    
    ldi r24, 'T'
    rcall lcd_data
    
    ldi r24, 'E'
    rcall lcd_data
    
    ldi r24, 'D'
    rcall lcd_data
    
    ret 
    
write_2_nibbles:
    push r24	    ;r24 contains data to be sent
    in r25, PIND
    andi r25, 0x0f
    andi r24, 0xf0
    add r24, r25
    out PORTD, r24
    sbi PORTD, 3
    nop
    nop
    cbi PORTD, 3
    pop r24
    swap r24
    andi r24, 0xf0
    add r24, r25
    out PORTD, r24
    sbi PORTD, 3
    nop
    nop
    cbi PORTD, 3
    ret
 
    
lcd_data:
    sbi PORTD, 2
    rcall write_2_nibbles
    ldi r24, 100
    ldi r25, 0
    rcall wait_usec
    ret
    

lcd_command:
    cbi PORTD, 2
    rcall write_2_nibbles
    ldi r24, 100
    ldi r25, 0
    rcall wait_usec
    ret

lcd_init:
    ldi r24, low(16*40)
    ldi r25, high(16*40)
    rcall wait_x_msec
    ldi r24, 0x30
    out PORTD, r24
    sbi PORTD, 3
    nop
    nop
    cbi PORTD, 3
    ldi r24, 100
    ldi r25, 0
    rcall wait_usec
    ldi r24, 0x30
    out PORTD, r24
    sbi PORTD, 3
    nop
    nop
    cbi PORTD, 3
    ldi r24, 100
    ldi r25, 0
    rcall wait_usec
    ldi r24, 0x20
    out PORTD, r24
    sbi PORTD, 3
    nop
    nop
    cbi PORTD, 3
    ldi r24, 100
    ldi r25, 0
    rcall wait_usec
    ldi r24, 0x28
    rcall lcd_command
    ldi r24, 0x0c
    rcall lcd_command
    ldi r24, 0x01
    rcall lcd_command
    ldi r24, low(5000)
    ldi r25, high(5000)
    rcall wait_usec
    ldi r24, 0x06
    rcall lcd_command
    ret  
       
wait_x_msec:
    rcall delay_986u
    sbiw r24, 1
    breq end
    
    rjmp help1

help1:
    rjmp help2		;2 cc
    
help2:
    rjmp help3		;2 cc

help3:
    rjmp wait_x_msec	;2 cc

end:
    ret
    
delay_986u:
    ldi r26, 98
    
loop_986u:
    rcall wait_4u
    dec r26
    brne loop_986u
    
    nop
    nop
    ret

wait_4u:
    ret

    
wait_usec:
    sbiw r24 ,1
    nop 
    nop
    nop
    nop
    nop
    nop
    nop 
    nop
    nop
    nop
    brne wait_usec
    ret