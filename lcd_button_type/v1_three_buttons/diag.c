// I2C ack diagnostic — does NOT need the LCD to render anything correctly.
// It only checks whether the bus gets an ACK at each candidate address,
// and reports the result by blinking the LCD backpack's backlight bit
// (P3 on the PCF8574), which requires only a successful i2c_start()+
// i2c_write(), not a working init sequence.
//
// Watch the LCD BACKLIGHT (not the character grid):
//   fast blink (~150ms)  -> address 0x27 ACKed
//   slow blink (~600ms)  -> address 0x3F ACKed
//   steady, never blinks -> NEITHER address ACKed (wiring/pullup problem)

#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"

#define LCD_BL 0x08

static void backlight_write(uint8_t addr7, uint8_t on)
{
    if (i2c_start(addr7 << 1))
    {
        i2c_write(on ? LCD_BL : 0x00);
        i2c_stop();
    }
}

int main(void)
{
    i2c_init();

    while (1)
    {
        if (i2c_start(0x27 << 1))
        {
            i2c_stop();
            backlight_write(0x27, 1);
            _delay_ms(150);
            backlight_write(0x27, 0);
            _delay_ms(150);
        }
        else if (i2c_start(0x3F << 1))
        {
            i2c_stop();
            backlight_write(0x3F, 1);
            _delay_ms(600);
            backlight_write(0x3F, 0);
            _delay_ms(600);
        }
        // else: neither ACKed this pass, loop and keep retrying
    }
}
