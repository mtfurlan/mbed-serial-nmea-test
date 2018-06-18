#include "mbed.h"

DigitalOut led(LED1);


Serial pc(USBTX, USBRX);
//Serial uart(p6, p8);

// main() runs in its own thread in the OS
int main() {
    pc.printf("HELLO WORLD\n");
    //uart.printf("HELLO WORLD\n");
    while (true) {
        led = !led;
        //pc.putc(pc.getc() + 1);
        wait(0.5);
        pc.printf("loop hi WORLD\n");
    }
}
