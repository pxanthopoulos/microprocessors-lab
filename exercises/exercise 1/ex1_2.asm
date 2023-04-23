.include "m328PBdef.inc"

reset:
    ldi r24, low(RAMEND)
    out SPL, r24
    ldi r24, high(RAMEND)
    out SPH, r24
    
main:
    ldi r25, 0x55	;A
    ldi r24, 0x43	;B
    ldi r23, 0x22	;C
    ldi r22, 0x02	;D
    
    ldi r21, 0x06	;counter
    
    ldi r26, 0x02	;increments
    ldi r27, 0x03
    ldi r28, 0x04
    ldi r29, 0x05
    
loop:
    
    ;compute F0 first
    
    mov r20, r25	;r20 = A
    com r20		;r20 = A'
    
    mov r19, r24
    com r19		;r19 = B'
    
    and r20, r19	;r20 = A' * B'
    
    and r19, r22	;r19 = B' * D
    
    or r20, r19
    com r20		;r20 = F0
    
    ;compute F1
    
    mov r19, r25	;r19 = A
    or r19, r23		;r19 = A + C
    
    mov r18, r22	;r18 = D
    com r18		;r18 = D'
    or r18, r24		;r18 = B + D'
    
    and r19, r18	;r19 = F1
    
    ;change A,B,C,D
    
    add r25, r26
    add r24, r27
    add r23, r28
    add r22, r29
    
    ;loop condition
    
    subi r21, 0x01
    brne loop