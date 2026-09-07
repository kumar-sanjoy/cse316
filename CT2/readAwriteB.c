#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main (void) {
    DDRA = 0x00;
    DDRB = 0xff;

    while(1) {
        PORTB = PINA;
        _delay_ms(500);
    }
}