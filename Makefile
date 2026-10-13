SRCS = src/main.cpp src/chip8.cpp
CC = g++
CFLAGS = -Wall -std=c++17
DEBUG = -g
INCLUDES = -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lraylib

all:
	$(CC) $(CFLAGS) $(SRCS) -o build/chip $(INCLUDES) $(LDFLAGS) 

debug:
	$(CC) $(CFLAGS) $(DEBUG) main.cpp chip8.cpp -o chip-dbg $(INCLUDES) $(LDFLAGS)
