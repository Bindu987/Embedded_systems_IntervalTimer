#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_timestamp.h"
#include <stdio.h>

// Define base addresses (adjust names based on your system.h)
#define KEY_BASE       KEYS_BASE
#define LED_BASE       LEDS_BASE
#define KEY3_MASK      0x08        // KEY3 is bit 3 (active low)
#define LEDR9_MASK     0x200       // LEDR9 is bit 9

int main() {
    printf("Timeout Detection Mode Started...\n");

    // Initialize timestamp timer
    if (alt_timestamp_start() < 0) {
        printf("Error starting timestamp timer.\n");
        return 1;
    }

    // 5-second timeout in ticks
    alt_u64 timeout_ticks = alt_timestamp_freq() * 5;
    alt_u64 last_activity = alt_timestamp();
    int led_state = 0;

    while (1) {
        int key_val = ~IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) & KEY3_MASK;

        if (key_val) {
            // KEY3 pressed
            last_activity = alt_timestamp();
            led_state = 0;
            IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, 0x000);  // Turn off all LEDs
            printf("KEY3 pressed: Resetting timeout.\n");

            // Debounce: wait until KEY3 is released
            while (~IORD_ALTERA_AVALON_PIO_DATA(KEY_BASE) & KEY3_MASK);
        }

        // Check for timeout
        alt_u64 now = alt_timestamp();
        if ((now - last_activity) >= timeout_ticks && led_state == 0) {
            IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, LEDR9_MASK);  // Turn on LEDR9
            printf("Timeout detected! LEDR9 turned on.\n");
            led_state = 1;
        }
    }

    return 0;
}
