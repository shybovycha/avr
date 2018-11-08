#ifndef F_CPU
#define F_CPU 20000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

// for ATmega328p
#define CS PINB2
#define SCK PINB5
#define MOSI PINB3

void SPI_Master_Init() {
    // set PB2 (CS), PB5 (SCK) and PB0 (MOSI) to output
    DDRB = (1 << CS) | (1 << SCK) | (1 << MOSI); 

    // enable SPI, Master and set SPI clock rate to F_CPU / 16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); 
}

int SPI_Transmit_Data(unsigned char data) {
    // put some data into SPI data register
    SPDR = data;

    // wait until SPI transmission is complete
    while (!( SPSR & (1 << SPIF) ));

    // return the data from the SPI data register
    return SPDR;
}

void digipot_Enable() {
    // write 1 to PB1
    PORTB = (0 << CS); 
}

void digipot_Set_Step(int step) {
    // 0b00 - write data / wiper position
    // 0b01 - increment wiper position/potentiometer value
    // 0b10 - decrement wiper position
    // 0b11 - read data / wiper position
    int digipotRegisterAddr = 0x00;

    SPI_Transmit_Data(digipotRegisterAddr);
    SPI_Transmit_Data(step);
}

void digipot_Disable() {
    PORTB = (1 << CS);
}

void main() {
    SPI_Master_Init();

    int step = 0;
    int maxStep = 255;
    int minStep = 0;
    int deltaStep = 1;

    DDRC = (1 << PINC4) | (1 << PINC5);

    while (1) {
        /*for (step = 0; step < 255; ++step) {
            digipot_Enable();
            _delay_ms(50);

            digipot_Set_Step(step);

            digipot_Disable();

            _delay_ms(50);
        }

        for (step = 254; step > 0; --step) {
            digipot_Enable();
            _delay_ms(50);

            digipot_Set_Step(step);

            digipot_Disable();

            _delay_ms(50);
        }*/

        digipot_Enable();
        digipot_Set_Step(0);
        digipot_Disable();
        PORTC = 0xff;
        _delay_ms(1500);

        digipot_Enable();
        digipot_Set_Step(80);
        digipot_Disable();
        PORTC = 0x00;
        _delay_ms(1500);

        digipot_Enable();
        digipot_Set_Step(160);
        digipot_Disable();
        PORTC = 0xff;
        _delay_ms(1500);

        digipot_Enable();
        digipot_Set_Step(240);
        digipot_Disable();
        PORTC = 0;
        _delay_ms(1500);
    }
}

