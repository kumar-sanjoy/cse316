#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main (void) {

    DDRB |= 0xff;
    uint8_t leds = 1;

    while(1) {
        leds = (leds << 1) | (leds >> 7);
        PORTB = leds; 
        _delay_ms(200);
    }

}