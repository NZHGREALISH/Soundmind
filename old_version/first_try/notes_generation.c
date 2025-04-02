#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define peripheral base addresses
#define AUDIO_BASE  0xFF203040  // Audio core base address
#define TIMER_BASE  0xFF202000  // Hardware Timer base address

// Define audio FIFO structure
struct audio_t {
    volatile unsigned int control;
    volatile unsigned char rarc;
    volatile unsigned char ralc;
    volatile unsigned char wsrc;
    volatile unsigned char wslc;
    volatile unsigned int ldata;
    volatile unsigned int rdata;
};

// Timer register structure
struct timer_t {
    volatile unsigned int status;
    volatile unsigned int control;
    volatile unsigned int period_low;
    volatile unsigned int period_high;
};

// Pointers to hardware peripherals
struct audio_t *const audiop = (struct audio_t *) AUDIO_BASE;
struct timer_t *const timerp = (struct timer_t *) TIMER_BASE;

// Define C major scale frequencies (Hz)
const int frequencies[] = {261, 293, 329, 349, 392, 440, 493, 523}; // C4 to C5

#define MAX_NOTES 10   // Maximum number of notes in sequence

// Function to generate a random sequence of musical notes
void generate_sequence(int *sequence, int length) {
    for (int i = 0; i < length; i++) {
        sequence[i] = rand() % 8;  // Pick a random note (0-7)
    }
}

// Function to start the hardware timer for a given duration (in microseconds)
void start_timer(int duration_us) {
    int timer_count = duration_us * 8;  // Convert time to 8kHz clock cycles

    timerp->period_low = (timer_count & 0xFFFF);
    timerp->period_high = (timer_count >> 16);
    timerp->control = 0b0110;  // Start timer with auto-reload mode
}

// Function to wait for the timer to expire
void wait_for_timer() {
    while ((timerp->status & 1) == 0);  // Poll the timer status register
    timerp->status = 1;  // Reset the timer interrupt flag
}

// Function to play a square wave at a given frequency
void play_note(int note_index, int duration_ms) {
    int freq = frequencies[note_index];  
    int period_in_samples = 8000 / freq;  // Compute period in 8kHz sample rate
    int half_period_count = 0;
    int current_amplitude = 0x7FFFFF;  // Max amplitude

    int total_samples = (duration_ms * 8000) / 1000;  // Convert ms to samples

    for (int i = 0; i < total_samples; i++) {
        if ((audiop->wsrc > 0) && (audiop->wslc > 0)) {  // Check FIFO space
            audiop->ldata = current_amplitude;
            audiop->rdata = current_amplitude;

            half_period_count++;
            if (half_period_count >= (period_in_samples >> 1)) {  // Toggle amplitude
                current_amplitude = (current_amplitude == 0x7FFFFF) ? 0 : 0x7FFFFF;
                half_period_count = 0;
            }
        }
    }
}

// Function to play a sequence with a timer-based delay
void play_sequence(int *sequence, int length, int delay_ms) {
    for (int i = 0; i < length; i++) {
        play_note(sequence[i], 500);  // Play each note for 500ms
        start_timer(delay_ms * 1000);  // Start the timer for delay
        wait_for_timer();  // Wait for timer to expire
    }
}

int main() {
    srand(time(NULL));  // Seed random number generator

    int sequence[MAX_NOTES];
    int current_length = 4;   // Start with 4 notes
    int current_delay = 500;  // Initial delay (ms)

    generate_sequence(sequence, current_length);
    play_sequence(sequence, current_length, current_delay);

    return 0;
}
