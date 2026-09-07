#include <avr/io.h>

int main(void) {
    uint8_t adc_result;
    DDRB = 0xff;
    PORTB = 0x00;

    ADMUX = (   
        1 << REFS0 |
        1 << ADLAR 
    ); // rest is auto 0


    ADCSRA = (
        1 << ADPS1 |
        1 << ADEN
    );

    while(1) {

        ADCSRA |= (1 << ADSC);

        while(ADCSRA & (1 << ADSC)) {
            ;
        } // wait for conversion finish

        adc_result = ADCH;
        PORTB = adc_result;

    }
    

}