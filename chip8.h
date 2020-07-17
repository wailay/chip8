#ifndef CHIP8_H
#define CHIP8_H

#include <fstream>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>

const int H = 32, W = 64;

class C8 {

public:

    C8();
    
    bool draw;
    int loaded_program_size;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char keyboard[16];

    void load(const char* ROM);
    void pollKey(sf::RenderWindow &window, sf::Event &event);
    void cycle();
    void reset();


    std::map<sf::Keyboard::Key, int> keyMap;
    static const unsigned char DIGITS[];
    
    
    unsigned char memory[4096];
    unsigned char V[16]; //16 general purpose registers
    unsigned char screen[2048];
    unsigned char sp_;
    unsigned char keyPressed;

    unsigned short stack[16];
    unsigned short opcode_;
    unsigned short pc_;
    unsigned short I_;

    void convert_screen(unsigned char *pixels, int multiplier);
    
    private:
        bool isKeyPressed;
        void fetch();
        void decode();
        void init();


};

#endif