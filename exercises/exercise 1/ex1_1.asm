.include "m328PBdef.inc"

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    
main:
    ldi r24, 0x11
    ldi r25, 0x00
    rcall wait_x_msec
    rjmp main
    
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