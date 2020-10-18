/*
 * keyboard_uart_test1.c
 *
 * Created: 11/10/2020 1:06:42 PM
 * Author: Artem Shubovych
 */

#include <avr/io.h>
#include <util/delay.h>

// #define __DEBUG__

#define __MASTER__
// #define __SLAVE__

#define USART_BAUDRATE 9600
#define UBRR_VALUE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define ROWS 5
#define COLUMNS 6

#define ROWS_PORT C
#define COLUMNS_PORT B

#define _PORT_PORT(p) PORT##p
#define _PORT_DDR(p) DDR##p
#define _PORT_PIN(p) PIN##p

#define _EXPAND_PORT(p) _PORT_PORT(p)
#define _EXPAND_PORT_DDR(p) _PORT_DDR(p)
#define _EXPAND_PORT_PIN(p) _PORT_PIN(p)

#define ROWS_PORT_PORT _EXPAND_PORT(ROWS_PORT)
#define ROWS_PORT_DDR _EXPAND_PORT_DDR(ROWS_PORT)
#define ROWS_PORT_PIN _EXPAND_PORT_PIN(ROWS_PORT)

#define COLS_PORT_PORT _EXPAND_PORT(COLS_PORT)
#define COLS_PORT_DDR _EXPAND_PORT_DDR(COLS_PORT)
#define COLS_PORT_PIN _EXPAND_PORT_PIN(COLS_PORT)

#ifdef __DEBUG__
#define HANDSHAKE_BYTE 'Q' // For debug purposes only
#else
#define HANDSHAKE_BYTE 0b11000011 // unused bits should all be HIGH
#endif

void USART0Init(void)
{
    // Set baud rate
    UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
    UBRR0L = (uint8_t) UBRR_VALUE;

    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

    //enable transmission and reception
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

void USART0SendByte(uint8_t u8Data)
{
    //wait while previous byte is completed
    while (!(UCSR0A & (1 << UDRE0)));

    // Transmit data
    UDR0 = u8Data;
}

uint8_t USART0ReceiveByte()
{
    // Wait for byte to be received
    while (!(UCSR0A & (1 << RXC0)));

    // Return received data
    return UDR0;
}

void initPins(void)
{
    ROWS_PORT_DDR = 0b11111000; // PORTC0..PORTC4 are rows, all used for OTPUT (DDRCi = HIGH)
    COLS_PORT_DDR = 0b00000000; // PORTB0..PORTB5 are columns, all used for INPUT (DDRBi = LOW)
}

void readKeyboardState(uint8_t* buffer)
{
    uint8_t READ_DELAY_US = 4;

    for (int i = 0; i < ROWS; ++i)
    {
        ROWS_PORT_PORT = (1 << i);
        _delay_us(READ_DELAY_US);
        buffer[i] = COLS_PORT_PIN;
    }
}

// TODO: this is a very naive implementation
uint8_t readRowState(uint8_t row)
{
    // set the row in question to HIGH and read all the columns
    ROWS_PORT_PORT |= (1 << row);
    return COLUMNS_PORT_PIN;
}

// TODO: this is a very naive implementation
void resetRowState(uint8_t row)
{
    ROWS_PORT_PORT &= ~(1 << row);
}

void checkRowAndSendStateChangesOverUsb(uint8_t previousRowState, uint8_t rowState, uint8_t columns)
{
    if (rowState != previousRowState)
    {
        for (int column = 0; column < columns; ++column)
        {
            uint8_t columnBitMask = (1 << column);
            uint8_t previousButtonState = (previousRowState & columnBitMask);
            uint8_t currentButtonState = (rowState & columnBitMask);

            if (previousButtonState != currentButtonState)
            {
                // TODO: send this change over USB
            }
        }
    }
}

// master has no need to send data over USART, only read
void masterMain(void)
{
    uint8_t masterRowStateBuffer[ROWS];
    uint8_t slaveRowStateBuffer[ROWS];

    uint8_t previousMasterRowStateBuffer[ROWS];
    uint8_t previousSlaveRowStateBuffer[ROWS];

    while (1)
    {
        // get master's keyboard state
        readKeyboardState(masterRowStateBuffer, ROWS);

        // receive slave's keyboard state
        // skip unnecessary bytes
        while (USART0ReceiveByte() != HANDSHAKE_BYTE);

        for (int row = 0; row < ROWS; ++row)
        {
            slaveRowStateBuffer[row] = USART0ReceiveByte();
        }

        // check which keys changed their states and send that data over USB
        for (int row = 0; row < ROWS; ++row)
        {
            // do this for master
            checkRowAndSendStateChangesOverUsb(previousMasterRowStateBuffer[row], masterRowStateBuffer[row], COLUMNS);

            // follow the same logic for slave data
            checkRowAndSendStateChangesOverUsb(previousSlaveRowStateBuffer[row], slaveRowStateBuffer[row], COLUMNS);

            // swap current and previous states for buffers
            previousMasterRowStateBuffer[row] = masterRowStateBuffer[row];
            previousSlaveRowStateBuffer[row] = slaveRowStateBuffer[row];
        }
    }
}

// slave only sends data over USART, never reading anything back
void slaveMain(void)
{
    uint8_t rowStateBuffer[ROWS];

    while (1)
    {
        readKeyboardState(rowStateBuffer);

        USART0SendByte(HANDSHAKE_BYTE);

        #ifdef __DEBUG__
            USART0SendByte('/');

            for (int i = 0; i < ROWS; ++i)
            {
                for (int t = 0; t < 8; ++t)
                {
                    if (rowStateBuffer[i] & (1 << t))
                    {
                        USART0SendByte('1');
                    } else
                    {
                        USART0SendByte('0');
                    }
                }

                USART0SendByte(';');
            }

            USART0SendByte('\r');
            USART0SendByte('\n');
        #else
            for (int i = 0; i < ROWS; ++i)
            {
                USART0SendByte(rowStateBuffer[i]);
            }
        #endif
    }
}

int main(void)
{
    // Initialize USART0
    USART0Init();

    initPins();

    #ifdef __MASTER__
        masterMain();
    #else
        slaveMain();
    #endif
}
