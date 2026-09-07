#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    DDRC = 0x00;
    DDRB = 0xff;
    DDRD = 0xff;

    while(1) {
        if (PINC < 100) {
            PORTB = PINC;
        } else {
            PORTD = PINC;
        }

        _delay_ms(500);
    }
}