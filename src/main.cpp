#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <raylib.h>
#include "chip8.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <rom>", argv[0]);
        return 1;
    }

    Chip8 chip8 = Chip8();

    int w = 1024;
    int h = 512;

    const char *title = argv[1];
    InitWindow(w, h, title);
    SetTargetFPS(60);

    if (chip8.load(argv[1]) != 0) {
        printf("Error loading\n");
        return 1;
    }

    while (!WindowShouldClose()) {
        for (int i = 0; i < 10; ++i) {
            chip8.emulate_cycle();
        }

        chip8.update_timers();

        PollInputEvents();

        if (chip8.drawFlag) {
            chip8.drawFlag = false;

            BeginDrawing();
            ClearBackground(BLACK);

            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 64; ++x) {
                    if (chip8.display[x][y] == 1) {
                        DrawRectangle(x * 16, y * 16, 16, 16, WHITE);
                    }
                }
            }
            EndDrawing();
        }
    }

    CloseWindow();
    return 0;
}
