#include "uart.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// Small ring buffer filled by the RX-complete ISR so a byte is captured
// the instant it arrives, even while the main loop is busy (e.g. writing
// to the LCD) instead of polling. Without this, a second byte arriving
// while UDR still holds an unread one overwrites it and is silently lost.
#define RX_BUF_SIZE 8
static volatile char rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_head = 0; // next write index
static volatile uint8_t rx_tail = 0; // next read index

ISR(USART_RXC_vect)
{
    unsigned char c = UDR;
    uint8_t next = (rx_head + 1) % RX_BUF_SIZE;
    if (next != rx_tail) { // drop byte if buffer is full
        rx_buf[rx_head] = c;
        rx_head = next;
    }
}

void uart_init(unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    UCSRA |= (1 << U2X);
    UCSRB = (1 << TXEN) | (1 << RXEN) | (1 << RXCIE);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

    // Flush internal hardware RX buffer
    unsigned char dummy;
    while (UCSRA & (1 << RXC)) {
        dummy = UDR;
    }
    (void)dummy;

    sei(); // enable global interrupts so USART_RXC_vect fires
}

uint8_t uart_available(void)
{
    return rx_head != rx_tail;
}

char uart_receive(void)
{
    while (rx_head == rx_tail); // wait for a byte if called when empty
    char c = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUF_SIZE;
    return c;
}

void uart_transmit(char data)
{
    while (!(UCSRA & (1 << UDRE))); // wait for empty transmit buffer
    UDR = data;
}
