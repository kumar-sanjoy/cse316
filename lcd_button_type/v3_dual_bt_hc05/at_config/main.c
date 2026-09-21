#ifndef F_CPU
// Defaults to the factory 1MHz internal RC oscillator (weak/default AT
// mode: power up the HC-05 normally, no button held, at its already
// configured 9600 baud). Build with -DF_CPU=8000000UL (after
// `make fuse-8mhz`) instead for full AT mode (button held at power-up,
// forced 38400 baud) -- see makefile.
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "uart.h"

// Full AT mode's forced 38400 baud has a ~8.5% UBRR rounding error at
// 1MHz F_CPU (too high for reliable UART), so that mode needs the 8MHz
// build. At 1MHz, use weak/default AT mode instead, which talks at
// whatever baud the module is already configured for (9600 here).
#if F_CPU == 8000000UL
#define BAUD 38400
#else
#define BAUD 9600
#endif
// Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// One-time HC-05 configuration tool. No PC/terminal available, so the
// ATmega itself "types" a fixed list of AT commands and the LCD acts as
// the terminal screen, showing what was sent and what came back.
//
// Before flashing this: run `make fuse-8mhz` once (switches this chip's
// clock to the internal 8MHz RC oscillator so 38400 baud is accurate).
// Power the HC-05 up in full AT mode (hold its button while applying
// power; release once its LED blinks slowly, ~once every 2 seconds).
//
// IMPORTANT: after AT config is done, run `make fuse-1mhz` to put the
// chip's clock back to the factory-default 1MHz BEFORE reflashing the
// real v3_dual_bt_hc05 firmware, which assumes F_CPU = 1000000UL.
//
// Usage: press PB0 to send the next command in the list and show its
// reply. Press PB1 to resend the current command (e.g. if it timed out
// or you want to re-read the reply). PB2 restarts from the first command.
//
// Edit COMMANDS[] below before flashing each board:
//   - Master board: AT+ROLE=1, AT+CMODE=0, AT+BIND=<slave address>
//   - Slave board:  AT+ROLE=0
//   - Both: AT+UART=9600,0,0 to lock in the baud rate used by v3 firmware
// Get a board's own address with AT+ADDR? (read it off the OTHER board's
// reply before wiring the BIND command on this one); AT+BIND wants the
// address with commas instead of colons, e.g. 98D3,71,FD3A21.
//
// Diagnostic mode: define LOOPBACK_TEST (below, or with -DLOOPBACK_TEST)
// to skip the HC-05 entirely and test only the ATmega's own UART TX/RX.
// Wire the ATmega's TXD pin directly to its own RXD pin (HC-05
// disconnected) and it should print "AT" back on the LCD every second --
// proving the transmit path works and isolating any remaining "no reply"
// down to the HC-05/AT-mode side rather than this firmware.
// #define LOOPBACK_TEST

static const char *COMMANDS[] = {
    "AT",
    "AT+ROLE?",
    "AT+ADDR?",
    "AT+UART?",
    // -- Edit/add the commands you actually need below --
    // "AT+ROLE=1",
    // "AT+CMODE=0",
    // "AT+BIND=98D3,71,FD3A21",
    // "AT+UART=9600,0,0",
};
#define NUM_CMDS (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

#define REPLY_BUF_SIZE 32
static char reply[REPLY_BUF_SIZE];

static void send_command(const char *cmd)
{
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print(cmd);

    for (const char *p = cmd; *p; p++)
        uart_transmit(*p);
    uart_transmit('\r');
    uart_transmit('\n');

    _delay_ms(500); // give the HC-05 time to reply

    uint8_t n = 0;
    while (uart_available() && n < REPLY_BUF_SIZE - 1)
    {
        char c = uart_receive();
        // Collapse \r\n into spaces so the 16-char LCD row stays readable
        reply[n++] = (c >= 32 && c < 127) ? c : ' ';
    }
    reply[n] = '\0';

    lcd_set_cursor(1, 0);
    lcd_print(reply[0] ? reply : "(no reply)");
}

#ifdef LOOPBACK_TEST
static void run_loopback_test(void)
{
    while (1)
    {
        uart_transmit('A');
        uart_transmit('T');
        _delay_ms(500); // give the HC-05 time to reply (matches send_command)

        lcd_clear();
        lcd_set_cursor(0, 0);
        if (uart_available())
        {
            char c1 = uart_receive();
            char c2 = uart_available() ? uart_receive() : '?';
            char buf[3] = { c1, c2, '\0' };
            lcd_print(buf);
        }
        else
        {
            lcd_print("loopback fail");
        }

        _delay_ms(1000);
    }
}
#endif

int main(void)
{
    // PB0-PB2 as inputs with pull-ups: buttons pull the pin low when pressed
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    uart_init(MYUBRR);
    lcd_init();

#ifdef LOOPBACK_TEST
    run_loopback_test(); // never returns
#endif

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("PB0=next PB1=");
    lcd_set_cursor(1, 0);
    lcd_print("resend PB2=first");
    _delay_ms(1500);

    uint8_t idx = 0;
    uint8_t last_pb0 = 1, last_pb1 = 1, last_pb2 = 1;

    send_command(COMMANDS[idx]);

    while (1)
    {
        uint8_t pb0 = (PINB & (1 << PB0)) ? 1 : 0;
        uint8_t pb1 = (PINB & (1 << PB1)) ? 1 : 0;
        uint8_t pb2 = (PINB & (1 << PB2)) ? 1 : 0;

        if (last_pb0 == 1 && pb0 == 0) // PB0 just pressed: next command
        {
            idx = (idx + 1) % NUM_CMDS;
            send_command(COMMANDS[idx]);
        }
        else if (last_pb1 == 1 && pb1 == 0) // PB1 just pressed: resend
        {
            send_command(COMMANDS[idx]);
        }
        else if (last_pb2 == 1 && pb2 == 0) // PB2 just pressed: restart list
        {
            idx = 0;
            send_command(COMMANDS[idx]);
        }

        last_pb0 = pb0;
        last_pb1 = pb1;
        last_pb2 = pb2;

        _delay_ms(50); // simple button debounce
    }
}
