#ifndef F_CPU
#define F_CPU 12000000UL
#endif

#include "miniCAN.h"
#include <util/delay.h>


int main(void) {
    MiniCAN_Frame test_frame;
    uint16_t counter = 0;
    char buffer[6];
    uint16_t num;
    uint8_t i;
    
    UART_init();
    _delay_ms(500); // Allow time for UART to stabilize
    
    UART_sendString("\r\n=== MiniCAN UART Demo ===\r\n");
    
    // Create a test frame
    test_frame.id = 0x10;
    test_frame.len = 4;
    test_frame.data[0] = 0xDE;
    test_frame.data[1] = 0xAD;
    test_frame.data[2] = 0xBE;
    test_frame.data[3] = 0xEF;
    
    while(1) {
        // Update frame data with counter
        test_frame.data[0] = (counter >> 8) & 0xFF;  // High byte
        test_frame.data[1] = counter & 0xFF;         // Low byte
        test_frame.data[2] = 0xAA;
        test_frame.data[3] = 0xBB;
        
        // Send MiniCAN frame
        miniCAN_sendFrame(&test_frame);
        
        // Send human-readable message
        UART_sendString("Sent frame - Counter: ");
        
        // Convert counter to string
        num = counter++;
        i = 0;
        
        if (num == 0) {
            UART_sendByte('0');
        } else {
            while (num > 0) {
                buffer[i++] = (num % 10) + '0';
                num /= 10;
            }
            while (i > 0) {
                UART_sendByte(buffer[--i]);
            }
        }
        
        UART_sendString("\r\n");
        
        _delay_ms(1000);
    }
}