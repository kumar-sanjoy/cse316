#include <avr/io.h>
#include <avr/interrupt.h>

volatile int overflowCount;

ISR(TIMER1_OVF_vect)
{
    overflowCount++;
    if(overflowCount==31)
    {
        PORTB = ~PORTB;
        overflowCount = 0;
    }
}

int main(void)
{
    overflowCount = 0;
    DDRB = 0xFF;
    PORTB = 0xFF;

    //configure timer
    TCCR1A = 0b00000000; // normal mode
    TCCR1B = 0b00000001; // no prescaler, internal clock

    TIMSK = 0b00000100; //Enable Overflow Interrupt
    sei(); //Global Interrupt Enable

    while(1);
}