#define F_CPU 16000000UL

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include<string.h>
#include<stdio.h>

#define PCA9555_0_ADDRESS 0x40 //A0=A1=A2=0 by hardware
#define TWI_READ 1 // reading from twi device
#define TWI_WRITE 0 // writing to twi device
#define SCL_CLOCK 100000L // twi clock in Hz
//Fscl=Fcpu/(16+2*TWBR0_VALUE*PRESCALER_VALUE)
#define TWBR0_VALUE ((F_CPU/SCL_CLOCK)-16)/2

// PCA9555 REGISTERS
typedef enum {
    REG_INPUT_0 = 0,
    REG_INPUT_1 = 1,
    REG_OUTPUT_0 = 2,
    REG_OUTPUT_1 = 3,
    REG_POLARITY_INV_0 = 4,
    REG_POLARITY_INV_1 = 5,
    REG_CONFIGURATION_0 = 6,
    REG_CONFIGURATION_1 = 7,
} PCA9555_REGISTERS;

//----------- Master Transmitter/Receiver -------------------
#define TW_START 0x08
#define TW_REP_START 0x10

//---------------- Master Transmitter ----------------------
#define TW_MT_SLA_ACK 0x18
#define TW_MT_SLA_NACK 0x20
#define TW_MT_DATA_ACK 0x28

//---------------- Master Receiver ----------------
#define TW_MR_SLA_ACK 0x40
#define TW_MR_SLA_NACK 0x48
#define TW_MR_DATA_NACK 0x58

#define TW_STATUS_MASK 0b11111000
#define TW_STATUS (TWSR0 & TW_STATUS_MASK)

//initialize TWI clock
void twi_init(void)
{
    TWSR0 = 0; // PRESCALER_VALUE=1
    TWBR0 = TWBR0_VALUE; // SCL_CLOCK 100KHz
}

// Read one byte from the twi device (request more data from device)
unsigned char twi_readAck(void)
{
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    while(!(TWCR0 & (1<<TWINT)));
    return TWDR0;
}

// Read one byte from the twi device (dont request more data from device)
unsigned char twi_readNak(void)
{
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    while(!(TWCR0 & (1<<TWINT)));
    return TWDR0;
}

// Issues a start condition and sends address and transfer direction.
// return 0 = device accessible, 1= failed to access device
unsigned char twi_start(unsigned char address)
{
    uint8_t twi_status;
    // send START condition
    TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));
    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;
    if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) return 1;
    // send device address
    TWDR0 = address;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    // wail until transmission completed and ACK/NACK has been received
    while(!(TWCR0 & (1<<TWINT)));
    // check value of TWI Status Register.
    twi_status = TW_STATUS & 0xF8;
    if ( (twi_status != TW_MT_SLA_ACK) && (twi_status != TW_MR_SLA_ACK) )
    {
        return 1;
    }
    return 0;
}

// Send start condition, address, transfer direction.
// Use ack polling to wait until device is ready
void twi_start_wait(unsigned char address)
{
    uint8_t twi_status;
    while ( 1 )
    {
        // send START condition
        TWCR0 = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
        // wait until transmission completed
        while(!(TWCR0 & (1<<TWINT)));
        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status != TW_START) && (twi_status != TW_REP_START)) continue;
        // send device address
        TWDR0 = address;
        TWCR0 = (1<<TWINT) | (1<<TWEN);
        // wail until transmission completed
        while(!(TWCR0 & (1<<TWINT)));
        // check value of TWI Status Register.
        twi_status = TW_STATUS & 0xF8;
        if ( (twi_status == TW_MT_SLA_NACK )||(twi_status ==TW_MR_DATA_NACK) )
        {
            /* device busy, send stop condition to terminate write operation */
            TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
            // wait until stop condition is executed and bus released
            while(TWCR0 & (1<<TWSTO));
            continue;
        }
        break;
    }
}

// Send one byte to twi device, Return 0 if write successful or 1 if write failed
unsigned char twi_write( unsigned char data )
{
    // send data to the previously addressed device
    TWDR0 = data;
    TWCR0 = (1<<TWINT) | (1<<TWEN);
    // wait until transmission completed
    while(!(TWCR0 & (1<<TWINT)));
    if( (TW_STATUS & 0xF8) != TW_MT_DATA_ACK) return 1;
    return 0;
}

// Send repeated start condition, address, transfer direction
//Return: 0 device accessible
// 1 failed to access device
unsigned char twi_rep_start(unsigned char address)
{
    return twi_start( address );
}

// Terminates the data transfer and releases the twi bus
void twi_stop(void)
{
    // send stop condition
    TWCR0 = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    // wait until stop condition is executed and bus released
    while(TWCR0 & (1<<TWSTO));
}

