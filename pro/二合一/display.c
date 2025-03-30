#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>    // 需要 sin/cos
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ========== 硬件相关宏 (根据你的硬件实际修改) ==========
#define PIXEL_CTRL_BASE 0xFF203020
#define PS2_BASE        0xFF200100
#define SW_BASE         0xFF200040

// ========== 屏幕、颜色、双缓冲 ==========
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BLACK_COLOR     0x0000

volatile int pixel_buffer_start;
short int Buffer1[SCREEN_HEIGHT][512];
short int Buffer2[SCREEN_HEIGHT][512];

// ========== 自定义一些颜色（RGB565 格式） ==========
// 这里给橙色取一个近似值 (R=255,G=165,B=0 => #FFA500)
// 在 RGB565 里可以用 0xFD00 或 0xFC80 等都行，这里选 0xFD00
#define ORANGE 0xFD00
#define BLACK  0x0000

// ========== 槽位相关 ==========
// 共有 5 个槽位，每个槽位的图案大小为 25×25 像素
// 使用自定义坐标，不再用固定间距
#define NUM_KEY_SLOTS   5

// 定义5个槽位的坐标
typedef struct {
    int x;
    int y;
} Coordinate;

Coordinate slot_positions[NUM_KEY_SLOTS] = {
  {27, 158},   // 槽位1
  {86, 159},   // 槽位2
  {146, 160},  // 槽位3
  {205, 160},  // 槽位4
  {267, 158}   // 槽位5
};


// 圆的直径 25，字母的画布也是围绕圆心
// 圆心在 (x+12, y+12)，半径=12
char letter_slots[NUM_KEY_SLOTS] = {0};  // 存放哪几个字母
int current_slot = 0;                   // 已用槽位数

// ========== 前置声明 ==========
void plot_pixel(int x, int y, short color);
void wait_for_vsync();
void clear_screen();

int  read_switches();
int  get_ps2_code();
void draw_background();
void draw_letter_slots();

// ====== 画橙色圆 ====== //
void draw_orange_circle(int x, int y);

// ====== 画加粗线(直线) ====== //
void draw_thick_vertical_line(int x, int y_start, int length, int thickness, short color);
void draw_thick_horizontal_line(int y, int x_start, int length, int thickness, short color);

// ====== 画加粗弧线 ====== //
void draw_thick_arc(int cx, int cy, int outer_r, int thickness,
                    float start_deg, float end_deg, short color);

// ====== 5个字母: 先画圆, 再组合线/弧 ====== //
void draw_C(int x, int y);
void draw_D(int x, int y);
void draw_E(int x, int y);
void draw_F(int x, int y);
void draw_G(int x, int y);

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
// ------------------------------------------
// 主函数
// ------------------------------------------
int main(void) {
    // 配置像素缓冲区指针
    volatile int* pixel_ctrl_ptr = (int*)PIXEL_CTRL_BASE;
    // 配置 PS/2
    volatile int* ps2_ptr = (int*)PS2_BASE;
    *ps2_ptr = 0xFF;  // 复位 PS/2

    // 初始化双缓冲
    *(pixel_ctrl_ptr + 1) = (int)(&Buffer1);
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int)(&Buffer2);
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();

    while (1) {
        // 1) 读取键盘扫描码
        int code = get_ps2_code();
        if (code != -1) {
            // 如果是 0xF0 (break code)，再读一次并丢弃
            if (code == 0xF0) {
                int discard;
                do {
                    discard = get_ps2_code();
                } while (discard == -1);
            } else {
                // 如果是 make code
                if (current_slot < NUM_KEY_SLOTS) {
                    // C=0x21, D=0x23, E=0x24, F=0x2B, G=0x34
                    switch (code) {
                        case 0x21: letter_slots[current_slot++] = 'C'; break;
                        case 0x23: letter_slots[current_slot++] = 'D'; break;
                        case 0x24: letter_slots[current_slot++] = 'E'; break;
                        case 0x2B: letter_slots[current_slot++] = 'F'; break;
                        case 0x34: letter_slots[current_slot++] = 'G'; break;
                        default: break;
                    }
                }
            }
        }

        // 2) 绘制背景 + 槽位
        draw_background();
        draw_letter_slots();

        // 3) vsync + 切换缓冲
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
    return 0;
}

