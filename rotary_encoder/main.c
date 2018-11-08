#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define ENCODER_A PB0
#define ENCODER_B PB1

void main() {
    int outputMask = (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3);
    DDRA = outputMask;
    DDRB = 0;

    int prevState = 0;
    int state = 0;

    int counter = 0;
    int counterMax = (1 << 4) - 1;

    while (1) {
        state = PINB & ((1 << ENCODER_A) | (1 << ENCODER_B));

        if (prevState != state) {
            // only update counter when we are in the end state, (0, 0)
            if (!state) {
                // if the changed bit is ENCODER_A
                if ((prevState ^ state) == (1 << ENCODER_A)) {
                    // increment; with limit up to counterMax
                    if (counter < counterMax) {
                        counter++;
                    } else {
                        counter = counterMax;
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

        PORTA = counter & outputMask;
    }
}
