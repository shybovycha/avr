#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

#define CS PB1
#define SCK PB2
#define MOSI PB0

void SPIMasterInit() {
    // set CS, SCK and MOSI to output
    DDRB = (1 << CS) | (1 << SCK) | (1 << MOSI);
}

int SPITransmitData(unsigned char data) {
    // SPI has two modes - 0,0 and 1,1 determined by the state of the SCK bit
    // at the moment when CS goes from inactive (HIGH) to active (LOW)

    // enable SPI mode 0,0:
    //   1. set SCK to LOW
    //   2. clear the MOSI output
    PORTB &= ~((1 << SCK) | (1 << MOSI));

    int received = 0;

    for (int i = 0; i < 8; ++i) {
        /* ====== writing stage ======== */

        // first - send some data
        // if the least significant bit of data is 1 - then set MOSI pin to HIGH, otherwise - set it to LOW
        // here i'm using the bitwise AND operator, since it is way faster than the modulo 2
        // most significant bit should go first!
        if (data & (1 << 7)) {
            PORTB |= (1 << MOSI);
        } else {
            PORTB &= ~(1 << MOSI);
        }

        // only after the data is about to be sent, send SCK
        // this will determine SPI mode: 1,1 or 0,0
        // set SCK to LOW
        PORTB |= (1 << SCK);

        // here we need to wait a little bit for slave to send us some data back
        _delay_ms(1);

        // shut down the SCK signal
        PORTB &= ~(1 << SCK);

        // shift zero into the received
        // this is effectively the same as data * 2
        received <<= 1;

        _delay_ms(1);

        // shift data one bit
        // this is effectively the same as data * 2
        data <<= 1;
    }

    // clear ports afterwards
    PORTB &= ~((1 << SCK) | (1 << MOSI));

    // return what we have received
    return received;
}

void digipotEnable() {
    PORTB &= ~(1 << CS);
}

void digipotDisable() {
    PORTB |= (1 << CS);
}

/**
 * 16-bit commands:
 *  - write data: AAAA00ZD DDDDDDDD
 *  - read data:  AAAA11ZX XXXXXXXX
 *
 * 8-bit commands:
 *  - increment (wiper position): AAAA01ZX
 *  - decrement (wiper position): AAAA10ZX
 *
 * here:
 *  - A - address bit
 *  - D - data bit, from master (MCU) to slave (digipot)
 *  - X - data bit, from slave (digipot) to master (MCU)
 *  - Z - error bit, LOW when an error occurs
 *
 * addresses:
 *  - 0x00 - default register
 */
int digipotWriteData(unsigned char data) {
    digipotEnable();

    SPITransmitData(0b00000000);
    int result = SPITransmitData(data);

    digipotDisable();

    return result;
}

int digipotIncrement() {
    digipotEnable();

    int result = SPITransmitData(0b00000100);

    digipotDisable();

    return result;
}

int digipotDecrement() {
    digipotEnable();

    int result = SPITransmitData(0b00001000);

    digipotDisable();

    return result;
}

void main() {
    SPIMasterInit();

    while (1) {
        digipotWriteData(0);
        _delay_ms(2000);

        digipotWriteData(64);
        _delay_ms(2000);

        digipotWriteData(128);
        _delay_ms(2000);

        digipotWriteData(192);
        _delay_ms(2000);

        digipotWriteData(255);
        _delay_ms(2000);

        digipotWriteData(192);
        _delay_ms(2000);

        digipotWriteData(128);
        _delay_ms(2000);

        digipotWriteData(64);
        _delay_ms(2000);
   }
}
