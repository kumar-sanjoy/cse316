#include "i2c.h"
#include <avr/io.h>
#include <util/delay.h>

#define SCL_PIN PC0
#define SDA_PIN PC1

void i2c_init(void)
{
    // 1. Bus Recovery: Release stuck SDA pin if power was cycled mid-transfer
    DDRC &= ~((1 << SCL_PIN) | (1 << SDA_PIN)); // Configure as inputs
    PORTC |= (1 << SCL_PIN) | (1 << SDA_PIN);   // Enable internal pull-ups
    _delay_us(10);

    // Toggle SCL 9 full high-to-low clock cycles to clear stuck slaves
    DDRC |= (1 << SCL_PIN); // SCL output
    for (uint8_t i = 0; i < 9; i++) {
        PORTC &= ~(1 << SCL_PIN); // SCL Low
        _delay_us(10);
        PORTC |= (1 << SCL_PIN);  // SCL High
        _delay_us(10);
    }

    // 2. Hardware TWI Setup
    TWSR = 0x00;
    TWBR = 10; // ~111kHz SCL frequency at 4MHz F_CPU
    TWCR = (1 << TWEN);
}

static uint8_t i2c_wait_twint(void)
{
    uint16_t timeout = 10000;
    while (!(TWCR & (1 << TWINT))) {
        if (--timeout == 0) return 0; // Prevent infinite hanging
    }
    return 1;
}

uint8_t i2c_start(uint8_t address)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    if (!i2c_wait_twint()) return 0;

    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!i2c_wait_twint()) return 0;

    return 1;
}

uint8_t i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    return i2c_wait_twint();
}

void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    _delay_us(10);
}
