#include "system.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define TIMER_BASE       TIMER_0_BASE
#define LED_BASE         LEDS_BASE
#define KEYS_BASE_ADDR   KEYS_BASE
#define KEY1_MASK        0x02       // KEY1 is bit 1
#define LEDR7_MASK       0x80       // LEDR7 = bit 7

volatile unsigned int t1 = 0, t2 = 0, duration = 0;
bool first_press = true;
bool in_repeat_mode = false;
unsigned int start_repeat = 0;
int cycle_count = 0;

// Timer Read Function
unsigned int read_timer() {
    IOWR_ALTERA_AVALON_TIMER_SNAPL(TIMER_BASE, 0); // Latch timer snapshot
    unsigned int low  = IORD_ALTERA_AVALON_TIMER_SNAPL(TIMER_BASE);
    unsigned int high = IORD_ALTERA_AVALON_TIMER_SNAPH(TIMER_BASE);
    return (high << 16) | low;
}

int main() {
    // Turn off all LEDs
    IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0x00);

    // Configure timer in continuous mode (32-bit)
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x04); // Stop
    IOWR_ALTERA_AVALON_TIMER_PERIODL(TIMER_BASE, 0xFFFF);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(TIMER_BASE, 0xFFFF);
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_BASE, 0x07); // Start

    printf("Period Measurement and Repeat Mode Started...\n");

    while (1) {
        int keys = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE_ADDR);

        // Check for KEY1 press (active-low)
        if (!(keys & KEY1_MASK)) {
            usleep(50000); // Debounce

            if (first_press) {
                t1 = read_timer();
                printf("First KEY1 press - Start Time: %u ticks\n", t1);
                first_press = false;
            } else {
                t2 = read_timer();
                if (t2 >= t1)
                    duration = t2 - t1;
                else
                    duration = (0xFFFFFFFF - t1) + t2;

                printf("Second KEY1 press - End Time: %u ticks\n", t2);
                printf("Measured Duration: %u ticks (approx %.3f seconds)\n", duration, duration / 1e6);

                start_repeat = read_timer();
                in_repeat_mode = true;
                first_press = true;
            }

            // Wait for KEY1 release
            while (!(IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE_ADDR) & KEY1_MASK));
        }

        // Auto-repeat logic
        if (in_repeat_mode) {
            unsigned int now = read_timer();
            unsigned int elapsed = (now >= start_repeat)
                                   ? (now - start_repeat)
                                   : ((0xFFFFFFFF - start_repeat) + now);

            if (elapsed >= 5000000) { // 5 seconds
                cycle_count++;

                printf("Cycle repeated %d times every 5 seconds.\n", cycle_count);

                // Blink LEDR7
                IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, LEDR7_MASK);
                usleep(500000); // LED ON 0.5 sec
                IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0x00);

                start_repeat = read_timer(); // Reset timer
            }
        }
    }

    return 0;
}
