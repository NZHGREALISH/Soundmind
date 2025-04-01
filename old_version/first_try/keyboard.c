#define PS2_BASE 0xFF200100
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030

void HEX_PS2(char, char, char);

int main(void) {
    volatile int * PS2_ptr = (int *)PS2_BASE;
    int PS2_data, RVALID;
    char byte1 = 0, byte2 = 0, byte3 = 0;

    *(PS2_ptr) = 0xFF;

    while (1) {
        PS2_data = *(PS2_ptr);
        RVALID = PS2_data & 0x8000;
        if (RVALID) {
            byte1 = byte2;
            byte2 = byte3;
            byte3 = PS2_data & 0xFF;
            HEX_PS2(byte1, byte2, byte3);
        }

        if ((byte2 == (char)0xAA) && (byte3 == (char)0x00)) {
            *(PS2_ptr) = 0xF4;
        }
    }
}

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