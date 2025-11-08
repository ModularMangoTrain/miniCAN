#ifndef MINICAN_H
#define MINICAN_H

#include <stdint.h>
#include <stdbool.h>

#define START_BYTE 0xAA
#define MAX_DATA 8

typedef struct{
    uint8_t id;
    uint8_t len;
    uint8_t data[MAX_DATA];
    uint8_t crc;
} MiniCAN_Frame;

void UART_init(void);
void UART_sendByte(uint8_t data);
uint8_t UART_receiveByte(void);
uint8_t crc8(const uint8_t *data, uint8_t len);
void miniCAN_sendFrame(MiniCAN_Frame *f);
bool miniCAN_receiveFrame(MiniCAN_Frame *f);

#endif // MINICAN_H