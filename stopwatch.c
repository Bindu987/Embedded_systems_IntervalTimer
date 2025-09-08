#include <stdio.h>
#include <unistd.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_timestamp.h"

// Active-low encoding for 7-segment display
int digit_encoding[10] = {
    0xC0, // 0
    0xF9, // 1
    0xA4, // 2
    0xB0, // 3
    0x99, // 4
    0x92, // 5
    0x82, // 6
    0xF8, // 7
    0x80, // 8
    0x90  // 9
};

// Function to show integer value (max 6 digits) on HEX0–HEX5
void display_on_hex(int value) {
    if (value > 999999) value = 999999;

    for (int i = 0; i < 6; i++) {
        int digit = value % 10;
        value /= 10;

        int encoded = digit_encoding[digit];

        switch (i) {
            case 0: IOWR_ALTERA_AVALON_PIO_DATA(HEX0_BASE, encoded); break;
            case 1: IOWR_ALTERA_AVALON_PIO_DATA(HEX1_BASE, encoded); break;
            case 2: IOWR_ALTERA_AVALON_PIO_DATA(HEX2_BASE, encoded); break;
            case 3: IOWR_ALTERA_AVALON_PIO_DATA(HEX3_BASE, encoded); break;
            case 4: IOWR_ALTERA_AVALON_PIO_DATA(HEX_4_BASE, encoded); break;
            case 5: IOWR_ALTERA_AVALON_PIO_DATA(HEX_5_BASE, encoded); break;
        }
    }
}

int main() {
    printf("== Stopwatch Mode: Hold KEY2 to Measure Time ==\n");

    if (alt_timestamp_start() < 0) {
        printf("Error: timestamp timer failed to start.\n");
        return 1;
    }

    int prev_keys = 0xF;
    alt_u64 start = 0, stop = 0, ticks = 0;
    int duration_us = 0;

    while (1) {
        int cur_keys = IORD_ALTERA_AVALON_PIO_DATA(KEYS_BASE);
        int key2_now = !(cur_keys & 0x04);
        int key2_prev = !(prev_keys & 0x04);

        if (key2_now && !key2_prev) {
            start = alt_timestamp();
            printf("KEY2 pressed.\n");
            printf("Start Condition: Timer started at tick = %lu\n", (unsigned long)start);
        }

        if (!key2_now && key2_prev) {
            stop = alt_timestamp();
            printf("KEY2 released.\n");
            printf("Stop Condition: Timer stopped at tick = %lu\n", (unsigned long)stop);

            ticks = (stop > start) ? (stop - start) : (start - stop);
            duration_us = (int)((double)ticks * 1e6 / alt_timestamp_freq());

            printf("Time Calculation: Duration = %d microseconds\n", duration_us);

            IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, duration_us & 0x3FF);
            display_on_hex(duration_us);
        }

        prev_keys = cur_keys;
        usleep(5000); // debounce
    }

    return 0;
}
