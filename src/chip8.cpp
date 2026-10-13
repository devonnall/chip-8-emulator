#include "chip8.h"
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <raylib.h>

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

int get_raylib_key(uint8_t chip8_key) {
    switch (chip8_key) {
        case 0x1: return KEY_ONE;
        case 0x2: return KEY_TWO;
        case 0x3: return KEY_THREE;
        case 0xC: return KEY_FOUR;
        case 0x4: return KEY_Q;
        case 0x5: return KEY_W;
        case 0x6: return KEY_E;
        case 0xD: return KEY_R;
        case 0x7: return KEY_A;
        case 0x8: return KEY_S;
        case 0x9: return KEY_D;
        case 0xE: return KEY_F;
        case 0xA: return KEY_Z;
        case 0x0: return KEY_X;
        case 0xB: return KEY_C;
        case 0xF: return KEY_V;
        default:  return 0;
    }
}

Chip8::Chip8() {}
Chip8::~Chip8() {}

void Chip8::init() {
    pc = 0x200;
    sp = 0;
    opcode = 0;
    I = 0;
    dt = 0;
    st = 0;
    drawFlag = false;
    
    for (int i = 0; i < 4096; ++i) {
        memory[i] = 0;
    }

    for (int i = 0; i < 16; ++i) {
        stack[i] = 0;
        V[i] = 0;
    }

    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 32; ++j) {
            display[i][j] = 0;
        }
    }

    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8_fontset[i];
    }

    srand(time(NULL));
}

int Chip8::load(const char *file_path) {
    init();

    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        perror("fopen");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    if (4096 - 512 < file_size) {
        printf("Error: ROM size is too large\n");
        return -1;
    }

    char *buf = (char*)malloc(file_size * sizeof(char));
    if (buf == NULL) {
        perror("malloc");
        return -1;
    }

    size_t bytes_read = fread(buf, 1, (size_t)file_size, fp);
    if (bytes_read != file_size) {
        printf("Error reading ROM file\n");
        return -1;
    }

    for (int i = 0; i < file_size; ++i) {
        memory[i + 512] = (uint8_t)buf[i];
    }

    fclose(fp);
    free(buf);
    return 0;
}

void Chip8::update_timers() {
    if (dt > 0) dt--;
    if (st > 0) st--;
}

void Chip8::emulate_cycle() {
    opcode = ((uint16_t)memory[pc] << 8) | memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    for (int x = 0; x < 64; ++x) {
                        for (int y = 0; y < 32; ++y) {
                            display[x][y] = 0;
                        }
                    }
                    drawFlag = true;
                    pc += 2;
                    break;
                case 0x00EE:
                    pc = stack[--sp];
                    pc += 2;
                    break;
                default:
                    pc += 2;
                    break;
            }
            break;
        case 0x1000:
            pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        case 0x4000:
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        case 0x5000:
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    break; 
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0003:
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0004:
                    if (V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4] > 255) {
                        V[15] = 1;
                    } else {
                        V[15] = 0;
                    }

                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
break;
                case 0x0005:
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
                        V[15] = 1;
                    } else {
                        V[15] = 0;
                    }

                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0006:
                    V[15] = V[(opcode & 0x0F00) >> 8] & 0x01;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    break;
                case 0x0007:
                    if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) {
                        V[15] = 1;
                    } else {
                        V[15] = 0;
                    }

                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0f00) >> 8];
                    break;
                case 0x000E:
                    V[15] = (V[(opcode & 0x0F00) >> 8] & 0x80) >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    break;
                default:
                    printf("Unknown opcode: %.4X\n", opcode);
                    break;
            }

            pc += 2;
            break;
        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        case 0xA000:
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000:
            pc = (opcode & 0x0FFF) + (uint16_t)V[0];
            break;
        case 0xC000:
            V[(opcode & 0x0F00) >> 8] = (uint8_t)(rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;
        case 0xD000: 
        {
            uint8_t xCoord = V[(opcode & 0x0F00) >> 8] % 64;
            uint8_t yCoord = V[(opcode & 0x00F0) >> 4] % 32;
            uint16_t height = opcode & 0x000F;
            uint16_t pixel;

            V[0xF] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        int xPos = (xCoord + xline) % 64;
                        int yPos = (yCoord + yline) % 32;

                        if (display[xPos][yPos] == 1) {
                            V[0xF] = 1;
                        }
                        display[xPos][yPos] ^= 1;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
        }
            break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:
                {
                    uint8_t key = V[(opcode & 0x0F00) >> 8];
                    if (IsKeyDown(get_raylib_key(key))) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                }
                    break;
                case 0x00A1:
                {
                    uint8_t key = V[(opcode & 0x0F00) >> 8];
                    if (!IsKeyDown(get_raylib_key(key))) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                }
                    break;
                default:
                    printf("Unknown opcode: %.4X\n", opcode);
                    pc += 2;
                    break;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = dt;
                    break;
                case 0x000A:
                {
                    int pressed = 0;
                    uint8_t chip8_key = 0;

                    int test_keys[] = {
                        KEY_X, KEY_ONE, KEY_TWO, KEY_THREE, 
                        KEY_Q, KEY_W, KEY_E, KEY_A, 
                        KEY_S, KEY_D, KEY_Z, KEY_C, 
                        KEY_FOUR, KEY_R, KEY_F, KEY_V
                    };
                    int chip8_vals[] = {
                        0x0, 0x1, 0x2, 0x3, 
                        0x4, 0x5, 0x6, 0x7, 
                        0x8, 0x9, 0xA, 0xB, 
                        0xC, 0xD, 0xE, 0xF
                    };

                    for (int i = 0; i < 16; ++i) {
                        if (IsKeyPressed(test_keys[i])) {
                            chip8_key = chip8_vals[i];
                            pressed = 1;
                            break;
                        }
                    }

                    if (!pressed) {
                        return;
                    }

                    V[(opcode & 0x0F00) >> 8] = chip8_key;
                }
                    break;
                case 0x0015:
                    dt = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    st = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x001E:
                    I += V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8] * 5;
                    break;
                case 0x0033:
                {
                    uint8_t vx = V[(opcode & 0x0F00) >> 8];
                    memory[I+2] = vx % 10;
                    vx /= 10;
                    memory[I+1] = vx % 10;
                    vx /= 10;
                    memory[I] = vx % 10;
                }
                    break;
                case 0x0055:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        memory[I + i] = V[i];
                    }
                    break;
                case 0x0065:
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i) {
                        V[i] = memory[I + i];
                    }
                    break;
                default:
                    printf("Unknown opcode: %.4X\n", opcode);
                    pc += 2;
                    break;
            }
            pc += 2;
            break;
        default:
            printf("Unknown opcode: %.4X\n", opcode);
            pc += 2;
            break;
    }
}
