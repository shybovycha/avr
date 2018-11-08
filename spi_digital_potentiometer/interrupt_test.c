#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL

volatile unsigned char prevBtnState = 0;

#define BTN1 1 << PB3
#define BTN2 1 << PB4

#define LED1 1 << PB1
#define LED2 1 << PB2

void interruptInit() {
    // pin change interrupt enable
    GIMSK = (1 << PCIE);

    // pin change interrupt enabled for PCINT4 (PB3, pin 2) && PCINT3 (PB4, pin 3)
    PCMSK = (1 << PCINT3) | (1 << PCINT4);

    // enable interrupts
    sei();
}

ISR(PCINT0_vect) {
    unsigned char btnState = PINB & PCMSK;

    if ((btnState & BTN1) && !(prevBtnState & BTN1)) {
        PORTB &= ~LED1;
    } else if (!(btnState & BTN1)) {
        PORTB |= LED1;
    }

    if ((btnState & BTN2) && !(prevBtnState & BTN2)) {
        PORTB &= ~LED2;
    } else if (!(btnState & BTN2)) {
        PORTB |= LED2;
    }

    prevBtnState = btnState;
}

void main() {
    DDRB = LED1 | LED2;
    PORTB = 0;

    interruptInit();

    while (1);
}
