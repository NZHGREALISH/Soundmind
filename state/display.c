#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define PIXEL_CTRL_BASE 0xFF203020
#define PS2_BASE        0xFF200100
#define SW_BASE         0xFF200040
#define AUDIO_BASE      0xFF203040   
#define TIMER_BASE      0xFF202000   
#define HEX3_HEX0_BASE  0xFF200020  
#define HEX5_HEX4_BASE  0xFF200030
#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240
#define BLACK_COLOR     0x0000
#define PS2_KEY_SPACE 0x29      // Space key
#define PS2_KEY_UP    0x75      // Up arrow 
#define PS2_KEY_DOWN  0x72      // Down arrow 
#define PS2_KEY_C     0x21      // C 
#define PS2_KEY_D     0x23      // D 
#define PS2_KEY_E     0x24      // E 
#define PS2_KEY_F     0x2B      // F 
#define PS2_KEY_G     0x34      // G
#define ORANGE 0xFD00
#define BLACK  0x0000
#define PINK  0xFD78
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
#include "study.h"
#undef BACKGROUND_H
#include "c.h"
#include "d.h"
#include "e.h"
#include "f.h"
#include "g.h"


int difficulty = 0; 

typedef enum {
    GAME_START,    // 开始界面
    GAME_STUDY,
    GAME_SETUP,    // 设置界面（选择难度）
    GAME_PLAYING,  // 游戏中（难度1-4）
    GAME_SUCCESS,  // 成功界面
    GAME_OVER      // 失败界面
} GameState;

// ========== 音频和波形表 ==========
#define TABLE_SIZE 1028          
#define MAX_AMPLITUDE 0x7FFFFF  


void play_random_note();
volatile int pixel_buffer_start;
short int Buffer1[SCREEN_HEIGHT][512];
short int Buffer2[SCREEN_HEIGHT][512];

// 当前游戏状态
GameState game_state;

// ========== 自定义一些颜色（RGB565 格式） ==========


// ========== 槽位相关 ==========
// 共有 5 个槽位，每个槽位的图案大小为 25×25 像素
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
int current_slot = 0;                    // 已用槽位数

struct audio_interface {
    volatile unsigned int control;
    volatile unsigned char read_available;
    volatile unsigned char read_left;
    volatile unsigned char write_available;
    volatile unsigned char write_left;
    volatile unsigned int left_data;
    volatile unsigned int right_data;
};

// Pointer to the audio interface hardware registers
struct audio_interface *const audio = (struct audio_interface *)0xff203040;

// ========== 计时器结构体 ==========
struct timer_t {
    volatile unsigned int status;
    volatile unsigned int control;
    volatile unsigned int periodl;
    volatile unsigned int periodh;
    volatile unsigned int snapl;
    volatile unsigned int snaph;
};

// ========== 前置声明 ==========
void plot_pixel(int x, int y, short color);
void wait_for_vsync();
void clear_screen();

int  get_ps2_code();
void draw_background();
void draw_letter_slots();
void HEX_PS2(char b1, char b2, char b3);

void play_audio(int32_t *samples, int n, int step, int replicate);

// ====== 画橙色圆 ====== //
void draw_orange_circle(int x, int y);
void draw_orange_circle_plus1(int x, int y);

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

// 在全局变量区域添加
int note_display[NUM_KEY_SLOTS] = {0};  // 跟踪每个音符的显示状态
int note_count = 0;  // 跟踪已播放的音符数量
char played_notes[NUM_KEY_SLOTS] = {0};  // 记录播放的音符序列

// ------------------------------------------
// 根据游戏状态选择背景
// ------------------------------------------
void draw_background() {
    short color;
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            switch (game_state) {
                case GAME_START:
                    color = background5[y][x]; // start.h
                    break;
                case GAME_STUDY:
                    color = background8[y][x]; // study.h
                    break;
                case GAME_SETUP:
                    // 根据难度级别选择背景
                    switch (difficulty) {
                        case 0: color = background1[y][x]; break; // 难度1
                        case 1: color = background2[y][x]; break; // 难度2
                        case 2: color = background3[y][x]; break; // 难度3
                        case 3: color = background4[y][x]; break; // 难度4
                        default: color = background1[y][x]; break;
                    }
                    break;
                case GAME_PLAYING:
                    // 根据难度级别选择背景
                    switch (difficulty) {
                        case 0: color = background1[y][x]; break; // 难度1
                        case 1: color = background2[y][x]; break; // 难度2
                        case 2: color = background3[y][x]; break; // 难度3
                        case 3: color = background4[y][x]; break; // 难度4
                        default: color = background1[y][x]; break;
                    }
                    break;
                case GAME_SUCCESS:
                    color = background6[y][x]; // suc.h
                    break;
                case GAME_OVER:
                    color = background7[y][x]; // over.h
                    break;
                default:
                    color = BLACK_COLOR;
                    break;
            }
            plot_pixel(x, y, color);
        }
    }
}

