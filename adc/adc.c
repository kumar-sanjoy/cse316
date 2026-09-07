#include <avr/io.h>

int main(void)
{
    uint16_t adc_value;
    uint8_t low, high;

    // AVCC as reference, ADC0 selected
    ADMUX = (1 << REFS0);

    // Enable ADC, Prescaler = 128 (recommended for 16 MHz)
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);

    while (1)
    {
        // Start conversion
        ADCSRA |= (1 << ADSC);

        // Wait until conversion completes
        while (ADCSRA & (1 << ADSC));

        // Read in correct order
        low  = ADCL;
        high = ADCH;

        // Combine into 10-bit value
        adc_value = ((uint16_t)high << 8) | low;

        // adc_value now contains 0–1023
    }
}