#ifndef MINICAN_H
#define MINICAN_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>

#define START_BYTE 0xAA
#define MAX_DATA_LENGTH 8

typedef struct {
    uint8_t id;
    uint8_t len;
    uint8_t data[MAX_DATA_LENGTH];
    uint8_t crc;
} MiniCAN_Frame;

// UART Functions
void UART_init(void);
void UART_sendByte(uint8_t data);
uint8_t UART_receiveByte(void);
void UART_sendString(const char *str);

// MiniCAN Functions
uint8_t crc8(const uint8_t *data, uint8_t len);
void miniCAN_sendFrame(MiniCAN_Frame *frame);
bool miniCAN_receiveFrame(MiniCAN_Frame *frame);

#endif // MINICAN_H