// ------------------------------------------
// 读取 PS/2 接口，如果无数据返回 -1
// ------------------------------------------
int get_ps2_code() {
    volatile int* ps2_ptr = (int*)PS2_BASE;
    int data = *ps2_ptr;
    if (data & 0x8000) { // RVALID=1
        return data & 0xFF;
    }
    return -1;
}

int read_switches() {
    volatile int* sw_ptr = (int*)SW_BASE;
    return (*sw_ptr) & 0x7;  // 只取低3位
}

// ------------------------------------------
// 根据开关状态选择背景
// ------------------------------------------
void draw_background() {
    int sw = read_switches();
    short color;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            switch (sw) {
                case 0: color = background1[y][x]; break;
                case 1: color = background2[y][x]; break;
                case 2: color = background3[y][x]; break;
                case 3: color = background4[y][x]; break;
                case 4: color = background5[y][x]; break;
                case 5: color = background6[y][x]; break;
                case 6: color = background7[y][x]; break;
                default: color = BLACK_COLOR;       break;
            }
            plot_pixel(x, y, color);
        }
    }
}

// ------------------------------------------
// 槽位里画字母 - 现在使用自定义坐标
// ------------------------------------------
void draw_letter_slots() {
    for (int i = 0; i < NUM_KEY_SLOTS; i++) {
        // 使用自定义坐标
        int sx = slot_positions[i].x;
        int sy = slot_positions[i].y;
        
        switch (letter_slots[i]) {
            case 'C': draw_C(sx, sy); break;
            case 'D': draw_D(sx, sy); break;
            case 'E': draw_E(sx, sy); break;
            case 'F': draw_F(sx, sy); break;
            case 'G': draw_G(sx, sy); break;
            default: break;
        }
    }
}

// ------------------------------------------
// 画一个 25×25 的橙色实心圆
// 圆心 (cx,cy) = (x+12,y+12), 半径=12
// ------------------------------------------
void draw_orange_circle(int x, int y) {
    int cx = x + 13;
    int cy = y + 13;
    int r  = 13;
    int r_sq = r*r;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r_sq) {
                plot_pixel(cx + dx, cy + dy, ORANGE);
            }
        }
    }
}

// ------------------------------------------
// 粗直线：垂直
// 以 (x,y_start) 为上端，长度 length，线宽 thickness
// ------------------------------------------
void draw_thick_vertical_line(int x, int y_start, int length, int thickness, short color) {
    // 线的中心是 x; 我们左右各扩展 thickness/2
    int half_t = thickness / 2;
    for (int yy = 0; yy < length; yy++) {
        for (int tx = -half_t; tx <= half_t; tx++) {
            plot_pixel(x + tx, y_start + yy, color);
        }
    }
}

// ------------------------------------------
// 粗直线：水平
// 以 (x_start,y) 为左端，长度 length，线宽 thickness
// ------------------------------------------
void draw_thick_horizontal_line(int y, int x_start, int length, int thickness, short color) {
    int half_t = thickness / 2;
    for (int xx = 0; xx < length; xx++) {
        for (int ty = -half_t; ty <= half_t; ty++) {
            plot_pixel(x_start + xx, y + ty, color);
        }
    }
}

// ------------------------------------------
// 画一段加粗圆弧：start_deg ~ end_deg (单位:度)
// (cx,cy): 圆心, outer_r:外半径, thickness:线宽
// ------------------------------------------
void draw_thick_arc(int cx, int cy, int outer_r, int thickness,
                    float start_deg, float end_deg, short color) {
    if (end_deg < start_deg) {
        end_deg += 360.f;
    }
    float step = 1.0f;  // 步进度数，越小弧线越平滑
    for (float a = start_deg; a <= end_deg; a += step) {
        float rad = a * (float)(M_PI / 180.0f);
        // 让线条有一定厚度
        for (int t = 0; t < thickness; t++) {
            int rr = outer_r - t;
            int px = (int)lroundf(cx + rr * cosf(rad));
            int py = (int)lroundf(cy + rr * sinf(rad));
            plot_pixel(px, py, color);
        }
    }
}