void PCA9555_0_write(PCA9555_REGISTERS reg, uint8_t value)
{
    twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
    twi_write(reg);
    twi_write(value);
    twi_stop();
}

uint8_t PCA9555_0_read(PCA9555_REGISTERS reg)
{
    uint8_t ret_val;
    twi_start_wait(PCA9555_0_ADDRESS + TWI_WRITE);
    twi_write(reg);
    twi_rep_start(PCA9555_0_ADDRESS + TWI_READ);
    ret_val = twi_readNak();
    twi_stop();
    return ret_val;
}

void write_2_nibbles(uint8_t c)
{
    uint8_t temp = c;
    uint8_t prev = PCA9555_0_read(REG_INPUT_0);
    prev &= 0x0F;
    c &= 0xF0;
    c |= prev;
    PCA9555_0_write(REG_OUTPUT_0, c);
    c |= 0x08;
    PCA9555_0_write(REG_OUTPUT_0, c);
    c &= 0xF7;
    PCA9555_0_write(REG_OUTPUT_0, c);
    
    c = temp;
    c &= 0x0F;
    c = c << 4;
    c |= prev;
    PCA9555_0_write(REG_OUTPUT_0, c);
    c |= 0x08;
    PCA9555_0_write(REG_OUTPUT_0, c);
    c &= 0xF7;
    PCA9555_0_write(REG_OUTPUT_0, c);
    
    return;
}

void lcd_data(uint8_t c)
{
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp |= 0x04;
    PCA9555_0_write(REG_OUTPUT_0, temp);
    write_2_nibbles(c);
    _delay_us(100);
    return;
}

void lcd_command(uint8_t c)
{
    uint8_t temp = PCA9555_0_read(REG_INPUT_0);
    temp &= 0xFB;
    PCA9555_0_write(REG_OUTPUT_0, temp);
    write_2_nibbles(c);
    _delay_us(100);
    return;
}

void lcd_init(void) 
{
    _delay_ms(40);
    
    PCA9555_0_write(REG_OUTPUT_0, 0x30);
    PCA9555_0_write(REG_OUTPUT_0, 0x38);
    PCA9555_0_write(REG_OUTPUT_0, 0x30);

    _delay_us(100);
    
    PCA9555_0_write(REG_OUTPUT_0, 0x30);
    PCA9555_0_write(REG_OUTPUT_0, 0x38);
    PCA9555_0_write(REG_OUTPUT_0, 0x30);
    
    _delay_us(100);
    
    PCA9555_0_write(REG_OUTPUT_0, 0x20);
    PCA9555_0_write(REG_OUTPUT_0, 0x28);
    PCA9555_0_write(REG_OUTPUT_0, 0x20);
    
    _delay_us(100);
    
    lcd_command(0x28);
    lcd_command(0x0C);
    lcd_command(0x01);
    _delay_us(5000);
    
    lcd_command(0x06);
    return;
}

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

void usart_init(unsigned int ubrr)
{
    UCSR0A=0;
    UCSR0B=(1<<RXEN0)|(1<<TXEN0);
    UBRR0H=(unsigned char)(ubrr>>8);
    UBRR0L=(unsigned char)ubrr;
    UCSR0C=(3 << UCSZ00);
    return;
}

void usart_transmit(uint8_t data)
{
    while(!(UCSR0A&(1<<UDRE0)));
    UDR0=data;
}

uint8_t usart_receive()
{
    while(!(UCSR0A&(1<<RXC0)));
    return UDR0;
}

static volatile unsigned char key_array[16] = {'*','0','#','D',
                                               '7','8','9','C',
                                               '4','5','6','B',
                                               '1','2','3','A'};

// returns 0 if no pressed button was detected, or the position of the button
//1 for first, 2 for second, 3 for third, 4 for fourth
uint8_t scan_row(uint8_t row)
{
    unsigned char output;
    output = 0x01 << (row - 1);
    output = (~output);
    output = output & 0x0F;
    PCA9555_0_write(REG_OUTPUT_1, output);
    _delay_us(20);
    uint8_t temp = (0x0F | PCA9555_0_read(REG_INPUT_1));
    temp = ~temp;
    temp = (temp >> 4);
    if ((temp & 0x01) == 0x01) {
        return 1;
    } else if ((temp & 0x02) == 0x02) {
        return 2;
    } else if ((temp & 0x04) == 0x04) {
        return 3;
    } else if ((temp & 0x08) == 0x08) {
        return 4;
    }
    return 0; //if no button was pressed
}

int scan_keypad(void)
{
    uint8_t column;
    uint8_t row = 1;
    while (row < 5)
    {
        column = scan_row(row);
        if(column != 0) break;
        row++;
    }
    if (column != 0) 
    {
        return (row-1) * 4 + (column-1);    // position of the letter in the array
    } else {
        return -1;
    }
}

