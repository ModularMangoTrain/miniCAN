# miniCAN
**miniCAN** is a lightweight, UART-based communication protocol inspired by CAN bus, implemented in pure C for the **ATmega644P** microcontroller.

It demonstrates how to design and transmit structured data frames with error detection, message identifiers, and variable payload lengths, using only a single UART interface.

Really, it's just my own personal project. I needed something that I would use regularly on GPIO pins.

# Features

- Implemented entirely in **C** (no external libraries)
- Works on any **AVR** microcontroller with UART (tested on ATmega644P)
- Simple CAN-like frame structure:
- **CRC-8** checksum for data integrity
- Blocking **send** and **receive** functions
- Easy integration with other UART-based systems
- Ready for expansion into multi-node or failover communication

## Frame Structure

| Field | Bytes | Description |
|--------|--------|-------------|
| **START** | 1 | Frame start marker (`0xAA`) |
| **ID** | 1 | Message ID or node identifier |
| **LEN** | 1 | Number of data bytes (0–8) |
| **DATA** | 0–8 | Payload bytes |
| **CRC** | 1 | CRC-8 checksum over ID, LEN, and DATA |

**Example frame (LEN = 3):**
→ Start(AA), ID(10), Len(03), Data(11 22 33), CRC(4F)

---
## Build Instructions (AVR-GCC)

Compile and flash using `avr-gcc` and `avrdude`:

```bash
avr-gcc -mmcu=atmega644p -Os main.c miniCAN.c -o miniCAN.elf
avr-objcopy -O ihex miniCAN.elf miniCAN.hex
avrdude -p m644p -c usbasp -U flash:w:miniCAN.hex
