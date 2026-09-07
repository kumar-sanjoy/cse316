#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {

    MCUCSR |= (1 << JTD);
    MCUCSR |= (1 << JTD);

    DDRA = 0x00;
    DDRB = 0xff;
    DDRC = 0xff;

    uint8_t value;

    while(1) {
        value = PINA; // read pina only once
        PORTB = (value >> 4) & 0x0f;
        PORTC = (value << 4) & 0xf0;

        _delay_ms(500);

    }


}