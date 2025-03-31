#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BLACK_COLOR 0x0000

volatile int pixel_buffer_start;
short int Buffer1[240][512];
short int Buffer2[240][512];

#include "stage1.h"
#include "stage2.h"
#include "stage3.h"
#include "stage4.h"


// Function declarations
void plot_pixel(int x, int y, short int color);
void clear_screen();
void wait_for_vsync();
void draw_background();

int main(void) {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

  // Initialize the buffer
  *(pixel_ctrl_ptr + 1) = (int)&Buffer1;
  wait_for_vsync();
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();

  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);
  clear_screen();


  // Main loop: Draws the background every frame
  while (1) {
    draw_background();  // Paint the background
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
  }
  return 0;
}

// Function to draw the background from the array
void draw_background() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
        plot_pixel(x, y, background[y][x]);
    }
  }
}

// Function to plot a single pixel
void plot_pixel(int x, int y, short int color) {
  volatile short int *one_pixel_address =
      (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
  *one_pixel_address = color;
}

// Synchronize with vertical sync
void wait_for_vsync() {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  *pixel_ctrl_ptr = 1;
  while (*(pixel_ctrl_ptr + 3) & 1);
}

// Clear screen with black (optional, used in setup)
void clear_screen() {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      plot_pixel(x, y, BLACK_COLOR);
    }
  }
}
