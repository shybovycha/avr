#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define ENCODER_A PB0
#define ENCODER_B PB1

#define COUNTER_MAX (1 << 3) - 1

#define OUTPUT_MASK (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3)

void main() {
    DDRA = OUTPUT_MASK;
    DDRB = 0;

    int prevState = 0;
    int state = 0;

    int counter = 0;

    while (1) {
        state = PINB & ((1 << ENCODER_A) | (1 << ENCODER_B));

        if (prevState != state) {
            // only update counter when we are in the end state, (0, 0)
            if (!state) {
                // if the changed bit is ENCODER_A
                if ((prevState ^ state) == (1 << ENCODER_A)) {
                    // increment; with limit up to COUNTER_MAX
                    if (counter < COUNTER_MAX) {
                        counter++;
                    } else {
                        counter = COUNTER_MAX;
                    }
                } else {
                    // increment; with limit down to 0
                    if (counter > 0) {
                        counter--;
                    } else {
                        counter = 0;
                    }
                }
            }

            prevState = state;
        }

        PORTA = counter & OUTPUT_MASK;
    }
}
