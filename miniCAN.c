#include "miniCAN.h"
#include <util/delay.h>

// ---------- UART Functions ----------
void UART_init(void) {
    // Set pin directions: PD1 as output (TXD), PD0 as input (RXD)
    DDRD |= (1 << PD1);
    DDRD &= ~(1 << PD0);
    
    // Manual baud rate calculation for 12MHz, 9600 baud
    UBRR0H = 0;
    UBRR0L = 77;  // 12000000 / (16 * 9600) - 1 = 77.083 → 77
    
    // Enable transmitter and receiver, 8-bit data
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}

void UART_sendByte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for buffer empty
    UDR0 = data;
}

uint8_t UART_receiveByte(void) {
    while (!(UCSR0A & (1 << RXC0))); // Wait for data received
    return UDR0;
}

void UART_sendString(const char *str) {
    while (*str) {
        UART_sendByte(*str++);
    }
}

// ---------- CRC8 Calculation ----------
uint8_t crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00;
    uint8_t i, j;
    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) crc = (crc << 1) ^ 0x07;
            else crc <<= 1;
        }
    }
    return crc;
}

// ---------- MiniCAN Functions ----------
void miniCAN_sendFrame(MiniCAN_Frame *frame) {
    uint8_t temp[2 + MAX_DATA_LENGTH];
    uint8_t i;
    
    // Calculate CRC for the frame (ID + LEN + DATA)
    temp[0] = frame->id;
    temp[1] = frame->len;
    for(i = 0; i < frame->len; i++) {
        temp[2 + i] = frame->data[i];
    }
    frame->crc = crc8(temp, 2 + frame->len);
    
    // Send frame via UART
    UART_sendByte(START_BYTE);
    UART_sendByte(frame->id);
    UART_sendByte(frame->len);
    
    for(i = 0; i < frame->len; i++) {
        UART_sendByte(frame->data[i]);
    }
    
    UART_sendByte(frame->crc);
}

bool miniCAN_receiveFrame(MiniCAN_Frame *frame) {
    uint8_t temp[2 + MAX_DATA_LENGTH];
    uint8_t i;
    uint8_t calc_crc;
    
    // Wait for start byte
    if (UART_receiveByte() != START_BYTE) return false;
    
    frame->id = UART_receiveByte();
    frame->len = UART_receiveByte();
    if (frame->len > MAX_DATA_LENGTH) return false;
    
    for(i = 0; i < frame->len; i++) {
        frame->data[i] = UART_receiveByte();
    }
    
    frame->crc = UART_receiveByte();
    
    // Verify CRC
    temp[0] = frame->id;
    temp[1] = frame->len;
    for(i = 0; i < frame->len; i++) {
        temp[2 + i] = frame->data[i];
    }
    calc_crc = crc8(temp, 2 + frame->len);
    
    return (calc_crc == frame->crc);
}