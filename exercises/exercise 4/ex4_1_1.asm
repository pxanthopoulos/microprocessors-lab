.include "m328PBdef.inc"
    
.def temp = r16
.def cnt = r17
.def integer = r18
.def dec1 = r19
.def dec2 = r20
.def ADC_L = r21
.def ADC_H = r22
    
.org 0x00
    jmp reset
    
.org 0x1A
    jmp timer1
    
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
    out DDRC, temp
    
    ; REFSn[1:0]=01 => select Vref=5V, MUXn[4:0]=0010 => select ADC2(pin PC2),
    ; ADLAR=0 => right adjust the ADC result
    ldi temp, (1 << REFS0) | (1 << MUX1) 
    sts ADMUX, temp
    
    ; ADEN=1 => ADC Enable, ADCS=0 => No Conversion yet
    ; ADIE=1 => enable adc interrupt, ADPS[2:0]=111 => fADC=16MHz/128=125KHz
    ldi temp, (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADATE)
    sts ADCSRA, temp
    
    ldi temp, 0x00
    sts ADCSRB, temp

    ldi temp, 0xFF
    ldi cnt, (1 << ADC2D)   ;cnt is used as dummy var
    eor temp, cnt   
    sts DIDR0, temp	;disable all other ADC inputs
    
    ldi temp,(1<<TOIE1)	    ;enable timer1 overflow interrupt
    sts TIMSK1, temp
    
    ldi r26 ,(1<<CS12) | (0<<CS11) | (1<<CS10) ; CK/1024
    sts TCCR1B, r26
    
    ldi cnt, 0x00	    ;initalize counter at 0
    out PORTB, cnt
    
    sei			    ;enable all interrupts
    
    lds temp, ADCSRA	    ;start first conversion
    ori temp, (1<<ADSC)
    sts ADCSRA, temp
    
    ldi temp, HIGH(49910)    ;reset TCNT1
    sts TCNT1H, temp
    ldi temp, LOW(49910)
    sts TCNT1L, temp 
    
main:
    rjmp main
    
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
    
    
timer1:
    cpi cnt, 0x3F
    brne incr
    ldi cnt, 0x00
    rjmp outb
incr:
    inc cnt
outb:
    out PORTB, cnt
    ldi temp, HIGH(49910)    ;reset TCNT1
    sts TCNT1H, temp
    ldi temp, LOW(49910)
    sts TCNT1L, temp 
    reti
    
    
adc_ready:
    lds ADC_L, ADCL
    lds ADC_H, ADCH
    
    rcall mul5
    mov integer, ADC_H
    lsr integer
    lsr integer		;integer contains now the integer part
    
    andi ADC_H, 0x03	;delete integer part
    
    rcall mul5
    lsl ADC_L
    rol ADC_H
    mov dec1, ADC_H
    lsr dec1
    lsr dec1		;dec1 contains now the first decimal point
    
    andi ADC_H, 0x03	;delete first decimal part
    
    rcall mul5
    lsl ADC_L
    rol ADC_H
    mov dec2, ADC_H
    lsr dec2
    lsr dec2		;dec2 contains now the second decimal point
    
    ori integer, 0x30	; to ascii
    ori dec1, 0x30
    ori dec2, 0x30
    
    rcall lcd_init
    ldi r24, low(16*2)
    ldi r25, high(16*2)
    rcall wait_x_msec
    
    mov r24, integer
    call lcd_data
    
    ldi r24, '.'
    call lcd_data
    
    mov r24, dec1
    call lcd_data
    
    mov r24, dec2
    call lcd_data
    
    ldi r24, low(16*100)
    ldi r25, high(16*100)
    rcall wait_x_msec
    
    reti
    

mul5:
    mov r14, ADC_L
    mov r15, ADC_H
    
    lsl ADC_L
    rol ADC_H
    
    lsl ADC_L
    rol ADC_H
    
    add ADC_L, r14
    adc ADC_H, r15
    
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