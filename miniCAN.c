#include <avr/io.h>
#include <util/delay.h>
#include <until/setbaud.h>
#include "miniCAN.h"

#define F_CPU 12000000UL
#define BAUD 57600 

#define START_BYTE 0xAA
#define MAX_DATA 8
#define USE_2X 0 // Can change depending on usage

// ---------- UART ----------
void UART_init(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    #if USE_2X
    UCSR0A = (UCSR0A & ~(1 << U2X0)) | (USE_2X << U2X0);
    #else
    UCSR0A = 0;
    #endif


    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_sendByte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

uint8_t UART_receiveByte(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

// ---------- CRC8 ----------
uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x07;
            else crc <<= 1;
        }
    }
    return crc;
}

// ---------- miniCAN ----------

void miniCAN_sendFrame(MiniCAN_Frame *f) {
    UART_sendByte(START_BYTE);
    UART_sendByte(f->id);
    UART_sendByte(f->len);
    for (uint8_t i = 0; i < f->len; i++) UART_sendByte(f->data[i]);
    UART_sendByte(f->crc);
}

bool miniCAN_receiveFrame(MiniCAN_Frame *f) {
    uint8_t byte = UART_receiveByte();
    if (byte != START_BYTE) return false;

    f->id = UART_receiveByte();
    f->len = UART_receiveByte();
    if (f->len > MAX_DATA) return false;

    for (uint8_t i = 0; i < f->len; i++)
        f->data[i] = UART_receiveByte();

    f->crc = UART_receiveByte();
    uint8_t calc_crc = crc8(&f->id, 2 + f->len);
    return (calc_crc == f->crc);
}


