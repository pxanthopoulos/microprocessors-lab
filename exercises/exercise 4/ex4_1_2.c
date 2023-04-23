#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

void write_2_nibbles(uint8_t c) {
    uint8_t temp = c;
    uint8_t prev = PIND;
    prev &= 0x0F;
    c &= 0xF0;
    c |= prev;
    PORTD = c;
    PORTD |= 0x08;
    PORTD &= 0xF7;
    
    c = temp;
    c &= 0x0F;
    c = c << 4;
    c |= prev;
    PORTD = c;
    PORTD |= 0x08;
    PORTD &= 0xF7;
    
    return;
}

void lcd_data(uint8_t c) {
    PORTD |= 0x04;
    write_2_nibbles(c);
    _delay_us(100);
    return;
}

void lcd_command(uint8_t c) {
    PORTD &= 0xFB;
    write_2_nibbles(c);
    _delay_us(100);
    return;
}

void lcd_init(void) {
    _delay_ms(40);
    
    PORTD = 0x30;
    PORTD |= 0x08;
    PORTD &= 0xF7;
    _delay_us(100);
    
    PORTD = 0x30;
    PORTD |= 0x08;
    PORTD &= 0xF7;
    _delay_us(100);
    
    PORTD = 0x20;
    PORTD |= 0x08;
    PORTD &= 0xF7;
    _delay_us(100);
    
    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x01);
    _delay_us(5000);
    
    lcd_command(0x06);
    return;
}

static volatile float ADC_val;
static volatile uint8_t integer;
static volatile uint8_t dec1;
static volatile uint8_t dec2;

int main(void) {
    DDRD = 0xFF;
    DDRC = 0x00;
    
    ADMUX = (1 << REFS0) | (1 << MUX1);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0x00;
    DIDR0 = ~(1 << ADC2D);
    
    while (1) {
        ADCSRA |= (1 << ADSC);
        while ((ADCSRA & (1 << ADSC)) == (1 << ADSC)) {
            _delay_us(10);
        }
        ADC_val = ADC;
        ADC_val = (ADC_val * 5) / 1024;
        
        integer = (uint8_t)ADC_val;
        float decimal = ADC_val - integer;
        dec1 = (uint8_t)(decimal * 10);
        dec2 = (uint8_t)(((decimal * 10) - dec1) * 10);
        
        integer |= 0x30;
        dec1 |= 0x30;
        dec2 |= 0x30;
        
        lcd_init();
        _delay_ms(2);
        
        lcd_data(integer);
        lcd_data('.');
        lcd_data(dec1);
        lcd_data(dec2);
        
        _delay_ms(100);     //result must be visible for some time before lcd inits again
    }  
}