/**
 *
 * Author: Artem Shubovych
 * Date: 2 Feb 2018
 *
 * Keyboard driver for ATtiny45-based prototype.
 *
 * Compile with:
 *
 *   avr-gcc -g -Os -mmcu=attiny45 -c main.c
 *
 * Note the `-Os` compiler flag - this will *dramatically* reduce the generated assembly code (~ 10x).
 *
 * Generic algorithm:
 *
 *
 */

#include <avr/io.h>
#include <util/delay.h>

// set CPU frequency to 20 MHz
#define F_CPU 20000000UL

#define ROWS 4
#define COLS 3

int main() {
    // PCINT0, PCINT1, PCINT2, PCINT3, PCINT5 - output, PCINT4 - input
    // e. g. PCINT = 0b11110111
    DDRB = 0b11110111;

    // turn off all the pins
    PORTB = 0x00;

    // turn on PCINT5 to switch demultiplexers on
    PORTB = 1 << 5;

    // keyboard state: 0 - key is up, 1 - key is down
    char state[ROWS * COLS];

    // iterators
    int currentRow, currentCol;

    // key state
    char newState, oldState;

    // key number
    int keyIndex;

    while (1) {
        for (currentRow = 0; currentRow < ROWS; ++currentRow) {
            // increment row counter
            PORTB ^= 1 << 0;
            _delay_ms(10);
            PORTB ^= 1 << 0;

            for (currentCol = 0; currentCol < COLS; ++currentCol) {
                // increment col counter
                PORTB ^= 1 << 2;
                _delay_ms(10);
                PORTB ^= 1 << 2;

                keyIndex = (currentRow * ROWS) + currentCol;

                oldState = state[keyIndex];
                newState = PORTB & (1 << 4);

                // if state has changed
                if (oldState != newState) {
                    if (oldState == 0) {
                        // key was pressed
                    } else {
                        // key was released
                    }
                }

                state[keyIndex] = newState;
            }

            // reset col counter
            PORTB ^= 1 << 3;
            _delay_ms(1);
            PORTB ^= 1 << 3;
        }

        // reset row counter
        PORTB ^= 1 << 1;
        _delay_ms(1);
        PORTB ^= 1 << 1;
    }

    return 0;
}