// ------------------------------------------
// 槽位里画字母 - 使用自定义坐标
// ------------------------------------------
void draw_letter_slots() {
    // 只在游戏进行中才显示槽位
    if (game_state == GAME_PLAYING) {
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

// 用于显示在HEX显示器上的函数
void HEX_PS2(char b1, char b2, char b3) {
    volatile int * HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
    volatile int * HEX5_HEX4_ptr = (int *)HEX5_HEX4_BASE;

    unsigned char seven_seg_decode_table[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
        0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
    };

    unsigned char hex_segs[] = {0, 0, 0, 0, 0, 0};
    unsigned int shift_buffer, nibble;
    unsigned char code;
    int i;

    shift_buffer = (b1 << 16) | (b2 << 8) | b3;
    for (i = 0; i < 6; ++i) {
        nibble = shift_buffer & 0x0000000F;
        code = seven_seg_decode_table[nibble];
        hex_segs[i] = code;
        shift_buffer = shift_buffer >> 4;
    }

    *(HEX3_HEX0_ptr) = *(int *)(hex_segs);
    *(HEX5_HEX4_ptr) = *(int *)(hex_segs + 4);
}

// ------------------------------------------
int main(void) {
    // 配置硬件接口指针
    volatile int* pixel_ctrl_ptr = (int*)PIXEL_CTRL_BASE;
    volatile int* ps2_ptr = (int*)PS2_BASE;
    struct timer_t *const timerp = (struct timer_t *)TIMER_BASE;
    
    // 初始化PS2键盘
    *ps2_ptr = 0xFF;  // 复位 PS/2
    
    // 初始化双缓冲
    *(pixel_ctrl_ptr + 1) = (int)(&Buffer1);
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen();

    *(pixel_ctrl_ptr + 1) = (int)(&Buffer2);
    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    clear_screen();
    
    // 设置初始游戏状态为开始界面
    game_state = GAME_START;
    
    // 初始化音频系统变量
    const int tone_periods[4] = {10, 20, 30, 40};
    int period_in_samples = tone_periods[0];
    int phase_index = 0;
    int phase_step = TABLE_SIZE / period_in_samples;
    
    // 定义难度级别的持续时间和暂停时间
    const int difficulty_play_time[4] = {4, 2, 2, 2};   // 各难度级别的播放持续触发次数
    const int difficulty_pause_time[4] = {2, 2, 1, 0};  // 各难度级别的暂停持续触发次数
                                 // 当前难度级别 (0-3)
    int timer_count = 0;                                 // 计时器触发计数
    int play_state = 1;                                  // 1=播放，0=暂停
    
    int is_playing = 0;                                  // 总体播放状态
    
    // 设置计时器，每0.25秒触发一次
    timerp->periodl = 0xFFFF;
    timerp->periodh = 0x05F5;  // 约12,500,000周期 = 0.25秒
    timerp->control = 0x7;     // 启动计时器，连续模式，启用中断

    // PS2键盘变量
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;
    
    // 键盘按键状态
    int space_pressed = 0;
    int up_pressed = 0;
    int down_pressed = 0;
    
    while (1) {
        // 读取PS2键盘数据
        PS2_data = *ps2_ptr;
        RVALID = PS2_data & 0x8000;
        
        if (RVALID) {
            byte1 = byte2;
            byte2 = byte3;
            byte3 = PS2_data & 0xFF;
            
            // 显示PS2数据到HEX显示器
            HEX_PS2(byte1, byte2, byte3);
            
            // 上箭头增加难度
            if (byte3 == PS2_KEY_UP && byte2 != 0xF0) {
                up_pressed = 1;
            } else if (byte3 == PS2_KEY_UP && byte2 == 0xF0) {
                if (up_pressed && game_state == GAME_SETUP) {
                    // 上箭头释放，增加难度
                    if (difficulty < 3) {
                        difficulty++;
                    }
                    up_pressed = 0;
                }
            }
            
            // 下箭头减少难度
            if (byte3 == PS2_KEY_DOWN && byte2 != 0xF0) {
                down_pressed = 1;
            } else if (byte3 == PS2_KEY_DOWN && byte2 == 0xF0) {
                if (down_pressed && game_state == GAME_SETUP) {
                    // 下箭头释放，减少难度
                    if (difficulty > 0) {
                        difficulty--;
                    }
                    down_pressed = 0;
                }
            }
            
            // 处理游戏状态切换和按键输入
            if (byte2 != 0xF0) { // 只在按下时处理，不在释放时处理
                // 如果是开始界面，按空格键进入设置界面
                if (game_state == GAME_START && byte3 == PS2_KEY_SPACE) {
                    game_state = GAME_STUDY;
                } 
                else if (game_state == GAME_STUDY ) {
                    switch (byte3) {
                        case PS2_KEY_C:
                            play_audio((int*)audio_samples_c, num_samples_c, 1, 1);
                            break;
                        case PS2_KEY_D:
                            play_audio((int*)audio_samples_d, num_samples_d, 1, 1);
                            break;
                        case PS2_KEY_E:
                            play_audio((int*)audio_samples_e, num_samples_e, 1, 1);
                            break;
                        case PS2_KEY_F:
                            play_audio((int*)audio_samples_f, num_samples_f, 1, 1);
                            break;
                        case PS2_KEY_G:
                            play_audio((int*)audio_samples_g, num_samples_g, 1, 1);
                            break;
                        case 0x5A: // 回车键
                            game_state = GAME_SETUP;
                            difficulty = 0; // 从难度1开始
                            break;
                }
            }
                // 在设置界面，按回车键开始游戏
                else if (game_state == GAME_SETUP && byte3 == PS2_KEY_SPACE) {
                    game_state = GAME_PLAYING;
                    current_slot = 0; // 重置槽位
                    note_count = 0;  // 重置音符计数
                    for (int i = 0; i < NUM_KEY_SLOTS; i++) {
                        letter_slots[i] = 0;
                        note_display[i] = 0;  // 重置显示状态
                        played_notes[i] = 0;  // 重置播放的音符记录
                    }
                    // 开始播放
                    is_playing = 1;
                    play_state = 1;
                    timer_count = 0;
                } 
                // 游戏中状态下的按键处理
                else if (game_state == GAME_PLAYING) {
                    switch (byte3) {
                        case PS2_KEY_C:
                            if (current_slot < NUM_KEY_SLOTS) {
                                letter_slots[current_slot++] = 'C';
                            }
                            break;
                        case PS2_KEY_D:
                            if (current_slot < NUM_KEY_SLOTS) {
                                letter_slots[current_slot++] = 'D';
                            }
                            break;
                        case PS2_KEY_E:
                            if (current_slot < NUM_KEY_SLOTS) {
                                letter_slots[current_slot++] = 'E';
                            }
                            break;
                        case PS2_KEY_F:
                            if (current_slot < NUM_KEY_SLOTS) {
                                letter_slots[current_slot++] = 'F';
                            }
                            break;
                        case PS2_KEY_G:
                            if (current_slot < NUM_KEY_SLOTS) {
                                letter_slots[current_slot++] = 'G';
                            }
                            break;
                    }
                }
                else if ((game_state == GAME_OVER || game_state == GAME_SUCCESS) && byte3 == PS2_KEY_SPACE) {
                    game_state = GAME_START;
                    current_slot = 0; // 重置槽位
                    note_count = 0;  // 重置音符计数
                    for (int i = 0; i < NUM_KEY_SLOTS; i++) {
                        letter_slots[i] = 0;
                        note_display[i] = 0;  // 重置显示状态
                        played_notes[i] = 0;  // 重置播放的音符记录
                    }
                    is_playing = 0;
                    play_state = 1;
                    timer_count = 0;
                    difficulty = 0;  // 重置难度
                }
            
            }
            
            // PS2键盘自动发送的0xAA 0x00序列表示键盘已就绪
            if ((byte2 == (char)0xAA) && (byte3 == (char)0x00)) {
                *ps2_ptr = 0xF4;  // 发送启用键盘命令
            }
        }
        
        // 检查是否计时器触发
        if ((timerp->status & 0x1) != 0) {
            // 清除计时器中断标志
            timerp->status = 0x1;
            
            if (is_playing) {
                timer_count++;
                
                // 根据当前状态和难度级别确定是否需要切换状态
                if (play_state) {
                    // 当前是播放状态
                    if (timer_count >= difficulty_play_time[difficulty]) {
                        // 播放时间结束，切换到暂停状态
                        play_state = 0;
                        timer_count = 0;
                        
                        // 最高难度没有暂停
                        if (difficulty == 3) {
                            play_random_note();
                            play_state = 1;
                        }
                    }
                } else {
                    // 当前是暂停状态
                    if (timer_count >= difficulty_pause_time[difficulty]) {
                        // 暂停时间结束，切换到播放状态
                        play_state = 1;
                        timer_count = 0;
                        
                        play_random_note();
                    }
                }
            }
        }
        

        // 绘制背景（基于当前难度）和槽位
        draw_background();
        draw_letter_slots();
        
        // 绘制问号圆圈
        if (game_state == GAME_PLAYING) {
            if (note_display[0]) draw_question_mark(27, 90);
            if (note_display[1]) draw_question_mark(83, 90);
            if (note_display[2]) draw_question_mark(140, 90);
            if (note_display[3]) draw_question_mark(204, 90);
            if (note_display[4]) draw_question_mark(266, 89);
        }
        
        // 同步和切换缓冲
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
    }
    
    return 0;
}


void play_audio(int32_t *samples, int n, int step, int replicate) {
    int i;
    // Clear output FIFOs and resume conversion
    audio->control = 0x8;
    audio->control = 0x0;
    for (i = 0; i < n; i += step) {
        for (int r = 0; r < replicate; r++) {
            // Wait until there is space in the FIFO
            while (audio->write_available == 0);
            // Write the sample to both channels
            audio->left_data = samples[i];
            audio->right_data = samples[i];
        }
    }
}

void play_random_note() {
    if (note_count >= NUM_KEY_SLOTS) {
        // 检查答案
        int correct_count = 0;
        for (int i = 0; i < NUM_KEY_SLOTS; i++) {
            if (letter_slots[i] == played_notes[i]) {
                correct_count++;
            }
        }
        
        // 根据正确数量决定游戏状态
        if (correct_count >= 3) {
            game_state = GAME_SUCCESS;
        } else {
            game_state = GAME_OVER;
        }
        return;
    }
    
    int r = rand() % 5;  // 生成 0 到 4 之间的随机数
    char note;
    
    // 根据随机数选择音符
    switch (r) {
        case 0:
            play_audio((int*)audio_samples_d, num_samples_d, 1, 1);
            note = 'D';
            break;
        case 1:
            play_audio((int*)audio_samples_c, num_samples_c, 1, 1);
            note = 'C';
            break;
        case 2:
            play_audio((int*)audio_samples_e, num_samples_e, 1, 1);
            note = 'E';
            break;
        case 3:
            play_audio((int*)audio_samples_f, num_samples_f, 1, 1);
            note = 'F';
            break;
        case 4:
            play_audio((int*)audio_samples_g, num_samples_g, 1, 1);
            note = 'G';
            break;
        default:
            note = 'C';
            break;
    }
    
    // 更新显示状态和记录音符
    note_display[note_count] = 1;  // 在当前位置显示新的音符
    played_notes[note_count] = note;  // 记录播放的音符
    note_count++;  // 增加计数
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
void draw_orange_circle_plus1(int x, int y) {
    int cx = x + 13;
    int cy = y + 13;
    int r  = 14;
    int r_sq = r*r;
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r_sq) {
                plot_pixel(cx + dx, cy + dy, PINK);
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
// 画问号
// ------------------------------------------
void draw_question_mark(int x, int y) {
    draw_orange_circle_plus1(x, y);
    int cx = x + 12;
    int cy = y + 12;
    int arc_cx = cx;
    int arc_cy = cy-3;  // move up 5 pixels
    draw_thick_arc(arc_cx, arc_cy, /*outer_r=*/6, /*thick=*/3,
                   /*start_deg=*/180.0f, /*end_deg=*/360.0f,
                   /*color=*/0x0000 /*BLACK*/);

    draw_thick_vertical_line(/*x=*/cx + 5,
                             /*y_start=*/cy - 3,
                             /*length=*/4,
                             /*thickness=*/3,
                             /*color=*/0x0000 /*BLACK*/);

    draw_thick_horizontal_line(/*y=*/(cy + 1),
                               /*x_start=*/(cx),
                               /*length=*/5,
                               /*thickness=*/3,
                               /*color=*/0x0000 /*BLACK*/);

    draw_thick_vertical_line(/*x=*/(cx),
                             /*y_start=*/(cy + 1),
                             /*length=*/5,
                             /*thickness=*/3,
                             /*color=*/0x0000 /*BLACK*/);

    {
        int dot_r = 2;
        int dot_cx = cx;
        int dot_cy = cy + 9;
        for (int dy = -dot_r; dy <= dot_r; dy++) {
            for (int dx = -dot_r; dx <= dot_r; dx++) {
                if (dx*dx + dy*dy <= dot_r*dot_r) {
                    plot_pixel(dot_cx + dx, dot_cy + dy, 0x0000 /*BLACK*/);
                }
            }
        }
    }
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