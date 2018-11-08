#include <avr/io.h>
#include <util/delay.h>

#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
    #define F_CPU 20000000UL

    #define CS PINB2
    #define SCK PINB5
    #define MOSI PINB3
    #define MISO PINB4
#elif defined (__AVR_ATtiny45__)
    #define F_CPU 16000000UL

    #define CS PINB1
    #define SCK PINB2
    #define MOSI PINB0
#endif

int setBit(int n, int b) {
    return n | (1 << b);
}

int clearBit(int n, int b) {
    return n & (~(1 << b));
}

void SPIMasterInit() {
    // set PB2 (CS), PB5 (SCK) and PB0 (MOSI) to output
    DDRB = (1 << CS) | (1 << SCK) | (1 << MOSI);
}

int SPITransmitData(unsigned long data, unsigned char dataSize) {
    // assert dataSize is either 8 or 16
    int received = 0;

    for (int i = 0; i < dataSize; ++i) {
        // first - send some data
        // set SCK to HIGH
        PORTB = clearBit(PORTB, SCK);

        // if the least significant bit of data is 1 - then set MOSI pin to HIGH, otherwise - set it to LOW
        // here i'm using the bitwise AND operator, since it is way faster than the modulo 2:
        // most significant bit should go first!
        if (data & (1 << (dataSize - 1))) {
            PORTB = setBit(PORTB, MOSI);
        } else {
            PORTB = clearBit(PORTB, MOSI);
        }

        // shift data one bit
        // this is effectively the same as data * 2
        data <<= 1;

        // shift zero into the received
        // this is effectively the same as data * 2
        received = received << 1;

        // here we need to wait a little bit for slave to send us some data back
        _delay_ms(5);

        #ifdef MISO
            // if the MISO is HIGH - then set 1 on the least significant bit of received
            if (PORTB & (1 << MISO)) {
                ++received;
            }
        #endif

        PORTB = setBit(PORTB, SCK);

        _delay_ms(5);
    }

    // return what we have received
    return received;
}

void digipotEnable() {
    // select slave device
    // SPI mode 11 requires LOW to be on CS (or SS, Slave Select)
    PORTB = (0 << CS);
}

/**
 * 16-bit commands:
 *  - write data: AAAA00DD DDDDDDDD
 *  - read data:  AAAA11XX XXXXXXXX
 *
 * 8-bit commands:
 *  - increment (wiper position): AAAA01XX
 *  - decrement (wiper position): AAAA10XX
 *
 * here:
 *  - A - address bit
 *  - D - data bit, from master (MCU) to slave (digipot)
 *  - X - data bit, from slave (digipot) to master (MCU)
 *
 * addresses:
 *  - 0x00 - default register
 */
void digipotWriteData(unsigned char data) {
    digipotEnable();
    SPITransmitData(0b000000 | (data >> 6), 8);
    SPITransmitData(data << 2, 8);
    digipotDisable();
}

void digipotIncrement() {
    digipotEnable();
    SPITransmitData(0b00000100, 8);
    digipotDisable();
}

void digipotDecrement() {
    digipotEnable();
    SPITransmitData(0b00001000, 8);
    digipotDisable();
}

void digipotDisable() {
    // disable slave device
    PORTB = (1 << CS);
}

void main() {
    SPIMasterInit();

    while (1) {
        // #ifdef FOUR_STATE
        //     PORTC = 0x00;
        //     digipotWriteData(0);
        //     _delay_ms(2000);

        //     PORTC = 1 << PINC5;
        //     digipotWriteData(63);
        //     _delay_ms(2000);

        //     PORTC = 1 << PINC4;
        //     digipotWriteData(127);
        //     _delay_ms(2000);

        //     PORTC = 0xff;
        //     digipotWriteData(255);
        //     _delay_ms(2000);
        // #else
        //     for (int i = 0; i <= 255; ++i) {
        //         PORTC = 1 << PINC5;
        //         digipotIncrement();
        //         _delay_ms(250);
        //         PORTC = 0;
        //         _delay_ms(250);
        //     }

        //     for (int i = 255; i >= 0; --i) {
        //         PORTC = 1 << PINC4;
        //         digipotDecrement();
        //         _delay_ms(250);
        //         PORTC = 0;
        //         _delay_ms(250);
        //     }
        // #endif
        digipotWriteData(0);

        for (int i = 0; i <= 255; ++i) {
            // digipotIncrement();
            digipotWriteData(i);
            _delay_ms(1500);
        }

        for (int i = 255; i >= 0; --i) {
            // digipotDecrement();
            digipotWriteData(i);
            _delay_ms(1500);
        }
   }
}

