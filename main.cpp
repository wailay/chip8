#include <iostream>
#include <fstream>
#include "chip8.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <iomanip>
#include <unistd.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>

std::map<sf::Keyboard::Key, int> keyMap;

#define PERIOD_MS 2


int main(int argc, char **argv) {

    sf::RenderWindow window(sf::VideoMode(1000, 600), "ch8");
    sf::Texture txt;
    txt.create(64,32);
    unsigned char* pixels = new unsigned char[64*32*4];
    sf::Color grey(110, 110, 110);
    std::vector<sf::Vertex> splits;
    sf::Vertex verticalSplit1(sf::Vector2f(640.f, 0), grey);
    sf::Vertex verticalSplit2(sf::Vector2f(640.f, 600.f), grey);
    splits.push_back(verticalSplit1);
    splits.push_back(verticalSplit2);

    sf::Vertex horSplit1(sf::Vector2f(0, 320.f), sf::Color(110, 110, 110));
    sf::Vertex horSplit2(sf::Vector2f(1000.f, 320.f), grey);

    splits.push_back(horSplit1);
    splits.push_back(horSplit2);

    

    C8* chip8 = new C8();

    char rom[] = "./roms/";
    strcat(rom, argv[1]);
    char ext[] = ".ch8";
    strcat(rom, ext);

    chip8->load(rom);
    
    sf::Sprite sprite;
    sprite.setScale(10.f, 10.f);
    std::cout << chip8->delay_timer << "\n";
    int n;

    unsigned long long start, last_clock;
    unsigned long long start2, last_clock2;
    double elapsed_time, e2;
    double acc = 0;
    while(true){
        
        start = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count();
        elapsed_time = (double) (start - last_clock) / 1000.f;
        if (elapsed_time >= (1.f/60.f))
            elapsed_time = (1.f / 60.f);
        last_clock = start;
        acc += elapsed_time;

        
        chip8->cycle();
        chip8->pollKey(window);
        
        while (acc >= (1.f/60.f)) {
            
            if (chip8->delay_timer > 0) {
                chip8->delay_timer--;
            }
            if (chip8->sound_timer > 0) {
                std::cout << "SOUND !!\n";
                chip8->sound_timer--;
            }
            acc -= (1.f/60.f);
        }

        if (chip8->draw) {
            chip8->convert_screen(pixels, 1);
            txt.update(pixels);
            sprite.setTexture(txt);
            sprite.scale(1.f, 1.f);
            window.clear();
            window.draw(sprite);
            window.draw(&splits[0], splits.size(), sf::Lines);
            window.display();
            chip8->draw = false;
        }
        // std::cout << "next";
        // std::cin.get();

        
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
    }
    

    return 0;


}