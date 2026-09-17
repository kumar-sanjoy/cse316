#ifndef F_CPU
#define F_CPU 1000000UL  // ATmega32 default fuses: internal RC osc, no divider
#endif

#include <avr/io.h>
#include <util/delay.h>

// Minimal alive-check: if this blinks, the chip is fetching, decoding,
// and executing instructions correctly, and its I/O pins work.
int main(void)
{
    DDRB = (1 << PB0);   // Only PB0 as output, everything else left alone

    while (1)
    {
        PORTB ^= (1 << PB0);
        _delay_ms(300);
    }
}
