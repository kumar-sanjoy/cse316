#ifndef F_CPU
#define F_CPU 1000000UL // ATmega32 default fuses: internal RC osc, no divider
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
    // PB0-PB2 as inputs with internal pull-ups: buttons must connect to GND when pressed
    // (ATmega32 has no internal pull-down, so this is the only wiring that works without extra parts)
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    lcd_init();

    uint8_t last_shown = 0xFF; // force first draw

    while (1)
    {
        uint8_t value;

        if (!(PINB & (1 << PB0)))
            value = 1;
        else if (!(PINB & (1 << PB1)))
            value = 2;
        else if (!(PINB & (1 << PB2)))
            value = 3;
        else
            value = 0;

        if (value != last_shown)
        {
            lcd_clear();
            lcd_set_cursor(0, 0);
            char c = '0' + value;
            char buf[2] = { c, '\0' };
            lcd_print(buf);
            last_shown = value;
        }

        _delay_ms(50); // simple debounce
    }
}
