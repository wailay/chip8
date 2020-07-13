#ifndef CHIP8_H
#define CHIP8_H

#include <fstream>
#include <ctime>

const int H = 32, W = 64;

class C8 {

public:

    C8();
    unsigned char delay_timer;
    unsigned char sound_timer;
    void load(const char* ROM);
private:
    unsigned char memory[4096];
    unsigned char V[16]; //16 general purpose registers
    unsigned short stack[16];

    unsigned char screen[2048];
    unsigned char keyboard[16];
    unsigned short opcode_;
    unsigned short pc_;
    unsigned short I_;
    unsigned char sp_;
    const static unsigned char DIGITS[];
    void init();
    void decode(unsigned short opcode);
    unsigned short fetch();

};

#endif