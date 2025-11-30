# miniCAN

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C Standard](https://img.shields.io/badge/C-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Platform](https://img.shields.io/badge/Platform-AVR-green.svg)](https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/8-bit-mcus/avr-mcus)

**miniCAN** is a lightweight, UART-based communication protocol inspired by CAN bus architecture, designed for embedded systems requiring reliable point-to-point or broadcast communication.

## Overview

This implementation provides a CAN-like messaging protocol over UART, featuring structured data frames with error detection, message identifiers, and variable payload lengths. The protocol is optimized for resource-constrained microcontrollers and offers a simple alternative to complex bus protocols.

### Key Benefits
- **Lightweight**: Minimal memory footprint and CPU overhead
- **Reliable**: CRC-8 error detection ensures data integrity
- **Flexible**: Variable payload length (0-8 bytes) with message IDs
- **Portable**: Pure C implementation compatible with any AVR microcontroller
- **Simple**: Easy integration with existing UART-based systems

## Features

### Core Functionality
- **Pure C Implementation** - No external dependencies
- **CRC-8 Error Detection** - Polynomial 0x07 for data integrity
- **Variable Frame Length** - 0-8 byte payloads
- **Message Identification** - 8-bit ID field for routing/filtering
- **Frame Synchronization** - Start byte delimiter (0xAA)
- **Blocking I/O Operations** - Simple send/receive functions

### Hardware Support
- **Primary Target**: ATmega644P (tested)
- **Compatible**: Any AVR microcontroller with UART
- **Clock Frequency**: Configurable (default: 12MHz)
- **Baud Rate**: Configurable (default: 9600)

## Protocol Specification

### Frame Structure

| Field | Size | Range | Description |
|-------|------|-------|-------------|
| **START** | 1 byte | `0xAA` | Frame synchronization marker |
| **ID** | 1 byte | `0x00-0xFF` | Message identifier |
| **LEN** | 1 byte | `0x00-0x08` | Payload length in bytes |
| **DATA** | 0-8 bytes | Variable | Message payload |
| **CRC** | 1 byte | `0x00-0xFF` | CRC-8 checksum |

**Total Frame Size**: 4-12 bytes (depending on payload)

### Frame Example
```
Message: ID=0x10, Data=[0x11, 0x22, 0x33]
Transmission: [0xAA][0x10][0x03][0x11][0x22][0x33][0x4F]
                ↑     ↑     ↑     ↑           ↑
              START   ID   LEN   DATA        CRC
```

### CRC Calculation
- **Algorithm**: CRC-8
- **Polynomial**: 0x07 (x³ + x² + x + 1)
- **Initial Value**: 0x00
- **Input Data**: ID + LEN + DATA fields
- **Error Detection**: Single-bit errors and most multi-bit errors

## API Reference

### Data Structures
```c
typedef struct {
    uint8_t id;                    // Message identifier
    uint8_t len;                   // Payload length (0-8)
    uint8_t data[MAX_DATA_LENGTH]; // Payload buffer
    uint8_t crc;                   // Calculated checksum
} MiniCAN_Frame;
```

### Core Functions
```c
// Initialize UART hardware
void UART_init(void);

// Send a complete frame
void miniCAN_sendFrame(MiniCAN_Frame *frame);

// Receive and validate a frame
bool miniCAN_receiveFrame(MiniCAN_Frame *frame);

// Calculate CRC-8 checksum
uint8_t crc8(const uint8_t *data, uint8_t len);
```

### Usage Example
```c
#include "miniCAN.h"

int main(void) {
    MiniCAN_Frame frame;
    
    // Initialize UART
    UART_init();
    
    // Prepare frame
    frame.id = 0x10;
    frame.len = 3;
    frame.data[0] = 0xDE;
    frame.data[1] = 0xAD;
    frame.data[2] = 0xBE;
    
    // Send frame (CRC calculated automatically)
    miniCAN_sendFrame(&frame);
    
    // Receive frame
    if (miniCAN_receiveFrame(&frame)) {
        // Frame received successfully
        // Process frame.id, frame.len, frame.data
    }
    
    return 0;
}
```

## Build Instructions

### Prerequisites
- **avr-gcc** - AVR cross-compiler
- **avrdude** - AVR programming utility
- **AVR programmer** (USBasp, Arduino as ISP, etc.)

### Compilation
```bash
# Compile source files
avr-gcc -mmcu=atmega644p -DF_CPU=12000000UL -Wall -Os demo.c miniCAN.c -o minican.elf

# Generate hex file
avr-objcopy -O ihex minican.elf minican.hex

# Program microcontroller
avrdude -c usbasp -P usb -p m644p -U flash:w:minican.hex
```

### Configuration Options
```c
// In miniCAN.h or compile flags
#define F_CPU 12000000UL    // Clock frequency
#define START_BYTE 0xAA     // Frame delimiter
#define MAX_DATA_LENGTH 8   // Maximum payload size
```

### Baud Rate Configuration
For different baud rates, modify `UBRR0L` in `UART_init()`:
```c
// Formula: UBRR = (F_CPU / (16 * BAUD)) - 1
// 9600 baud @ 12MHz: UBRR0L = 77
// 19200 baud @ 12MHz: UBRR0L = 38
```

## Performance Characteristics

| Metric | Value |
|--------|-------|
| **Code Size** | ~1.2KB (optimized) |
| **RAM Usage** | ~20 bytes (frame buffer) |
| **Throughput** | ~960 bytes/sec @ 9600 baud |
| **Latency** | ~1ms per frame (8-byte payload) |
| **Error Detection** | 99.6% (CRC-8) |

## Limitations & Considerations

### Current Limitations
- **Blocking I/O**: Functions block until completion
- **No Timeout**: Receiver can hang on incomplete frames
- **Single Error Recovery**: No automatic frame resynchronization
- **No Flow Control**: No built-in congestion management

### Recommended Improvements
- Add timeout mechanisms for robust operation
- Implement non-blocking I/O with state machines
- Add frame filtering and routing capabilities
- Include error statistics and diagnostics

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by the CAN bus protocol specification
- Built for the AVR microcontroller ecosystem
- Designed for educational and practical embedded applications
