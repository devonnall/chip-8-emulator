#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

class Chip8 {
    private:
        uint8_t memory[4096];
        uint16_t stack[16];
        uint16_t sp;
        uint8_t V[16];
        uint16_t pc;
        uint16_t I;
        uint16_t opcode;
        uint8_t dt;
        uint8_t st;

        void init();
    public:
        uint8_t display[64][32];
        bool drawFlag;

        Chip8();
        ~Chip8();

        int load(const char *file_path);
        void emulate_cycle();
        void update_timers();
};

#endif // CHIP8_H