// ------------------------------------------
// C: 先画橙色圆，再用一段弧线画"C"形
// ------------------------------------------
void draw_C(int x, int y) {
    draw_orange_circle(x, y);

    int cx = x + 12; 
    int cy = y + 12;
    // 粗弧线: radius=8, thickness=3, 45° ~ 315°
    draw_thick_arc(cx, cy, 8, 3, 45, 315, BLACK);
}

// ------------------------------------------
// D: 左侧竖线 + 右侧大弧
// ------------------------------------------
void draw_D(int x, int y) {
    draw_orange_circle(x, y);
    int cx = x + 12;
    int cy = y + 12;

    // 左竖线：大约从上边到下边  (cx-8, cy-8) -> 长度16
    // 让线宽=3
    draw_thick_vertical_line(cx-4, cy-9, 18, 3, BLACK);

    // 右侧弧: 90° ~ 270°, 圆心向右偏一点 (cx+5), radius=9
    draw_thick_arc(cx-2, cy, 10, 3, -90, 90, BLACK);
}

// ------------------------------------------
// E: 左竖线 + 上/中/下 3 条水平线
// ------------------------------------------
void draw_E(int x, int y) {
    draw_orange_circle(x, y);
    int cx = x + 12;
    int cy = y + 12;

    // 左竖线(同D)
    draw_thick_vertical_line(cx-5, cy-8, 16, 3, BLACK);

    // 上横 (y=cy-8, x从cx-8开始,长度=9,线宽=3)
    draw_thick_horizontal_line(cy-8, cx-5, 10, 3, BLACK);
    // 中横 (y=cy,    x从cx-8开始,长度=9,线宽=3)
    draw_thick_horizontal_line(cy,    cx-5, 10, 3, BLACK);
    // 下横 (y=cy+8,  x从cx-8开始,长度=9,线宽=3)
    draw_thick_horizontal_line(cy+8,  cx-5, 10, 3, BLACK);
}

// ------------------------------------------
// F: 左竖线 + 上/中 2 条水平线 (没有最下面)
// ------------------------------------------
void draw_F(int x, int y) {
    draw_orange_circle(x, y);
    int cx = x + 12;
    int cy = y + 12;

    // 左竖线
    draw_thick_vertical_line(cx-5, cy-6, 16, 3, BLACK);

    // 上横
    draw_thick_horizontal_line(cy-6, cx-5, 9, 3, BLACK);
    // 中横
    draw_thick_horizontal_line(cy+2, cx-5, 9, 3, BLACK);
}

// ------------------------------------------
// G: 类似 C，但在右下多一段弧
// ------------------------------------------
void draw_G(int x, int y) {
    draw_orange_circle(x, y);
    int cx = x + 12;
    int cy = y + 12;

    // 外围"C"
    draw_thick_arc(cx, cy, 8, 3, 45, 315, BLACK);

    // 在右下多加一个小弧 (cx+2, cy+3), radius=4, angle=280~360
    draw_thick_vertical_line(cx+4, cy, 6, 3, BLACK);
}

// ------------------------------------------
// 像素与同步
// ------------------------------------------
void plot_pixel(int x, int y, short color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) 
        return; // 防越界
    volatile short* addr = (volatile short*)(pixel_buffer_start + (y << 10) + (x << 1));
    *addr = color;
}

void clear_screen() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            plot_pixel(x, y, BLACK_COLOR);
        }
    }
}

void wait_for_vsync() {
    volatile int* pixel_ctrl_ptr = (int*)PIXEL_CTRL_BASE;
    *pixel_ctrl_ptr = 1; // start sync
    while ((*(pixel_ctrl_ptr + 3)) & 1) {
        // 等待 busy 位清零
    }
}