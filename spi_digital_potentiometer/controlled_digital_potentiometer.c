#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define CS PB1
#define SCK PB2
#define MOSI PB0

#define BTN1 1 << PB3
#define BTN2 1 << PB4

volatile unsigned char prevBtnState = 0;

void SPIMasterInit() {
    DDRB = (1 << CS) | (1 << SCK) | (1 << MOSI);
}

void SPITransmitData(unsigned char data) {
    PORTB &= ~((1 << SCK) | (1 << MOSI));

    for (int i = 0; i < 8; ++i) {
        if (data & (1 << 7)) {
            PORTB |= (1 << MOSI);
        } else {
            PORTB &= ~(1 << MOSI);
        }

        PORTB |= (1 << SCK);

        _delay_ms(1);

        PORTB &= ~(1 << SCK);

        _delay_ms(1);

        data <<= 1;
    }

    PORTB &= ~((1 << SCK) | (1 << MOSI));
}

void digipotEnable() {
    PORTB &= ~(1 << CS);
}

void digipotDisable() {
    PORTB |= (1 << CS);
}

void digipotWriteData(unsigned char data) {
    digipotEnable();

    SPITransmitData(0b00000000);
    SPITransmitData(data);

    digipotDisable();
}

void digipotIncrement() {
    digipotEnable();

    SPITransmitData(0b00000100);

    digipotDisable();
}

void digipotDecrement() {
    digipotEnable();

    SPITransmitData(0b00001000);

    digipotDisable();
}

void interruptInit() {
    // pin change interrupt enable
    GIMSK = (1 << PCIE);

    // pin change interrupt enabled for PCINT4 && PCINT3
    PCMSK = (1 << PCINT3) | (1 << PCINT4);

    // enable interrupts
    sei();
}

ISR(PCINT0_vect) {
    unsigned char btnState = PINB & PCMSK;

    int digipotValue = 0;

    if ((btnState & BTN1) && !(prevBtnState & BTN1)) {
        digipotValue = 0;
    } else if (!(btnState & BTN1)) {
        digipotValue = 1;
    }

    if ((btnState & BTN2) && !(prevBtnState & BTN2)) {
        digipotValue = 0;
    } else if (!(btnState & BTN2)) {
        digipotValue = -1;
    }

    prevBtnState = btnState;

    if (digipotValue > 0) {
        digipotIncrement();
    } else if (digipotValue < 0) {
        digipotDecrement();
    }

    _delay_ms(1000);
}

void main() {
    interruptInit();
    SPIMasterInit();

    _delay_ms(1500);

    digipotWriteData(0);

    while (1);
}
