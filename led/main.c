#ifndef F_CPU
#define F_CPU 1000000UL  // ATmega32 default fuses: internal RC osc, no divider
#endif

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRB = 0xFF;

    while (1)
    {
        PORTB ^= (1 << PB0);
        _delay_ms(500);
    }
}