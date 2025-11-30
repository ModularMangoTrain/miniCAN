#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Mock definitions for testing without AVR hardware
#define START_BYTE 0xAA
#define MAX_DATA_LENGTH 8

typedef struct {
    uint8_t id;
    uint8_t len;
    uint8_t data[MAX_DATA_LENGTH];
    uint8_t crc;
} MiniCAN_Frame;

// Test buffer for simulating UART communication
static uint8_t test_buffer[32];
static uint8_t buffer_index = 0;
static uint8_t buffer_size = 0;

// Mock UART functions
void mock_UART_reset(void) {
    buffer_index = 0;
    buffer_size = 0;
    memset(test_buffer, 0, sizeof(test_buffer));
}

void mock_UART_sendByte(uint8_t data) {
    if (buffer_size < sizeof(test_buffer)) {
        test_buffer[buffer_size++] = data;
    }
}

uint8_t mock_UART_receiveByte(void) {
    if (buffer_index < buffer_size) {
        return test_buffer[buffer_index++];
    }
    return 0;
}

// CRC8 implementation (same as original)
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

// Modified miniCAN functions using mocks
void miniCAN_sendFrame(MiniCAN_Frame *frame) {
    uint8_t temp[2 + MAX_DATA_LENGTH];
    uint8_t i;
    
    temp[0] = frame->id;
    temp[1] = frame->len;
    for(i = 0; i < frame->len; i++) {
        temp[2 + i] = frame->data[i];
    }
    frame->crc = crc8(temp, 2 + frame->len);
    
    mock_UART_sendByte(START_BYTE);
    mock_UART_sendByte(frame->id);
    mock_UART_sendByte(frame->len);
    
    for(i = 0; i < frame->len; i++) {
        mock_UART_sendByte(frame->data[i]);
    }
    
    mock_UART_sendByte(frame->crc);
}

bool miniCAN_receiveFrame(MiniCAN_Frame *frame) {
    uint8_t temp[2 + MAX_DATA_LENGTH];
    uint8_t i;
    uint8_t calc_crc;
    
    if (mock_UART_receiveByte() != START_BYTE) return false;
    
    frame->id = mock_UART_receiveByte();
    frame->len = mock_UART_receiveByte();
    if (frame->len > MAX_DATA_LENGTH) return false;
    
    for(i = 0; i < frame->len; i++) {
        frame->data[i] = mock_UART_receiveByte();
    }
    
    frame->crc = mock_UART_receiveByte();
    
    temp[0] = frame->id;
    temp[1] = frame->len;
    for(i = 0; i < frame->len; i++) {
        temp[2 + i] = frame->data[i];
    }
    calc_crc = crc8(temp, 2 + frame->len);
    
    return (calc_crc == frame->crc);
}

// Test functions
void test_crc8_calculation(void) {
    printf("Testing CRC8 calculation...\n");
    
    uint8_t data1[] = {0x10, 0x03, 0x11, 0x22, 0x33};
    uint8_t expected_crc = 0x4F;
    uint8_t result = crc8(data1, 5);
    
    assert(result == expected_crc);
    printf("CRC8 basic test passed\n");
    
    // Test empty data
    uint8_t empty_crc = crc8(NULL, 0);
    assert(empty_crc == 0x00);
    printf("CRC8 empty data test passed\n");
}

void test_frame_send_receive(void) {
    printf("Testing frame send/receive...\n");
    
    MiniCAN_Frame tx_frame, rx_frame;
    
    // Setup test frame
    tx_frame.id = 0x10;
    tx_frame.len = 3;
    tx_frame.data[0] = 0x11;
    tx_frame.data[1] = 0x22;
    tx_frame.data[2] = 0x33;
    
    mock_UART_reset();
    
    // Send frame
    miniCAN_sendFrame(&tx_frame);
    
    // Reset buffer index for reading
    buffer_index = 0;
    
    // Receive frame
    bool result = miniCAN_receiveFrame(&rx_frame);
    
    assert(result == true);
    assert(rx_frame.id == tx_frame.id);
    assert(rx_frame.len == tx_frame.len);
    assert(memcmp(rx_frame.data, tx_frame.data, tx_frame.len) == 0);
    assert(rx_frame.crc == tx_frame.crc);
    
    printf("Basic send/receive test passed\n");
}

