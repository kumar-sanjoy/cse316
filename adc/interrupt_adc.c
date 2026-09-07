#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t adc_result;

ISR (ADC_vect) {
    adc_result = ADCH;
    PORTB = adc_result;
}

int main(void) {

    DDRB = 0xff;
    PORTB = 0x00;

    ADMUX = (
        1 << REFS0 |
        1 << ADLAR 
    );

    ADCSRA = (
        1 << ADEN |
        1 << ADIE
    );

    sei();

    while (1) {
        ADCSRA |= ( 1 << ADSC );
        
    }

}