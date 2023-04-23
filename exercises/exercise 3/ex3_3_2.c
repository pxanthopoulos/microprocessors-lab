#define F_CPU 16000000UL
#include <avr/io.h>

void pd0(void) {
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << WGM13 ) | (1 << CS11);
    ICR1 = 0x3E7F;
    OCR1A = 0x1F40;
    
    while ((PIND & 0x01) != 0x01);
    return;
}

void pd1(void) {
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << WGM13 ) | (1 << CS11);
    ICR1 = 0x1F3F;
    OCR1A = 0x0FA0;
    
    while ((PIND & 0x02) != 0x02);
    return;
}

void pd2(void) {
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << WGM13 ) | (1 << CS11);
    ICR1 = 0x0F9F;
    OCR1A = 0x07D0;
    
    while ((PIND & 0x04) != 0x04);
    return;
}

void pd3(void) {
    TCCR1A = (1 << WGM11) | (1 << COM1A1);
    TCCR1B = (1 << WGM12) | (1 << WGM13 ) | (1 << CS11);
    ICR1 = 0x07CF;
    OCR1A = 0x03E8;
    
    while ((PIND & 0x08) != 0x08);
    return;
}

int main(void) {
    
    DDRD = 0b00000000;
    DDRB |= 0b00111111;
    static unsigned char x;
    
    while (1) {
        TCNT1 = 0x0000;
        TCCR1A = (1 << WGM11);
        TCCR1B = (1 << WGM12) | (1 << WGM13 );
        while (1) {
            x = PIND;
            if ((x & 0x01) != 0x01) {   //check if PD0 was pressed
                pd0();
                break;
            }
            if ((x & 0x02) != 0x02) {   //check if PD1 was pressed
                pd1();
                break;
            }
            if ((x & 0x04) != 0x04) {   //check if PD2 was pressed
                pd2();
                break;
            }
            if ((x & 0x08) != 0x08) {   //check if PD3 was pressed
                pd3();
                break;
            }
        }
    }
}