int scan_keypad_rising_edge(void)
{
    int result1;
    int result2;
    result1 = scan_keypad();
    _delay_ms(15);
    result2 = scan_keypad();
    if (result1 == result2) {
        return result1;
    } else {
        return -1;
    }
}

unsigned char keypad_to_ascii(void)
{
    int pos = scan_keypad_rising_edge();
    if (pos == -1) {
        return 0;
    } else {
        return key_array[pos];
    }
}

/* all messages that have arrived are stored here
we suppose no reply is longer than 100 bytes */  
static char recv_msg_buf[100];

// length of message currently stored in message_buf
static uint8_t recv_msg_lim = 0;

/* reads from the USART until a newline character is found
   and stores to the buffer */
void read_store(void)
{   
    static uint8_t val;
    recv_msg_lim = 0;

    while(1)
    {
        val = usart_receive();

        if (val == '\n')
        {
            break;
        }

        recv_msg_buf[recv_msg_lim] = val;
        recv_msg_lim++;
    }
    return;
}

// send message to the USART
void send_msg(const char* msg, int len)
{
    for(int i = 0; i < len; i++)
    {
        usart_transmit(msg[i]);
    }
    return;
}

//print contents of recv_msg_buf, used only in step 4
void print_recvbuf(void)
{
    lcd_init();
    _delay_ms(2);
    
    lcd_data('4');
    lcd_data('.');
    lcd_data(' ');

    for(int i = 0; i < recv_msg_lim; i++)
    {
        if(i==13)
        {
            lcd_command(0xC0);
        }
        lcd_data(recv_msg_buf[i]);
    }
    return;
}

// print #.Success where # is the relative number
void print_success(uint8_t step)
{
    lcd_init();
    _delay_ms(2);

    lcd_data(step | 0x30);
    lcd_data('.');
    lcd_data(' ');
    lcd_data('S');
    lcd_data('u');
    lcd_data('c');
    lcd_data('c');
    lcd_data('e');
    lcd_data('s');
    lcd_data('s');
    return;
}

// print #.Fail where # is the relative number
void print_fail(uint8_t step)
{
    lcd_init();
    _delay_ms(2);

    lcd_data(step | 0x30);
    lcd_data('.');
    lcd_data(' ');
    lcd_data('F');
    lcd_data('a');
    lcd_data('i');
    lcd_data('l');
    return;
}

// check contents of recv_msg_buf and if it is "Success", print relative message
// if "Fail", print relative message
// step is the number to be printed before the relative message
uint8_t check_print(uint8_t step)
{
    const char* txt_to_compare = "\"Success\"";

    int ret = strncmp(txt_to_compare, recv_msg_buf, recv_msg_lim);  //check if recv_msg_buf contains Success

    if(ret == 0){
        print_success(step);
        return 0;
    } else{
        print_fail(step);
        return 1;
    }
}

uint8_t esp_connect(void)
{
    const char* msg = "ESP:connect\n";
    send_msg(msg, strlen(msg));
    read_store();
    uint8_t ret = check_print(1);
    return ret;
}

uint8_t esp_url(void)
{
    const char* msg = "ESP:url:\"http://192.168.1.250:5000/data\"\n";
    send_msg(msg, strlen(msg));
    read_store();
    uint8_t ret = check_print(2);
    return ret;
}

struct payload_struct {
    float temperature;
    float pressure;
    int team;
    const char* status;
};

struct payload_struct payload;

// reads temp and stores it into the payload struct (adds 16.3 to resemble body temp)
void read_temp(void)
{
    uint8_t lsbyte, msbyte;
    uint16_t val;
    float ret;
    
    if(one_wire_reset() == 0)
    {
        val = 0x8000;
    } else
    {
        one_wire_transmit_byte(0xCC);
    
        one_wire_transmit_byte(0x44);

        while(one_wire_receive_bit() == 0x00)
        {
            // _delay_ms(752);
            _delay_ms(1);
        }   // wait for the measurement to become available

        if(one_wire_reset() == 0)
        {
            val = 0x8000;
        } else
        {
            one_wire_transmit_byte(0xCC);

            one_wire_transmit_byte(0xBE);

            lsbyte = one_wire_receive_byte();

            msbyte = one_wire_receive_byte();
            
            val = msbyte;
            val  = val << 8;
            val += lsbyte;
        }
    }
    if(val == 0x8000)
    {
        payload.temperature = 0.0;
    } else
    {
        if(val >= 0xF800)
        {
            val = ~val;
            val = val + 1;
            ret = val;
            ret = (ret/16)*(-1);
        } else
        {
            ret = val;
            ret = ret / 16;
        }
        payload.temperature = ret + 16.3;
        return;
    }
}       

void adc_init(void)
{
    DDRC &= 0xC0;
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0x00;
    DIDR0 = ~(1 << ADC0D);
    return;
}

