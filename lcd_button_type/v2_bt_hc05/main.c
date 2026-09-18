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

// Labels are printed once at startup; after that only the single digit
// position is rewritten, so a redraw is as fast as possible and can't
// stall the main loop long enough to lose an incoming UART byte.
#define STATE_DIGIT_COL 7
#define SIGNAL_DIGIT_COL 8

static void show_state(uint8_t value)
{
    lcd_set_cursor(0, STATE_DIGIT_COL);
    char buf[2] = { (char)('0' + value), '\0' };
    lcd_print(buf);
}

static void show_signal(char c)
{
    lcd_set_cursor(1, SIGNAL_DIGIT_COL);
    // Show non-printable bytes (e.g. \r, \n) as a space instead of garbage
    char buf[2] = { (c >= 32 && c < 127) ? c : ' ', '\0' };
    lcd_print(buf);
}

int main(void)
{
    // PB0-PB2 as inputs with pull-ups: buttons pull the pin low when pressed
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    uart_init(MYUBRR);
    lcd_init();

    uint8_t bt_value = 0;      // sticky value last received from phone, holds until changed
    uint8_t last_shown = 0xFF; // force first draw
    char last_signal = 0;      // force first draw

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("State: ");
    lcd_set_cursor(1, 0);
    lcd_print("Signal: ");
    show_state(0);
    show_signal(' ');
    last_shown = 0;

    while (1)
    {
        // Phone (HC-05) input: only digits 1-3 update the sticky value.
        // Drain every buffered byte this pass (a phone app often sends a
        // trailing \r/\n after each digit) so nothing is left queued up
        // to appear stale on the next update, and skip non-printable
        // terminator bytes on the signal row instead of showing them as
        // a blank that looks like the signal was "cleared".
        while (uart_available())
        {
            char c = uart_receive();
            if (c == '1' || c == '2' || c == '3')
                bt_value = c - '0';

            if (c >= 32 && c < 127 && c != last_signal)
            {
                show_signal(c);
                last_signal = c;
            }
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
            show_state(value);
            last_shown = value;
        }

        _delay_ms(50); // simple button debounce
    }
}
