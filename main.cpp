#include "mbed.h"

DigitalOut led(LED1);
DigitalOut testPin(P1_11);

//pin names
//mbed-os/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_NRF52/TARGET_MCU_NRF52840/TARGET_NRF52840_DK/PinNames.h

//Serial pc(USBTX, USBRX);
Serial pc(P0_6, P0_8);

Serial uart(P0_26, P0_27);

// main() runs in its own thread in the OS
int main() {
    pc.printf("HELLO WORLD\n");
    uart.printf("HELLO WORLD\n");
    while (true) {
        led = !led;
        testPin = !testPin;
        //pc.putc(pc.getc() + 1);
        wait(0.5);
        pc.printf("loop hi WORLD\n");
        uart.printf("loop hi WORLD\n");
    }
}
