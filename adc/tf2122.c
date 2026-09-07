



// #include <avr/io.h>
// #include <avr/interrupt.h>

// uint8_t count;

// ISR(INT0_vect) {
//     count += 1;
// }

// ISR(INT1_vect) {
//     count -= 1;
// }

// int main (void) {

//     DDRA = 0xff;
//     count = 0;

//     GICR = (1 << INT0) | (1 << INT1);

//     MCUCR = MCUCR & 0b11111111;

//     while(1) {
//         PORTA = count;
//     }
    
// }



#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint8_t count; 

ISR(INT2_vect) {
    count += 1;
}

ISR(INT0_vect) {
    count -= 1;
}

int main (void) {

    DDRA = 0xff;
    DDRB &= ~(1 << PB2);
    DDRD &= ~(1 << PD2);
    count = 0;

    GICR = (1 << INT0) | (1 << INT2);

    MCUCR |= (
        1 << ISC01 |
        1 << ISC00
    );

    MCUCSR |= (
        1 << ISC2
    );

    sei();

    while(1) {
        PORTA = count;
    }
    
}