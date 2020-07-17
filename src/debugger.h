#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include "chip8.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

class Debugger {
    public:
        Debugger(sf::RenderWindow &window, C8 &chip8, sf::Font &font);
        void draw_memory_debug();
        void draw_V_reg_debug();
        void draw_common_reg();
        void draw_info();
        void set_breakpoint(bool bp);
        bool breakpoint;
        void next();

    private:
        void init_memory_debug();
        std::string disassemble(unsigned short opcode, unsigned short position);
        sf::RenderWindow &window;
        C8 &chip8;
        sf::Font &font;
        std::vector<sf::Text> memory_debug;
        int fontSize;
};

#endif
