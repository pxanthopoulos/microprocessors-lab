.include "m328PBdef.inc"

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    
init:
    ser r23
    out DDRD, r23	;PORTD as output
    
    ldi r23, 0b00000001	;LED initialized at LSB
    set

;if T=1, move left
;if T=0, move right
    
check_T:
    brts shift_left	;if T is set (1), branch to shift left routine
    
shift_right:
    out PORTD, r23	;output 
    ldi r24, 0xf4	
    ldi r25, 0x01
    rcall wait_x_msec	;wait 500ms  
    lsr r23		;shift right
    cpi r23, 0x00	;check if the LED is at LSB   
    brne check_T	;if not, continue shifting right

change_to_left:
    set			;T becomes 1, so we shift left   
    ldi r24, 0xe8	
    ldi r25, 0x03
    rcall wait_x_msec	;wait extra 1000ms   
    ldi r23, 0x02	;new led position
    rjmp check_T	;continue
    
shift_left:
    out PORTD, r23	;output  
    ldi r24, 0xf4
    ldi r25, 0x01
    rcall wait_x_msec	;wait 500ms   
    lsl r23		;shift left
    cpi r23, 0x00	;check if the LED is at MSB
    brne check_T	;if not, continue shifting left

change_to_right:	
    clt			;T becomes 0, so we shift right
    ldi r24, 0xe8	
    ldi r25, 0x03
    rcall wait_x_msec	;wait extra 1000ms
    ldi r23, 0x40	;new led position
    rjmp check_T	;continue
   
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