void test_frame_validation(void) {
    printf("Testing frame validation...\n");
    
    MiniCAN_Frame frame;
    
    // Test invalid length
    mock_UART_reset();
    test_buffer[0] = START_BYTE;
    test_buffer[1] = 0x10;
    test_buffer[2] = 9; // Invalid length > MAX_DATA_LENGTH
    buffer_size = 3;
    buffer_index = 0;
    
    bool result = miniCAN_receiveFrame(&frame);
    assert(result == false);
    printf("Invalid length test passed\n");
    
    // Test CRC error
    mock_UART_reset();
    test_buffer[0] = START_BYTE;
    test_buffer[1] = 0x10;
    test_buffer[2] = 2;
    test_buffer[3] = 0xAA;
    test_buffer[4] = 0xBB;
    test_buffer[5] = 0xFF; // Wrong CRC
    buffer_size = 6;
    buffer_index = 0;
    
    result = miniCAN_receiveFrame(&frame);
    assert(result == false);
    printf("CRC error test passed\n");
    
    // Test wrong start byte
    mock_UART_reset();
    test_buffer[0] = 0x55; // Wrong start byte
    buffer_size = 1;
    buffer_index = 0;
    
    result = miniCAN_receiveFrame(&frame);
    assert(result == false);
    printf("Wrong start byte test passed\n");
}

void test_edge_cases(void) {
    printf("Testing edge cases...\n");
    
    MiniCAN_Frame tx_frame, rx_frame;
    
    // Test zero-length frame
    tx_frame.id = 0xFF;
    tx_frame.len = 0;
    
    mock_UART_reset();
    miniCAN_sendFrame(&tx_frame);
    buffer_index = 0;
    
    bool result = miniCAN_receiveFrame(&rx_frame);
    assert(result == true);
    assert(rx_frame.id == 0xFF);
    assert(rx_frame.len == 0);
    printf("Zero-length frame test passed\n");
    
    // Test maximum length frame
    tx_frame.id = 0x01;
    tx_frame.len = MAX_DATA_LENGTH;
    for (int i = 0; i < MAX_DATA_LENGTH; i++) {
        tx_frame.data[i] = i + 1;
    }
    
    mock_UART_reset();
    miniCAN_sendFrame(&tx_frame);
    buffer_index = 0;
    
    result = miniCAN_receiveFrame(&rx_frame);
    assert(result == true);
    assert(rx_frame.len == MAX_DATA_LENGTH);
    assert(memcmp(rx_frame.data, tx_frame.data, MAX_DATA_LENGTH) == 0);
    printf("Maximum length frame test passed\n");
}

void test_protocol_integrity(void) {
    printf("Testing protocol integrity...\n");
    
    // Verify frame structure matches specification
    assert(sizeof(MiniCAN_Frame) >= 11); // 1+1+8+1 minimum
    assert(START_BYTE == 0xAA);
    assert(MAX_DATA_LENGTH == 8);
    
    printf("Protocol constants verified\n");
    
    // Test frame size calculation
    MiniCAN_Frame frame;
    frame.len = 3;
    uint8_t expected_frame_size = 5 + frame.len; // START+ID+LEN+DATA+CRC
    
    mock_UART_reset();
    miniCAN_sendFrame(&frame);
    
    assert(buffer_size == expected_frame_size);
    printf("Frame size calculation verified\n");
}

int main(void) {
    printf("=== MiniCAN Unit Tests ===\n\n");
    
    test_crc8_calculation();
    test_frame_send_receive();
    test_frame_validation();
    test_edge_cases();
    test_protocol_integrity();
    
    printf("\n✅ All tests passed!\n");
    printf("MiniCAN protocol implementation verified.\n");
    
    return 0;
}