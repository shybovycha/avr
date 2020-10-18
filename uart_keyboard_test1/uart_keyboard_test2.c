/*
 * keyboard_uart_test1.c
 *
 * Created: 11/10/2020 1:06:42 PM
 * Author: Artem Shubovych
 */

#include <avr/io.h>
#include <util/delay.h>

#include "hid_keys.h"

// #define __DEBUG__

// #define __MASTER__
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

#define COLS_PORT_PORT _EXPAND_PORT(COLUMNS_PORT)
#define COLS_PORT_DDR _EXPAND_PORT_DDR(COLUMNS_PORT)
#define COLS_PORT_PIN _EXPAND_PORT_PIN(COLUMNS_PORT)

#ifdef __DEBUG__
  #define HANDSHAKE_BYTE 'Q' // For debug purposes only
#else
  #define HANDSHAKE_BYTE 0b11000011 // unused bits should all be HIGH
#endif

void USARTInit()
{
  #ifdef __AVR_ATmega328__
    // Set baud rate
    UBRR0H = (uint8_t) (UBRR_VALUE >> 8);
    UBRR0L = (uint8_t) UBRR_VALUE;

    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);

    //enable transmission and reception
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
  #endif

  #ifdef __AVR_ATmega8__
    // Set baud rate
    UBRRH = (uint8_t) (UBRR_VALUE >> 8);
    UBRRL = (uint8_t) UBRR_VALUE;

    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);

    //enable transmission and reception
    UCSRB |= (1 << RXEN) | (1 << TXEN);
  #endif
}

inline void USARTSendByte(uint8_t u8Data)
{
  #ifdef ATMEGA328
    //wait while previous byte is completed
    while (!(UCSR0A & (1 << UDRE0)));

    // Transmit data
    UDR0 = u8Data;
  #endif

  #ifdef ATMEGA8
    //wait while previous byte is completed
    while (!(UCSRA & (1 << UDRE)));

    // Transmit data
    UDR = u8Data;
  #endif
}

inline uint8_t USARTReceiveByte()
{
  #ifdef ATMEGA328
    // Wait for byte to be received
    while (!(UCSR0A & (1 << RXC0)));

    // Return received data
    return UDR0;
  #endif

  #ifdef ATMEGA8
   // Wait for byte to be received
    while (!(UCSRA & (1 << RXC)));

    // Return received data
    return UDR;
  #endif
}

void processRow(
    uint8_t *currentRowState,
    uint8_t *previousRowState,
    uint8_t row,
    uint8_t *buffer,
    uint8_t *bufferIdx,
    const uint8_t *KEYS
) {

    uint8_t previousRowState1 = previousRowState[row];
    uint8_t rowState1 = currentRowState[row];

    previousRowState[row] = currentRowState[row];

    for (int col = 0; col < COLUMNS; ++col) {
        short wasKeyDown = previousRowState1 & 1;
        short isKeyStillDown = rowState1 & 1;

        if (wasKeyDown != isKeyStillDown) {
            uint8_t idx = *bufferIdx << 1;
            buffer[idx] = KEYS[(row * COLUMNS) + col];
            buffer[++idx] = isKeyStillDown;

            *bufferIdx = idx;
        }

        previousRowState1 >>= 1;
        rowState1 >>= 1;
    }
}

int main(void)
{
  // Initialize USART0; naming is slightly different in ATMega8 and ATMega328
  USARTInit();

  // init pins for keyboard IO
  ROWS_PORT_DDR = 0b11111000; // PORTC0..PORTC4 are rows, all used for OTPUT (DDRCi = HIGH)
  COLS_PORT_DDR = 0b00000000; // PORTB0..PORTB5 are columns, all used for INPUT (DDRBi = LOW)

  #ifdef __MASTER__
    const uint8_t KEYS_SLAVE[ROWS * COLUMNS] = {
        KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
        KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T,
        KEY_LEFTSHIFT, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G,
        KEY_LEFTCTRL, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B,
        KEY_SPACE, KEY_BACKSPACE, KEY_LEFTALT, KEY_LEFT, KEY_RIGHT,
    };

    const uint8_t KEYS_MASTER[ROWS * COLUMNS] = {
        KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS,
        KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_BACKSLASH,
        KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE,
        KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHTSHIFT,
        KEY_UP, KEY_DOWN, KEY_LEFTBRACE, KEY_RIGHTBRACE, KEY_ENTER,
    };

    uint8_t READ_DELAY_US = 4;

    uint8_t masterRowStateBuffer[ROWS];
    uint8_t slaveRowStateBuffer[ROWS];

    uint8_t previousMasterRowStateBuffer[ROWS];
    uint8_t previousSlaveRowStateBuffer[ROWS];

    uint8_t QUEUE_SIZE = 64;
    uint8_t keyQueue[QUEUE_SIZE * 2];
    uint8_t keyQueueIdx = 0;

    while (1)
    {
      // get master's keyboard state
      uint8_t rowMask = 1;

      for (int row = 0; row < ROWS; ++row)
      {
        ROWS_PORT_PORT = rowMask;
        _delay_us(READ_DELAY_US);
        masterRowStateBuffer[row] = COLS_PORT_PIN;
        rowMask <<= 1;
      }

      // receive slave's keyboard state
      // skip unnecessary bytes
      while (USARTReceiveByte() != HANDSHAKE_BYTE);

      for (int row = 0; row < ROWS; ++row)
      {
        slaveRowStateBuffer[row] = USARTReceiveByte();
      }

      // check which keys changed their states and send that data over USB
      for (int row = 0; row < ROWS; ++row)
      {
        // do this for master
        processRow(masterRowStateBuffer, previousMasterRowStateBuffer, row, keyQueue, &keyQueueIdx, KEYS_MASTER);

        // follow the same logic for slave data
        processRow(slaveRowStateBuffer, previousSlaveRowStateBuffer, row, keyQueue, &keyQueueIdx, KEYS_SLAVE);
      }

      // at this point, send the keyQueue over USB
    }
  #else
    uint8_t rowStateBuffer[ROWS];
    uint8_t READ_DELAY_US = 4;

    while (1)
    {
      // read keyboard state
      uint8_t rowMask = 1;

      for (int i = 0; i < ROWS; ++i)
      {
        ROWS_PORT_PORT = rowMask;
        _delay_us(READ_DELAY_US);
        rowStateBuffer[i] = COLS_PORT_PIN;
        rowMask <<= 1;
      }

      // send the state over USART
      USARTSendByte(HANDSHAKE_BYTE);

      #ifdef __DEBUG__
        USARTSendByte('/');

        for (int i = 0; i < ROWS; ++i)
        {
          for (int t = 0; t < 8; ++t)
          {
            if (rowStateBuffer[i] & (1 << t))
            {
              USARTSendByte('1');
            } else
            {
              USARTSendByte('0');
            }
          }

          USARTSendByte(';');
        }

        USARTSendByte('\r');
        USARTSendByte('\n');
      #else
        for (int i = 0; i < ROWS; ++i)
        {
          USARTSendByte(rowStateBuffer[i]);
        }
      #endif
    }
  #endif
}
