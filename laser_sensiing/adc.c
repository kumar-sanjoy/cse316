#include "adc.h"
#include <avr/io.h>

// TEPT4400 sensor node (10k pull-up to +5V, phototransistor to GND)
// is on PA0 (physical pin 40) = ADC0

void adc_init(void)
{
    ADMUX = (1 << REFS0); // AVCC as reference, right-adjusted result, channel 0
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS0); // Enable ADC, /32 prescaler -> 125kHz @ 4MHz F_CPU
}

uint16_t adc_read(uint8_t channel)
{
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07); // Keep reference bits, select channel
    ADCSRA |= (1 << ADSC);                     // Start conversion
    while (ADCSRA & (1 << ADSC))               // Wait for conversion to finish
        ;
    return ADC;
}

// Averages multiple samples to smooth out mains-flicker / electrical noise
uint16_t adc_read_avg(uint8_t channel, uint8_t samples)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++)
        sum += adc_read(channel);
    return (uint16_t)(sum / samples);
}
