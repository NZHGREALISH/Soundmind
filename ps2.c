#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// PS/2 keyboard registers
#define PS2_DATA       (*(volatile uint32_t*) 0xFF200100)
#define PS2_CONTROL    (*(volatile uint32_t*) 0xFF200104)

// Bit positions in PS2_DATA register
#define PS2_RVALID     0x8000      // Bit 15 (RVALID)
#define PS2_DATA_MASK  0xFF        // Bits 0-7 (Data)
#define PS2_RAVAIL_MASK 0xFFFF0000 // Bits 16-31 (RAVAIL)

// 7-segment display registers
#define HEX3_HEX0      (*(volatile uint32_t*) 0xFF200020) // Controls HEX3 to HEX0
#define HEX5_HEX4      (*(volatile uint32_t*) 0xFF200030) // Controls HEX5 to HEX4

// LED registers
#define LEDR           (*(volatile uint32_t*) 0xFF200000) // Red LEDs

// PS/2 Scan codes for arrow keys
#define UP_ARROW_CODE      0x75    // E0 75 for extended key
#define DOWN_ARROW_CODE    0x72    // E0 72 for extended key
#define EXTENDED_CODE      0xE0    // Prefix for extended keys

// Function to set a specific hex display (0-5)
void set_hex_display(int display_num, uint8_t value) {
    static const uint8_t seven_seg_digits[16] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111, // 9
        0b01110111, // A
        0b01111100, // b
        0b00111001, // C
        0b01011110, // d
        0b01111001, // E
        0b01110001  // F
    };
    
    uint8_t pattern = (value < 16) ? seven_seg_digits[value] : 0;
    
    if (display_num >= 0 && display_num <= 3) {
        // Update HEX3-HEX0 (address 0xFF200020)
        uint32_t current = HEX3_HEX0;
        // Clear the bits for the specified display
        current &= ~(0x7F << (display_num * 8));
        // Set the new pattern
        current |= (pattern << (display_num * 8));
        HEX3_HEX0 = current;
    } else if (display_num >= 4 && display_num <= 5) {
        // Update HEX5-HEX4 (address 0xFF200030)
        uint32_t current = HEX5_HEX4;
        // Adjust display_num for HEX5-HEX4 register (0 for HEX4, 1 for HEX5)
        int adjusted_num = display_num - 4;
        // Clear the bits for the specified display
        current &= ~(0x7F << (adjusted_num * 8));
        // Set the new pattern
        current |= (pattern << (adjusted_num * 8));
        HEX5_HEX4 = current;
    }
}

// Clear all 7-segment displays
void clear_all_displays() {
    HEX3_HEX0 = 0;
    HEX5_HEX4 = 0;
}

// Set LED based on key press
void set_led(int led_num, bool state) {
    if (state) {
        // Turn on the specified LED
        LEDR |= (1 << led_num);
    } else {
        // Turn off the specified LED
        LEDR &= ~(1 << led_num);
    }
}

// Handle PS/2 keyboard scan code
void handle_key(uint8_t scan_code) {
    static bool extended_key = false;
    static bool key_release = false;
    
    // Display scan code only on the last two 7-segment displays (HEX5 and HEX4)
    set_hex_display(4, scan_code & 0xF);           // Lower nibble on HEX4
    set_hex_display(5, (scan_code >> 4) & 0xF);    // Upper nibble on HEX5
    
    // Check for extended key prefix (E0)
    if (scan_code == EXTENDED_CODE) {
        extended_key = true;
        return;
    }
    
    // Check for key release prefix (F0)
    if (scan_code == 0xF0) {
        key_release = true;
        return;
    }
    
    // Handle extended key arrow presses
    if (extended_key) {
        if (!key_release) {
            if (scan_code == UP_ARROW_CODE) {
                printf("UP ARROW pressed\n");
                set_led(0, true);  // Turn on LEDR0
                set_led(1, false); // Turn off LEDR1
            }
            else if (scan_code == DOWN_ARROW_CODE) {
                printf("DOWN ARROW pressed\n");
                set_led(1, true);  // Turn on LEDR1
                set_led(0, false); // Turn off LEDR0
            }
        } else {
            // Key released - not turning off LEDs as per requirements
            // Just reset flags
            printf("Arrow key released: %02X\n", scan_code);
        }
        
        // Reset flags
        extended_key = false;
        key_release = false;
    }
}

// Read PS/2 data
void read_ps2() {
    printf("Reading PS/2 keyboard...\n");
    printf("Press UP arrow key to light LEDR0\n");
    printf("Press DOWN arrow key to light LEDR1\n");
    
    // Turn off all LEDs initially
    LEDR = 0;
    
    // Display "rd" on the last two 7-segment displays
    set_hex_display(4, 0xB); // d
    set_hex_display(5, 0xA); // r
    
    while (1) {
        // Check if data is available by testing RVALID
        if (PS2_DATA & PS2_RVALID) {
            // Data is available, read it
            uint32_t ps2_data = PS2_DATA;
            uint8_t scan_code = ps2_data & PS2_DATA_MASK;
            
            // Output debug info to console
            printf("PS/2 Data: 0x%02X\n", scan_code);
            
            // Process the key
            handle_key(scan_code);
        }
        
        // Small delay to avoid tight polling loop
        for (volatile int i = 0; i < 1000; i++);
    }
}

// Enable PS/2 port interrupt
void enable_ps2_interrupt() {
    // Set RE (bit 0) to enable interrupts when RAVAIL > 0
    PS2_CONTROL |= 0x01;
    
    // Clear any pending interrupts by reading the PS2_DATA register
    volatile uint32_t dummy = PS2_DATA;
    
    printf("PS/2 Interrupts Enabled\n");
}

int main() {
    printf("PS/2 Keyboard Arrow Keys to LEDs\n");
    
    // Initialize the displays
    clear_all_displays();
    
    // Make sure the PS/2 FIFO is empty before we start
    volatile uint32_t dummy;
    while (PS2_DATA & PS2_RVALID) {
        dummy = PS2_DATA;
    }
    
    // Enable PS/2 interrupt
    enable_ps2_interrupt();
    
    // Read keyboard data and control LEDs
    read_ps2();
    
    return 0;
}