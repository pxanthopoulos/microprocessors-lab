#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

static volatile uint8_t prev_state = 0;
static volatile uint16_t ADC_val;
static volatile uint8_t cnt;

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

void print_clear(void) {
    lcd_init();
    _delay_ms(2);
    lcd_data('C');
    lcd_data('L');
    lcd_data('E');
    lcd_data('A');
    lcd_data('R');
    return;
}

void print_gas_detected(void) {
    lcd_init();
    _delay_ms(2);
    lcd_data('G');
    lcd_data('A');
    lcd_data('S');
    lcd_data(' ');
    lcd_data('D');
    lcd_data('E');
    lcd_data('T');
    lcd_data('E');
    lcd_data('C');
    lcd_data('T');
    lcd_data('E');
    lcd_data('D');
    return;
}

void blink(uint8_t temp) {
    if (cnt < 4) {
        PORTB = temp;
        cnt++;
    }else {
        PORTB = 0x00;
        cnt++;
    }
    if (cnt == 8) {
        cnt = 0;
    }
    return;
}

void level1(void) {
    PORTB = 0x01;
    if (prev_state != 0) {
        print_clear();
        prev_state = 0;
    }
    return;
}

void level2(void) {
    blink(0x03);
    if (prev_state != 1) {
        print_gas_detected();
        prev_state = 1;
    }
    return;
}

void level3(void) {
    blink(0x07);
    if (prev_state != 1) {
        print_gas_detected();
        prev_state = 1;
    }
    return;
}

void level4(void) {
    blink(0x0F);
    if (prev_state != 1) {
        print_gas_detected();
        prev_state = 1;
    }
    return;
}

void level5(void) {
    blink(0x1F);
    if (prev_state != 1) {
        print_gas_detected();
        prev_state = 1;
    }
    return;
}

void level6(void) {
    blink(0x3F);
    if (prev_state != 1) {
        print_gas_detected();
        prev_state = 1;
    }
    return;
}

int main(void) {
    DDRD = 0xFF;
    DDRB = 0xFF;
    DDRC = 0x00;
    
    ADMUX = (1 << REFS0) | (1 << MUX1) | (1 << MUX0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0x00;
    DIDR0 = ~(1 << ADC3D);
    
    cnt = 0;
    prev_state = 9;
    print_clear();
    
    while (1) {
        ADCSRA |= (1 << ADSC);
        while ((ADCSRA & (1 << ADSC)) == (1 << ADSC)) {
            _delay_us(10);
        }
        ADC_val = ADC;
        
        if (ADC_val < 205) {
            level1();
        }else if (ADC_val >= 205 && ADC_val < 391) {
            level2();
        }else if (ADC_val >= 391 && ADC_val < 576) {
            level3();
        }else if (ADC_val >= 576 && ADC_val < 761) {
            level4();
        }else if (ADC_val >= 761 && ADC_val < 947) {
            level5();
        }else {
            level6();
        }
        _delay_ms(100);
    }
}