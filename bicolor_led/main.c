/* =========================================================================
 * Bicolor 8x8 LED Matrix Driver - ATmega32
 * -------------------------------------------------------------------------
 * Permanent-wiring version: all 24 wires connected at once.
 *   PA -> rows (anode, active HIGH)
 *   PC -> red columns   (cathode, active LOW)   [JTAG disabled - see below]
 *   PD -> green columns (cathode, active LOW)
 *
 * Because both color ports are driven within the same row-multiplexing
 * step, red and green pixels can be lit independently AND simultaneously
 * (which mixes to amber/yellow on a common bicolor LED). No more
 * physically re-wiring between the red demo and the green demo - just
 * fill in the red and green pattern arrays separately.
 * ========================================================================= */

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

/* =========================== CONFIG ==================================== */

/* Port driving the ROWS (anode, active HIGH). */
#define ROW_PORT   PORTA
#define ROW_DDR    DDRA

/* Port driving the RED columns (cathode, active LOW). */
#define RED_COL_PORT   PORTC
#define RED_COL_DDR    DDRC

/* Port driving the GREEN columns (cathode, active LOW). */
#define GREEN_COL_PORT   PORTD
#define GREEN_COL_DDR    DDRD

/* PORTC pins C2-C5 double as JTAG (TCK/TMS/TDO/TDI) on the ATmega32.
 * Since red columns now live on PORTC, JTAG must be disabled or those
 * pins won't behave as plain GPIO. Keep this at 1. */
#define DISABLE_JTAG 1

/* Per-row on-time during multiplexing, in ms. Tune by trial and error:
 * too short -> dim symbol, too long -> visible row-by-row flicker. */
#define ROW_DELAY_MS 0.5

/* How many full-matrix refresh passes each mode/phase stays on screen.
 * Bigger number = longer on screen, not brighter (brightness is
 * ROW_DELAY_MS). Adjust to taste. */
#define STATIC_REFRESH_COUNT     150
#define FLASH_ON_REFRESH_COUNT   100
#define FLASH_OFF_REFRESH_COUNT  100
#define ROTATE_REFRESH_PER_PHASE 200

/* Number of frames in the rotation animation. Must equal the number of
 * rows in image_rotate_red[][] / image_rotate_green[][] below. */
#define NUM_PHASES 4

/* Which way to play the rotation frames.
 *   1 -> play phases 0, 1, 2, ... NUM_PHASES-1  (as generated)
 *   0 -> play phases NUM_PHASES-1, ... 1, 0     (reverse order) */
#define ROTATE_FORWARD 1

/* ========================================================================= */

/* Static symbols for part 1 and 2 (static display + flash).
 * Fill each independently with hex_to_c.py output - run it once per
 * color. Wherever a bit is 0 in BOTH arrays for the same row/column,
 * you'll see amber/yellow instead of pure red or green. */
volatile unsigned char image_red[8]
= { 0b11101111, 0b11100111, 0b11101011, 0b11101111, 0b11101111, 0b10000011, 0b11111111, 0b11111111 };

volatile unsigned char image_green[8]
= { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 };

/* Rotation animation frames for part 3 - one set per color. Keep
 * NUM_PHASES above in sync with however many frames you generate. */
volatile unsigned char image_rotate_red[NUM_PHASES][8]
= {
    { 0b11101111, 0b11100111, 0b11101011, 0b11101111, 0b11101111, 0b10000011, 0b11111111, 0b11111111 },
    { 0b11111111, 0b11111111, 0b11011011, 0b10111011, 0b00000011, 0b11111011, 0b11111011, 0b11111111 },
    { 0b11111111, 0b11111111, 0b11000001, 0b11110111, 0b11110111, 0b11010111, 0b11100111, 0b11110111 },
    { 0b11111111, 0b11011111, 0b11011111, 0b11000000, 0b11011101, 0b11011011, 0b11111111, 0b11111111 }
};

volatile unsigned char image_rotate_green[NUM_PHASES][8]
= {
    { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 },
    { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 },
    { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 },
    { 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111 }
};

/* All columns off (active LOW, so 1 = off). One array is enough since
 * "all off" looks the same on either color port. */
volatile unsigned char blankscreen[8] =
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* Row selector: a single '1' bit that walks down through the rows, one
 * HIGH row at a time (top -> bottom, then wraps back to top). */
volatile unsigned char row_selector = 0b10000000;

unsigned char next_row_selector(unsigned char current)
{
    unsigned char next = current >> 1;
    if (!next) {
        next = 0b10000000;   /* wrap back to row 1 */
    }
    return next;
}

/* One full multiplexed refresh of the matrix, showing "red_pattern" on
 * the red columns and "green_pattern" on the green columns at the same
 * time. Call this repeatedly (see show_static/show_flash/show_rotation)
 * to keep an image visibly lit via persistence of vision. */
void show_pattern(volatile unsigned char red_pattern[8], volatile unsigned char green_pattern[8])
{
    unsigned char r = row_selector;
    ROW_PORT = r;
    for (int i = 0; i < 8; i++) {
        RED_COL_PORT   = red_pattern[i];
        GREEN_COL_PORT = green_pattern[i];
        _delay_ms(ROW_DELAY_MS);
        r = next_row_selector(r);
        ROW_PORT = r;
    }
}

/* Part 1: static display. */
void show_static(void)
{
    for (int i = 0; i < STATIC_REFRESH_COUNT; i++) {
        show_pattern(image_red, image_green);
    }
}

/* Part 2: flashing display. */
void show_flash(void)
{
    for (int i = 0; i < FLASH_ON_REFRESH_COUNT; i++) {
        show_pattern(image_red, image_green);
    }
    for (int i = 0; i < FLASH_OFF_REFRESH_COUNT; i++) {
        show_pattern(blankscreen, blankscreen);
    }
}

/* Part 3: rotating display (left or right, controlled by ROTATE_FORWARD). */
void show_rotation(void)
{
#if ROTATE_FORWARD
    for (int phase = 0; phase < NUM_PHASES; phase++) {
#else
    for (int phase = NUM_PHASES - 1; phase >= 0; phase--) {
#endif
        for (int j = 0; j < ROTATE_REFRESH_PER_PHASE; j++) {
            show_pattern(image_rotate_red[phase], image_rotate_green[phase]);
        }
    }
}

int main(void)
{
    ROW_DDR = 0xFF;
    RED_COL_DDR = 0xFF;
    GREEN_COL_DDR = 0xFF;

#if DISABLE_JTAG
    MCUCSR = (1 << JTD);
    MCUCSR = (1 << JTD);   /* must be written twice consecutively */
#endif

    while (1) {
        /* Comment out whichever parts you're not demoing right now.
         * Leaving all three uncommented just cycles through them. */
        show_static();
        show_flash();
        show_rotation();
    }
}