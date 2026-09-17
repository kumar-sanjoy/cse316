#ifndef F_CPU
#define F_CPU 1000000UL // ATmega32 default fuses: internal RC osc, no divider
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "uart.h"

#define BAUD 9600
// Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

static void show_value(uint8_t value)
{
    lcd_clear();
    lcd_set_cursor(0, 0);
    char c = '0' + value;
    char buf[2] = { c, '\0' };
    lcd_print(buf);
}

int main(void)
{
    // PB0-PB2 as inputs with pull-ups: buttons pull the pin low when pressed
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    uart_init(MYUBRR);
    lcd_init();

    uint8_t bt_value = 0;   // sticky value last received from phone, holds until changed
    uint8_t last_shown = 0xFF; // force first draw

    show_value(0);
    last_shown = 0;

    while (1)
    {
        // Phone (HC-05) input: only digits 1-3 update the sticky value,
        // anything else from the phone is ignored.
        if (uart_available())
        {
            char c = uart_receive();
            if (c == '1' || c == '2' || c == '3')
                bt_value = c - '0';
        }

        // Buttons take priority while held; on release, fall back to
        // whatever value the phone last sent (never back to 0 on its own).
        uint8_t value;
        if (!(PINB & (1 << PB0)))
            value = 1;
        else if (!(PINB & (1 << PB1)))
            value = 2;
        else if (!(PINB & (1 << PB2)))
            value = 3;
        else
            value = bt_value;

        if (value != last_shown)
        {
            show_value(value);
            last_shown = value;
        }

        _delay_ms(50); // simple button debounce
    }
}
