#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

static volatile float ADC_val;
static volatile uint8_t integer;
static volatile uint8_t dec1;
static volatile uint8_t dec2;

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

void read_print_ADCval(void) {
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

    lcd_data(integer);
    lcd_data('.');
    lcd_data(dec1);
    lcd_data(dec2);

    _delay_ms(1000);     //result must be visible for some time before lcd inits again
    return;
}

void pb2(void) {
    OCR1A = 80;
    TCCR1A |= (1 << COM1A1);
    
    lcd_init();
    _delay_ms(2);
    
    lcd_data('2');
    lcd_data('0');
    lcd_data('%');
    
    while(1) {
        lcd_command(0xC0);
        read_print_ADCval();
        if ((PINB & 0x04) == 0x04) {
            break;
        }
    }
    
    return;
}

void pb3(void) {
    TCCR1A |= (1 << COM1A1);
    OCR1A = 160;
    
    lcd_init();
    _delay_ms(2);
    
    lcd_data('4');
    lcd_data('0');
    lcd_data('%');
    
    while(1) {
        lcd_command(0xC0);
        read_print_ADCval();
        if ((PINB & 0x08) == 0x08) {
            break;
        }
    }
    
    return;
}

void pb4(void) {
    TCCR1A |= (1 << COM1A1);
    OCR1A = 240;
    
    lcd_init();
    _delay_ms(2);
    
    lcd_data('6');
    lcd_data('0');
    lcd_data('%');
    
    while(1) {
        lcd_command(0xC0);
        read_print_ADCval();
        if ((PINB & 0x10) == 0x10) {
            break;
        }
    }
    
    return;
}

void pb5(void) {
    TCCR1A |= (1 << COM1A1);
    OCR1A = 320;
    
    lcd_init();
    _delay_ms(2);
    
    lcd_data('8');
    lcd_data('0');
    lcd_data('%');
    
    while(1) {
        lcd_command(0xC0);
        read_print_ADCval();
        if ((PINB & 0x20) == 0x20) {
            break;
        }
    }
    
    return;
}

int main(void) {
    DDRD = 0xFF;
    DDRB = 0x02;
    DDRC = 0x00;
    
    static unsigned char x;
    
    ICR1 = 399;
    TCNT1 = 0x0000;
    TCCR1A = (1 << WGM11);
    TCCR1B = (1 << WGM12) | (1 << WGM13 ) | (1 << CS11);
    
    ADMUX = (1 << REFS0) | (1 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0x00;
    DIDR0 = ~(1 << ADC1D);
    
    while (1) {
        x = PINB;
        if ((x & 0x04) != 0x04) {   //check if PB2 was pressed
            pb2();
        }
        if ((x & 0x08) != 0x08) {   //check if PB3 was pressed
            pb3();
        }
        if ((x & 0x10) != 0x10) {   //check if PB4 was pressed
            pb4();
        }
        if ((x & 0x20) != 0x20) {   //check if PB5 was pressed
            pb5();
        }
        lcd_init();
        _delay_ms(2);
        TCCR1A = (1 << WGM11);
    }
}
