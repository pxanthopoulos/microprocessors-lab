#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

ISR(INT1_vect)
{
    while (EIFR==0x02) {
        EIFR = 0x02;
        _delay_ms(5);
    }
    TCNT1 = 3036;
    PORTB = 0xFF;
    _delay_ms(500);
    PORTB = 0x01;
}
        
ISR(TIMER1_OVF_vect)
{
    PORTB = 0x00;
}

static unsigned char x;

int main(void)
{
    TIMSK1 = (1<<TOIE1);                    //timer1 overflow interrupt enabled
    
    EICRA = (1 << ISC11) | (1 << ISC10);
    
    EIMSK = (1 << INT1);                    //int1 enabled at rising edge
        
    TCCR1B = (1<<CS12) | (0<<CS11) | (1<<CS10);     //CK/1024
    
    sei();
            
    DDRB = 0xFF;        //PORTB as output
    DDRC = 0x00;        //PORTC as input
    
    PORTB = 0x00;
    
    while (1)
    {   
        x = PINC;
        if ((x & 0x20) == 0x20) {          //check if PC5 was pressed
            while ((PINC & 0x20) == 0x20);    //wait for PC5 to be released
            _delay_ms(10);                  //debounce avoidance
            TCNT1 = 3036;
            PORTB = 0xFF;
            _delay_ms(500);
            PORTB = 0x01;
        }
    }
}