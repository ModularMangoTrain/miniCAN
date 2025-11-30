#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// Include test framework
#include "test_miniCAN.c"

// Integration test scenarios
void test_multiple_frames(void) {
    printf("Testing multiple frame transmission...\n");
    
    MiniCAN_Frame frames[5];
    MiniCAN_Frame received[5];
    
    // Create test frames
    for (int i = 0; i < 5; i++) {
        frames[i].id = 0x20 + i;
        frames[i].len = (i % 8) + 1;
        for (int j = 0; j < frames[i].len; j++) {
            frames[i].data[j] = (i * 10) + j;
        }
    }
    
    // Send all frames
    mock_UART_reset();
    for (int i = 0; i < 5; i++) {
        miniCAN_sendFrame(&frames[i]);
    }
    
    // Receive all frames
    buffer_index = 0;
    for (int i = 0; i < 5; i++) {
        bool result = miniCAN_receiveFrame(&received[i]);
        assert(result == true);
        assert(received[i].id == frames[i].id);
        assert(received[i].len == frames[i].len);
        assert(memcmp(received[i].data, frames[i].data, frames[i].len) == 0);
    }
    
    printf("Multiple frames test passed\n");
}

void test_stress_random_data(void) {
    printf("Testing stress with random data...\n");
    
    srand(time(NULL));
    
    for (int test = 0; test < 100; test++) {
        MiniCAN_Frame tx_frame, rx_frame;
        
        // Generate random frame
        tx_frame.id = rand() % 256;
        tx_frame.len = rand() % (MAX_DATA_LENGTH + 1);
        
        for (int i = 0; i < tx_frame.len; i++) {
            tx_frame.data[i] = rand() % 256;
        }
        
        mock_UART_reset();
        miniCAN_sendFrame(&tx_frame);
        buffer_index = 0;
        
        bool result = miniCAN_receiveFrame(&rx_frame);
        assert(result == true);
        assert(rx_frame.id == tx_frame.id);
        assert(rx_frame.len == tx_frame.len);
        assert(memcmp(rx_frame.data, tx_frame.data, tx_frame.len) == 0);
    }
    
    printf("Random data stress test passed (100 frames)\n");
}

void test_error_injection(void) {
    printf("Testing error injection scenarios...\n");
    
    MiniCAN_Frame frame;
    
    // Test single bit flip in CRC
    mock_UART_reset();
    frame.id = 0x42;
    frame.len = 4;
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    frame.data[3] = 0xEF;
    
    miniCAN_sendFrame(&frame);
    
    // Flip one bit in CRC
    test_buffer[buffer_size - 1] ^= 0x01;
    buffer_index = 0;
    
    bool result = miniCAN_receiveFrame(&frame);
    assert(result == false);
    printf("Single bit CRC error detected\n");
    
    // Test data corruption
    mock_UART_reset();
    miniCAN_sendFrame(&frame);
    
    // Corrupt data byte
    test_buffer[4] ^= 0xFF; // Flip all bits in first data byte
    buffer_index = 0;
    
    result = miniCAN_receiveFrame(&frame);
    assert(result == false);
    printf("Data corruption detected\n");
}

void test_performance_metrics(void) {
    printf("Testing performance metrics...\n");
    
    clock_t start, end;
    double cpu_time_used;
    
    // Test CRC calculation performance
    uint8_t test_data[MAX_DATA_LENGTH + 2];
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i;
    }
    
    start = clock();
    for (int i = 0; i < 10000; i++) {
        crc8(test_data, sizeof(test_data));
    }
    end = clock();
    
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("CRC8 performance: 10000 calculations in %f seconds\n", cpu_time_used);
    
    // Test frame processing performance
    MiniCAN_Frame frame;
    frame.id = 0x55;
    frame.len = MAX_DATA_LENGTH;
    memset(frame.data, 0xAA, MAX_DATA_LENGTH);
    
    start = clock();
    for (int i = 0; i < 1000; i++) {
        mock_UART_reset();
        miniCAN_sendFrame(&frame);
        buffer_index = 0;
        miniCAN_receiveFrame(&frame);
    }
    end = clock();
    
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Frame processing: 1000 send/receive cycles in %f seconds\n", cpu_time_used);
}

void test_protocol_compliance(void) {
    printf("Testing protocol compliance...\n");
    
    // Verify frame format compliance
    MiniCAN_Frame frame;
    frame.id = 0x10;
    frame.len = 3;
    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;
    
    mock_UART_reset();
    miniCAN_sendFrame(&frame);
    
    // Verify transmitted bytes match specification
    assert(test_buffer[0] == START_BYTE);  // START
    assert(test_buffer[1] == 0x10);        // ID
    assert(test_buffer[2] == 0x03);        // LEN
    assert(test_buffer[3] == 0x11);        // DATA[0]
    assert(test_buffer[4] == 0x22);        // DATA[1]
    assert(test_buffer[5] == 0x33);        // DATA[2]
    assert(test_buffer[6] == frame.crc);   // CRC
    assert(buffer_size == 7);              // Total frame size
    
    printf("Frame format compliance verified\n");
    
    // Test CRC covers correct fields (ID + LEN + DATA)
    uint8_t manual_crc_data[] = {0x10, 0x03, 0x11, 0x22, 0x33};
    uint8_t expected_crc = crc8(manual_crc_data, 5);
    assert(frame.crc == expected_crc);
    
    printf("CRC calculation compliance verified\n");
}

int main(void) {
    printf("=== MiniCAN Integration Tests ===\n\n");
    
    // Run unit tests first
    test_crc8_calculation();
    test_frame_send_receive();
    test_frame_validation();
    test_edge_cases();
    test_protocol_integrity();
    
    printf("\n=== Integration Test Suite ===\n");
    
    // Run integration tests
    test_multiple_frames();
    test_stress_random_data();
    test_error_injection();
    test_performance_metrics();
    test_protocol_compliance();
    
    printf("\nAll unit and integration tests passed\n");
    printf("MiniCAN protocol fully verified and compliant.\n");
    
    return 0;
}