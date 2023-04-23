/*
 * File:   ex2_3_2.c
 * Author: panos
 *
 * Created on November 1, 2022, 12:07 AM
 */


#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

volatile int counter = 3496;

ISR (INT1_vect) {
    PORTB = 0xFF;
    _delay_ms(500);
    PORTB = 0x01;
    counter = 3496;
}

int main(void) {
    EICRA = (1 << ISC11) | (1 << ISC10);
    EIMSK = (1 << INT1);
    sei();

    DDRB = 0xFF;
    PORTB = 0x00;

    while (1) {
        while (counter > 0) {
            _delay_ms(1);
            counter = counter - 1;
        }
        PORTB = 0x00;
        counter = 3496;
    }
}