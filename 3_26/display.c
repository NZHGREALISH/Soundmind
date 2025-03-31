#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BLACK_COLOR 0x0000

// 开关的内存映射地址
#define SW_BASE 0xFF200040

volatile int pixel_buffer_start;
short int Buffer1[240][512];
short int Buffer2[240][512];

#include "stage1.h" // 背景1 (00)
#undef BACKGROUND_H
#include "stage2.h" // 背景2 (01)
#undef BACKGROUND_H
#include "stage3.h" // 背景3 (10)
#undef BACKGROUND_H
#include "stage4.h" // 背景4 (11)
#undef BACKGROUND_H
#include "start.h" 
#undef BACKGROUND_H
#include "suc.h" 
#undef BACKGROUND_H
#include "over.h"
#undef BACKGROUND_H

// Function declarations
void plot_pixel(int x, int y, short int color);
void clear_screen();
void wait_for_vsync();
void draw_background();
int read_switches();

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
  
  // Main loop: Draws the background every frame based on switches
  while (1) {
    draw_background();  // 根据开关状态绘制背景
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
  }
  return 0;
}

// 读取开关状态的函数
int read_switches() {
  volatile int *switch_ptr = (int *)SW_BASE;
  return *switch_ptr & 0x7; // 只读取最低两位 (SW0和SW1)
}

// 根据开关状态绘制不同的背景
void draw_background() {
  int switch_value = read_switches();
  
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      short int color;
      
      // 根据开关状态选择背景
      switch (switch_value) {
        case 0:  // 00 - 使用背景1
          color = background1[y][x];  // stage1.h
          break;
        case 1:  // 01 - 使用背景2
          color = background2[y][x]; // stage2.h
          break;
        case 2:  // 10 - 使用背景3
          color = background3[y][x]; // stage3.h
          break;
        case 3:  // 11 - 使用背景4
          color = background4[y][x]; // stage4.h
          break;
        case 4:  // 11 - 使用背景4
        color = background5[y][x]; // stage4.h
        break;
        case 5:  // 11 - 使用背景4
        color = background6[y][x]; // stage4.h
        break;
        case 6: 
        color = background7[y][x]; // stage4.h
        break;
        default:
          color = BLACK_COLOR;
          break;
      }
      
      plot_pixel(x, y, color);
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