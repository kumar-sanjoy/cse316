#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#define BAUD 9600
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// ==================================================
// Motor Left (Driver 1: Front-Left & Rear-Left)
// ==================================================
#define AIN1 PC0
#define AIN2 PC1

// ==================================================
// Motor Right (Driver 2: Front-Right & Rear-Right)
// ==================================================
#define BIN1 PC2
#define BIN2 PC3

// TB6612 Standby
#define STBY PC4

// ==================================================
// Speed Presets
// ==================================================
#define SPEED_LOW_DRIVE   140
#define SPEED_LOW_TURN    130

#define SPEED_HIGH_DRIVE  255
#define SPEED_HIGH_TURN   220

// Forward pulse duration while "shooting" (ms)
#define SHOOT_PULSE_MS    100

// ==================================================
// UART
// ==================================================
void uart_init(unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    // Double transmission speed mode for lower baud error at 1MHz
    UCSRA = (1 << U2X);

    // Enable Receiver and Transmitter
    UCSRB = (1 << TXEN) | (1 << RXEN);

    // Frame format: 8 data bits, 1 stop bit
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

uint8_t uart_available(void)
{
    return (UCSRA & (1 << RXC));
}

char uart_receive(void)
{
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

// ==================================================
// PWM & GPIO Initialization
// ==================================================
void pwm_init(void)
{
    // Left Motors PWM = PD5 (OC1A)
    // Right Motors PWM = PD4 (OC1B)
    DDRD |= (1 << PD5) | (1 << PD4);

    // Direction pins & Standby on PORTC
    DDRC |= (1 << AIN1) |
            (1 << AIN2) |
            (1 << BIN1) |
            (1 << BIN2) |
            (1 << STBY);

    // Enable TB6612 drivers (Pull STBY HIGH)
    PORTC |= (1 << STBY);

    // Timer1: 8-bit Fast PWM (Mode 5: WGM13:0 = 0101)
    // Clear OC1A / OC1B on compare match, set at BOTTOM (Non-inverting)
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    
    // No Prescaler (Clock / 1)
    TCCR1B = (1 << WGM12) | (1 << CS10);

    OCR1A = 0;
    OCR1B = 0;
}

// ==================================================
// Individual Motor Functions
// dir: 0 = Stop/Coast, 1 = Forward, 2 = Reverse
// ==================================================
void motor_left(uint8_t dir, uint8_t speed)
{
    if (dir == 1)
    {
        // Forward
        PORTC |= (1 << AIN1);
        PORTC &= ~(1 << AIN2);
    }
    else if (dir == 2)
    {
        // Reverse
        PORTC &= ~(1 << AIN1);
        PORTC |= (1 << AIN2);
    }
    else
    {
        // Coast / Stop
        PORTC &= ~((1 << AIN1) | (1 << AIN2));
    }

    OCR1A = speed;
}

void motor_right(uint8_t dir, uint8_t speed)
{
    if (dir == 1)
    {
        // Forward
        PORTC |= (1 << BIN1);
        PORTC &= ~(1 << BIN2);
    }
    else if (dir == 2)
    {
        // Reverse
        PORTC &= ~(1 << BIN1);
        PORTC |= (1 << BIN2);
    }
    else
    {
        // Coast / Stop
        PORTC &= ~((1 << BIN1) | (1 << BIN2));
    }

    OCR1B = speed;
}

// ==================================================
// Vehicle Direction Controls
// ==================================================
void car_forward(uint8_t speed)
{
    motor_left(1, speed);
    motor_right(1, speed);
}

void car_backward(uint8_t speed)
{
    motor_left(2, speed);
    motor_right(2, speed);
}

void car_turn_left(uint8_t speed)
{
    motor_left(2, speed);
    motor_right(1, speed);
}

void car_turn_right(uint8_t speed)
{
    motor_left(1, speed);
    motor_right(2, speed);
}

void car_stop(void)
{
    motor_left(0, 0);
    motor_right(0, 0);
}

// ==================================================
// Shoot: brief forward pulse, then resume whatever
// motion was active beforehand (or stop, if idle)
// ==================================================
void car_shoot(uint8_t drive_speed, uint8_t turn_speed, char resume_motion)
{
    // Push pulse always fires at full power, regardless of power mode
    car_forward(SPEED_HIGH_DRIVE);
    _delay_ms(SHOOT_PULSE_MS);

    switch (resume_motion)
    {
        case '2': car_forward(drive_speed);  break;
        case '8': car_backward(drive_speed); break;
        case '4': car_turn_left(turn_speed); break;
        case '6': car_turn_right(turn_speed); break;
        default:  car_stop();                break;
    }
}

// ==================================================
// Main Execution
// ==================================================
int main(void)
{
    // Disable JTAG interface to free PC2, PC3, PC4, PC5 for GPIO use
    MCUCSR |= (1 << JTD);
    MCUCSR |= (1 << JTD);

    uart_init(MYUBRR);
    pwm_init();

    // Power Mode: 0 = Low Power, 1 = High Power (defaults to Low Power)
    uint8_t is_high_power = 0;
    char current_motion = '0';

    // Set right after a shoot pulse; suppresses the very next STOP
    // command, since controllers commonly send '0' on ANY button
    // release (including the push button), which would otherwise
    // wrongly cancel a direction that is still being held.
    uint8_t suppress_next_stop = 0;

    while (1)
    {
        if (uart_available())
        {
            char cmd = uart_receive();

            if (cmd == '5')
            {
                // Toggle power mode
                is_high_power = !is_high_power;

                // Dynamically update speed if car is currently moving
                uint8_t drive_speed = is_high_power ? SPEED_HIGH_DRIVE : SPEED_LOW_DRIVE;
                uint8_t turn_speed  = is_high_power ? SPEED_HIGH_TURN  : SPEED_LOW_TURN;

                switch (current_motion)
                {
                    case '2': car_forward(drive_speed); break;
                    case '8': car_backward(drive_speed); break;
                    case '4': car_turn_left(turn_speed); break;
                    case '6': car_turn_right(turn_speed); break;
                    default:  break;
                }
            }
            else
            {
                uint8_t drive_speed = is_high_power ? SPEED_HIGH_DRIVE : SPEED_LOW_DRIVE;
                uint8_t turn_speed  = is_high_power ? SPEED_HIGH_TURN  : SPEED_LOW_TURN;

                switch (cmd)
                {
                    case '2':
                        // FRONT (FORWARD)
                        current_motion = '2';
                        suppress_next_stop = 0;
                        car_forward(drive_speed);
                        break;

                    case '8':
                        // BACK (REVERSE)
                        current_motion = '8';
                        suppress_next_stop = 0;
                        car_backward(drive_speed);
                        break;

                    case '4':
                        // LEFT TURN
                        current_motion = '4';
                        suppress_next_stop = 0;
                        car_turn_left(turn_speed);
                        break;

                    case '6':
                        // RIGHT TURN
                        current_motion = '6';
                        suppress_next_stop = 0;
                        car_turn_right(turn_speed);
                        break;

                    case '0':
                        // STOP (unless this is the spurious release-stop
                        // from the push button that was just pressed)
                        if (suppress_next_stop)
                        {
                            suppress_next_stop = 0;
                        }
                        else
                        {
                            current_motion = '0';
                            car_stop();
                        }
                        break;

                    case '1':
                        // SHOOT (brief forward pulse, then resume prior motion)
                        car_shoot(drive_speed, turn_speed, current_motion);
                        suppress_next_stop = 1;
                        break;

                    default:
                        break;
                }
            }
        }
    }

    return 0;
}
