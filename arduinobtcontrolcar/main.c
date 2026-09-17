#ifndef F_CPU
#define F_CPU 1000000UL
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

#define LED_PIN PB0

// The app's light button sends its code twice per tap (press + release).
// After toggling, briefly ignore/discard further bytes so the second
// send from the same tap doesn't flip the LED right back.
#define LED_DEBOUNCE_MS 250

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
// LED (PB0)
//--------------------------------------------------

void led_set(uint8_t on)
{
    if (on)
        PORTB |= (1 << LED_PIN);
    else
        PORTB &= ~(1 << LED_PIN);
}

void led_debounce_drain(void)
{
    _delay_ms(LED_DEBOUNCE_MS);
    while (uart_available())
    {
        uart_receive();
    }
}

//--------------------------------------------------
// LCD button display
//--------------------------------------------------

void show_button(const char *label)
{
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Button:");
    lcd_set_cursor(1, 0);
    lcd_print(label);
}

// Shown for any byte we don't yet have a mapping for (e.g. Horn/Light
// buttons), so the exact code they send can be read off the LCD and
// wired up next.
void show_unknown(char c)
{
    char code_buf[5];
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

//--------------------------------------------------
// Main
//--------------------------------------------------

int main(void)
{
    uart_init(MYUBRR);
    lcd_init();

    DDRB |= (1 << LED_PIN);
    led_set(0);
    uint8_t led_on = 0;

    lcd_set_cursor(0, 0);
    lcd_print("BT Ready");
    uart_print("System Ready\r\n");

    while (1)
    {
        if (uart_available())
        {
            char c = uart_receive();

            switch (c)
            {
                case 'F': show_button("Up");    break;
                case 'B': show_button("Down");  break;
                case 'L': show_button("Left");  break;
                case 'R': show_button("Right"); break;
                case 'S': show_button("Stop");  break;

                case 'X':
                    led_on = !led_on;
                    led_set(led_on);
                    show_button(led_on ? "LED ON" : "LED OFF");
                    led_debounce_drain();
                    break;

                case 'Y':
                    show_button("Horn");
                    break;

                default:
                    show_unknown(c);
                    break;
            }
        }
    }
}
