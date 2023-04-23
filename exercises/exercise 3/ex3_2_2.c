#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    
    static unsigned char table[13] = {5, 26, 46, 66, 87, 107, 128, 148, 168, 189, 209, 230, 250};
    static unsigned char x;
    
    TCCR1A = (1<<WGM10) | (1<<COM1A1);
    TCCR1B = (1<<WGM12) | (1<<CS11);
    
    DDRB |= 0b00111111;
    
    static int counter = 7;
    OCR1AL = table[counter - 1];
    
    DDRD = 0b00000000;
    
    while (1) {                 // != on the conditions for lab, == for sim
        x = PIND;
        if ((x & 0x02) != 0x02) {          //check if PD1 was pressed
            while ((PIND & 0x02) != 0x02);    //wait for PD1 to be released
            _delay_ms(10);                  //debounce avoidance
            if (counter != 13) {
                counter++;
                OCR1AL = table[counter - 1];
            }
        }
        else if ((x & 0x04) != 0x04) {     //check if PD2 was pressed
            while ((PIND & 0x04) != 0x04);    //wait for PD2 to be released
            _delay_ms(10);                  //debounce avoidance
            if (counter != 1) {
                counter--;
                OCR1AL = table[counter - 1];
            }
        }
    }
}