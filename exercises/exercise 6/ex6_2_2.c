#define F_CPU 16000000UL

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

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

static volatile unsigned char key_array[16] = {'*','0','#','D',
                                               '7','8','9','C',
                                               '4','5','6','B',
                                               '1','2','3','A'};

static volatile unsigned char output;

// returns 0 if no pressed button was detected, or the position of the button
//1 for first, 2 for second, 3 for third, 4 for fourth
uint8_t scan_row(uint8_t row)
{
    output = 0x01 << (row - 1);
    output = (~output) & 0x0F;
    PCA9555_0_write(REG_OUTPUT_1, output);
    //maybe delay
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

void blinky(void) {
    for(int i = 0; i < 10; i++) {
        PORTB = 0xFF;
        _delay_ms(250);
        PORTB = 0x00;
        _delay_ms(250);
    }
}

unsigned char submitted_code[2];

int main(void)
{
    unsigned char c;
    unsigned char code[2] = {'0','1'};
    DDRB = 0xFF;
    PORTB = 0x00;
    twi_init();
    PCA9555_0_write(REG_CONFIGURATION_0, 0x00);
    PCA9555_0_write(REG_CONFIGURATION_1, 0xF0);
    
    // read first button
    
    while(1) {
        c = keypad_to_ascii();
        if (c == 0) {
            continue;
        } else {
            while (!(keypad_to_ascii()==0)); // wait for the button to be unpressed
            submitted_code[0] = c;           // store first character
        }
        
        while(1) {                       // same procedure for the second character
            c = keypad_to_ascii();
            if (c == 0) {
                continue;
            } else {
                while (!(keypad_to_ascii()==0)); // wait for the button to be unpressed
                submitted_code[1] = c;           // store second character
                if ((submitted_code[0] == code[0]) && (submitted_code[1] == code[1])) {
                    PORTB = 0xFF;
                    _delay_ms(4000);
                    PORTB = 0x00;
                    _delay_ms(1000);
                    break;
                } else {
                    blinky();
                    break;
                }
            }
        }
        
    }
}