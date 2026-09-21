#ifndef F_CPU
#define F_CPU 1000000UL // ATmega32 default fuses: internal RC osc, no divider
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "uart.h"

// Identical firmware runs on BOTH boards. Each side has its own 3 buttons
// and its own HC-05 paired directly to the other board's HC-05 (bound as
// master/slave via AT commands once, so they auto-connect on power-up with
// no phone involved). Pressing a button here transmits its digit to the
// other board, which mirrors it exactly like one of its own button presses.

#define BAUD 9600
// Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// Labels are printed once at startup; after that only the single digit
// position is rewritten, so a redraw is as fast as possible and can't
// stall the main loop long enough to lose an incoming UART byte.
#define STATE_DIGIT_COL 7
#define REMOTE_DIGIT_COL 8

static void show_state(uint8_t value)
{
    lcd_set_cursor(0, STATE_DIGIT_COL);
    char buf[2] = { (char)('0' + value), '\0' };
    lcd_print(buf);
}

static void show_remote(uint8_t value)
{
    lcd_set_cursor(1, REMOTE_DIGIT_COL);
    // 0 means "nothing received yet" -- show a dash instead of a digit
    char buf[2] = { value ? (char)('0' + value) : '-', '\0' };
    lcd_print(buf);
}

int main(void)
{
    // PB0-PB2 as inputs with pull-ups: buttons pull the pin low when pressed
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    uart_init(MYUBRR);
    lcd_init();

    uint8_t remote_value = 0; // sticky value last received from the other board
    uint8_t last_local = 0;   // last local button value transmitted (edge detect)
    uint8_t last_shown = 0xFF;      // force first draw
    uint8_t last_remote_shown = 0xFF; // force first draw

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("State: ");
    lcd_set_cursor(1, 0);
    lcd_print("Remote: ");
    show_state(0);
    show_remote(0);
    last_shown = 0;
    last_remote_shown = 0;

    while (1)
    {
        // Drain every buffered byte this pass so nothing is left queued up
        // to appear stale on the next update.
        while (uart_available())
        {
            char c = uart_receive();
            if (c == '1' || c == '2' || c == '3')
                remote_value = c - '0';
        }

        uint8_t local_value;
        if (!(PINB & (1 << PB0)))
            local_value = 1;
        else if (!(PINB & (1 << PB1)))
            local_value = 2;
        else if (!(PINB & (1 << PB2)))
            local_value = 3;
        else
            local_value = 0;

        // Tell the other board whenever our own button selection changes,
        // so it can mirror it just like one of its own button presses.
        // Only sent on change (not while 0/released) so the other board's
        // sticky remote_value is never forced back to a "no press" state.
        if (local_value != 0 && local_value != last_local)
            uart_transmit((char)('0' + local_value));
        last_local = local_value;

        // Buttons take priority while held; on release, fall back to
        // whatever value the other board last sent (never back to 0).
        uint8_t value = local_value ? local_value : remote_value;

        if (value != last_shown)
        {
            show_state(value);
            last_shown = value;
        }

        if (remote_value != last_remote_shown)
        {
            show_remote(remote_value);
            last_remote_shown = remote_value;
        }

        _delay_ms(50); // simple button debounce
    }
}
