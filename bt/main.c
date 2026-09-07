#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#define BAUD 9600
// Calculation for Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

//--------------------------------------------------
// UART Initialization
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
// Main
//--------------------------------------------------

int main(void)
{
    uart_init(MYUBRR);

    // Set PB0 and PB1 as outputs
    DDRB |= (1 << PB0) | (1 << PB1);

    // Turn LEDs off initially
    PORTB &= ~((1 << PB0) | (1 << PB1));

    uart_print("System Ready\r\n");

    while (1)
    {
        // Check for incoming commands without any delay
        if (uart_available())
        {
            char cmd = uart_receive();

            switch (cmd)
            {
                case '0':
                    PORTB |= (1 << PB0);   // Keep PB0 ON
                    PORTB &= ~(1 << PB1);  // Keep PB1 OFF
                    uart_print("PB0 ON\r\n");
                    break;

                case '1':
                    PORTB |= (1 << PB1);   // Keep PB1 ON
                    PORTB &= ~(1 << PB0);  // Keep PB0 OFF
                    uart_print("PB1 ON\r\n");
                    break;

                // Silently ignore carriage returns, newlines, and spaces sent by mobile app
                case '\r':
                case '\n':
                case ' ':
                    break;

                default:
                    // uart_print("Unknown Command\r\n");
                    break;
            }
        }
        // No delay here—state persists in PORTB until a new command is received
    }
}