void read_pressure(void)
{
    float ADC_val;
    ADCSRA |= (1 << ADSC);
    while ((ADCSRA & (1 << ADSC)) == (1 << ADSC)) {
        _delay_us(10);
    }
    ADC_val = ADC;
    ADC_val = (ADC_val * 20) / 1024;
    payload.pressure = ADC_val;
}

void change_status(void)
{
    if(keypad_to_ascii() == '1')
    {
        payload.status = "NURSECALL";
        return;
    }
    if(payload.pressure > 12 || payload.pressure < 4)
    {
        payload.status = "CHECKPRESSURE";
        return;
    }
    if(payload.temperature > 37 || payload.temperature < 34)
    {
        payload.status = "CHECKTEMPERATURE";
        return;
    }
    if(keypad_to_ascii() == '#')
    {
        payload.status = "OK";
        return;
    }
    if(strncmp(payload.status,"NURSECALL",10) != 0)
    {
        payload.status = "OK";
        return;
    }
    return;
}

// function that prints to lcd the data that the exercise 8.2 requires
void print_to_lcd(void)
{   
    lcd_init();
    _delay_ms(2);

    char tempbuff[20];

    float temperature = payload.temperature;
    int integer = (int)temperature;
    int decimals = (int)((temperature - integer) * 10);
        
    int pos = snprintf(tempbuff, 20, "%d.%01d", integer, decimals<0 ? -decimals : decimals);
    tempbuff[pos++] = 223;
    pos += snprintf(tempbuff + pos, 20 - pos, "C ");
    
    float pressure = payload.pressure;
    integer = (int)pressure;
    decimals = (int)((pressure - integer) * 10);
        
    pos += snprintf(tempbuff + pos, 20 - pos, "%d.%01dcmH2O", integer, decimals<0 ? -decimals : decimals);

    for(int i = 0; i < pos; i++)
    {
        lcd_data(tempbuff[i]);
    }

    lcd_command(0xC0);

    for(int i = 0; i < strlen(payload.status); i++)
    {
        lcd_data(payload.status[i]);
    }
    return;
}

char payload_json[200];
int json_pos = 0;
void parse_payload_json(void)
{
    json_pos = 0;
    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "ESP:");
    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "payload:[");

    // add temperature

    float temperature = payload.temperature;
    int integer = (int)temperature;
    int decimals = (int)((temperature - integer) * 10);

    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "{\"name\":\"temperature\",\"value\":\"%d.%01d\"}", integer, decimals<0 ? -decimals : decimals);
    
    payload_json[json_pos++] = ',';

    // add pressure

    float pressure = payload.pressure;
    integer = (int)pressure;
    decimals = (int)((pressure - integer) * 10);
        
    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "{\"name\":\"pressure\",\"value\":\"%d.%01d\"}", integer, decimals<0 ? -decimals : decimals);
    
    payload_json[json_pos++] = ',';

    // add team number

    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "{\"name\":\"team\",\"value\":\"%d\"}", payload.team);

    payload_json[json_pos++] = ',';

    // add status

    json_pos += snprintf(payload_json + json_pos, 200 - json_pos, "{\"name\":\"status\",\"value\":\"%s\"}", payload.status);

    payload_json[json_pos++] = ']';
    payload_json[json_pos++] = '\n';
    return;
}

uint8_t esp_payload(void)
{
    parse_payload_json();    
    send_msg(payload_json, json_pos);
    read_store();
    uint8_t ret = check_print(3);
    return ret;
}

void esp_transmit(void)
{
    const char* msg = "ESP:transmit\n";
    send_msg(msg, strlen(msg));    
    read_store();
    print_recvbuf();
    return;
}

int main(void)
{
    twi_init();
    usart_init(103);                                //for communication with the esp/arduino
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);     //for lcd control
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    adc_init();
    payload.team = 1;
    
    while(1)
    {
        int count = 0;
        while(1)
        {
            if(esp_connect() == 0)
            {
                break;
            }
            else
            {
                count ++;
            }
            if (count == 3)
            {
                break;
            }
        }
        
        _delay_ms(1500);

        count = 0;
        while(1)
        {
            if(esp_url() == 0)
            {
                break;
            }
            else
            {
                count ++;
            }
            if (count == 3)
            {
                break;
            }
        }

        _delay_ms(1500);
        
        read_temp();
        read_pressure();
        change_status();
        print_to_lcd();
        
        _delay_ms(1500);
        
        count = 0;
        while(1)
        {
            if(esp_payload() == 0)
            {
                break;
            }
            else
            {
                count ++;
            }
            if (count == 3)
            {
                break;
            }
        }
        
        _delay_ms(1500);
        
        esp_transmit();
        
        _delay_ms(1500);
    }    
    return 0;
}