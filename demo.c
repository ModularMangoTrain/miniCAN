#include "miniCAN.h"
#include <avr/io.h>
#include <util/delay.h>

int main(void){
    UART_init();

    MiniCAN_Frame frame;
    frame.id = 0x10;
    frame.len = 3;
    frame.data[0] = 0x11;
    frame.data[1] = 0x22;
    frame.data[2] = 0x33;

    frame.crc = crc8(&frame.id, 2 + frame.len);

    for(;;){
        miniCAN_sendFrame(&frame);
        _delay_ms(1000);
    }

}
