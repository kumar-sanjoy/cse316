#ifndef F_CPU
// ATmega32 ships with the CKDIV8 fuse programmed, so the internal 1MHz
// RC oscillator is divided by 8, giving an actual clock of 125kHz.
// This must match reality for the UART baud rate calc to be correct.
#define F_CPU 125000UL
#endif

#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include "i2c.h"
#include "lcd.h"

#define BAUD 9600
// Calculation for Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// Longest byte-code text is "255" (3 chars) + null terminator
#define NUM_BUF_SIZE 5

// Some BT controller button widgets send their code twice per tap
// (press + release). After showing a key, briefly ignore/discard
// further bytes so the second send from the same tap isn't treated
// as a new keypress.
#define KEY_DEBOUNCE_MS 150

#define LED_PIN PB3

//--------------------------------------------------
// UART
//--------------------------------------------------

void uart_init(unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    UCSRA |= (1 << U2X);
    UCSRB = (1 << TXEN) | (1 << RXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

    // Flush internal hardware RX buffer
    unsigned char dummy;
    while (UCSRA & (1 << RXC)) {
        dummy = UDR;
    }
}

void uart_transmit(char data)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

void uart_print(const char *str)
{
    while (*str)
        uart_transmit(*str++);
}

uint8_t uart_available(void)
{
    return (UCSRA & (1 << RXC));
}

char uart_receive(void)
{
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

//--------------------------------------------------
// Activity LED (PB3)
//--------------------------------------------------

void led_toggle(void)
{
    PORTB ^= (1 << LED_PIN);
}

//--------------------------------------------------
// LCD key display
//--------------------------------------------------

void show_key(char key)
{
    char label[2] = {key, '\0'};

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Key:");
    lcd_set_cursor(1, 0);
    lcd_print(label);

    uart_print("Key: ");
    uart_transmit(key);
    uart_print("\r\n");
}

// Shown for any byte we don't have a mapping for, so the exact code
// it sends can be read off the LCD and wired up next.
void show_unknown(char c)
{
    char code_buf[NUM_BUF_SIZE];
    itoa((uint8_t)c, code_buf, 10);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Unknown code:");
    lcd_set_cursor(1, 0);
    lcd_print(code_buf);

    if (c >= 32 && c <= 126)
    {
        char ch_buf[4] = {' ', '\'', c, '\0'};
        lcd_print(ch_buf);
        lcd_print("'");
    }

    uart_print("Unknown byte code=");
    uart_print(code_buf);
    uart_print("\r\n");
}

void key_debounce_drain(void)
{
    _delay_ms(KEY_DEBOUNCE_MS);
    while (uart_available())
    {
        uart_receive();
    }
}

//--------------------------------------------------
// Main
//--------------------------------------------------

int main(void)
{
    uart_init(MYUBRR);
    lcd_init();

    DDRB |= (1 << LED_PIN);
    PORTB &= ~(1 << LED_PIN);

    lcd_set_cursor(0, 0);
    lcd_print("BT Ready");
    uart_print("System Ready\r\n");

    while (1)
    {
        if (uart_available())
        {
            char c = uart_receive();
            led_toggle();

            switch (c)
            {
                // 4x4 keypad layout:
                //  1 2 3 A
                //  4 5 6 B
                //  7 8 9 C
                //  * 0 # D
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case 'A': case 'B': case 'C': case 'D':
                case '*': case '#':
                    show_key(c);
                    key_debounce_drain();
                    break;

                default:
                    show_unknown(c);
                    break;
            }
        }
    }
}
