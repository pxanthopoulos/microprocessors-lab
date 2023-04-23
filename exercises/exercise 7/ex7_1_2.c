#define F_CPU 16000000UL

#include<avr/io.h>
#include<util/delay.h>

uint8_t one_wire_reset(void)
{
    DDRD |= 0x10;
    PORTD &= 0xEF;
    _delay_us(480);
    
    DDRD &= 0xEF;
    PORTD &= 0xEF;
    
    _delay_us(100);
    
    uint8_t temp = PIND;
    _delay_us(380);
    
    temp &= 0x10;       //isolate PD4 bit

    if (temp == 0x10) 
    {
        return 0;
    }
    return 1;
}

uint8_t one_wire_receive_bit(void)
{
    DDRD |= 0x10;
    PORTD &= 0xEF;
    
    _delay_us(2);
    
    DDRD &= 0xEF;
    PORTD &= 0xEF;
    
    _delay_us(10);
    
    uint8_t temp = PIND;
    
    temp &= 0x10;
    temp = temp >> 4;
    
    _delay_us(49);
    
    return temp;
}

void one_wire_transmit_bit(uint8_t val)
{
    DDRD |= 0x10;
    PORTD &= 0xEF;
    
    _delay_us(2);
    
    val &= 0x01;
    
    if(val == 0x01)
    {
        PORTD |= 0x10;
    } else if(val == 0x00)
    {
        PORTD &= 0xEF;
    }
    
    _delay_us(58);
    
    DDRD &= 0xEF;
    PORTD &= 0xEF;
    
    _delay_us(1);
    
    return;
}

uint8_t one_wire_receive_byte(void)
{
    uint8_t temp, ret = 0x00;
    
    for(int i=0; i<8; i++)
    {
        temp = one_wire_receive_bit();
        temp = temp << i;
        ret |= temp;
    }
    return ret;
}

void one_wire_transmit_byte(uint8_t byte)
{
    uint8_t temp;
    
    for(int i=0; i<8; i++)
    {
        temp = byte >> i;
        temp &= 0x01;
        one_wire_transmit_bit(temp);
    }
    return;
}

static volatile uint16_t measurement;
static volatile uint8_t lsbyte, msbyte;

uint16_t read_temp(void)
{
    uint16_t ret;
    
    if(one_wire_reset() == 0)
    {
        return 0x8000;
    } else
    {
        one_wire_transmit_byte(0xCC);
    
        one_wire_transmit_byte(0x44);

        while(one_wire_receive_bit() == 0x00)
        {
            _delay_ms(750);
        }   // wait for the measurement to become available

        if(one_wire_reset() == 0)
        {
            return 0x8000;
        } else
        {
            one_wire_transmit_byte(0xCC);

            one_wire_transmit_byte(0xBE);

            lsbyte = one_wire_receive_byte();

            msbyte = one_wire_receive_byte();
            
            ret = msbyte;
            ret  = ret << 8;
            ret += lsbyte;
            return ret;
        }       
    }    
}

int main(void)
{
    while(1)
    {
        measurement = read_temp();
        _delay_ms(1000);
    }
}