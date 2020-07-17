#include <iostream>
#include <fstream>
#include "chip8.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <iomanip>
#include <unistd.h>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cstring>
#include <math.h>
#include "debugger.h"

std::map<sf::Keyboard::Key, int> keyMap;

int PERIOD_US = 2000;

void init_split_screen(std::vector<sf::Vertex> *splits)
{
    sf::Color grey(110, 110, 110);

    sf::Vertex verticalSplit1(sf::Vector2f(640.f, 0), grey);
    sf::Vertex verticalSplit2(sf::Vector2f(640.f, 600.f), grey);
    splits->push_back(verticalSplit1);
    splits->push_back(verticalSplit2);

    sf::Vertex horSplit1(sf::Vector2f(0, 320.f), sf::Color(110, 110, 110));
    sf::Vertex horSplit2(sf::Vector2f(1000.f, 320.f), grey);

    splits->push_back(horSplit1);
    splits->push_back(horSplit2);
}

void init_sound(sf::Sound *sound, sf::SoundBuffer *sbuffer)
{
    signed short buffer[44100];
    double x = 0;
    for (int i = 0; i < 44100; i++)
    {
        buffer[i] = 10000 * sin(x * 2.f * 3.14159265f);
        x += 700.f / 44100.f;
    }
    sbuffer->loadFromSamples(buffer, 44100, 1, 44100);
    sound->setBuffer(*sbuffer);
    sound->setLoop(true);

}
int main(int argc, char **argv)
{

    if (argc < 2)
    {
        fprintf(stderr, "You need to specify a rom to load as an argument - ./chip8 <ROM> \n");
        exit(0);
    }

    C8 *chip8 = new C8();
    std::stringstream rom;
    std::string romName = argv[1];
    rom << "./roms/" << romName << ".ch8";
    chip8->load(rom.str().c_str());
    sf::RenderWindow window(sf::VideoMode(1000, 600), "ch8");

    std::vector<sf::Vertex> splits;
    init_split_screen(&splits);

    sf::Sound sound;
    sf::SoundBuffer sbuffer;
    init_sound(&sound, &sbuffer);
    

    sf::Texture txt;
    txt.create(64, 32);
    unsigned char *pixels = new unsigned char[64 * 32 * 4];
    sf::Sprite sprite;
    //The sprite contains the texture that represent the chip8 64x32 pixel screen, it is then scaled by 10 times.
    sprite.setScale(10.f, 10.f);

    sf::Font font;
    font.loadFromFile("share.ttf");
    Debugger dbgr(window, *chip8, font);

    unsigned long long start, last_clock = 0;
    double elapsed_time;
    double acc = 0;
    bool running = true;

    while (running)
    {
        window.clear();
        sf::Event event;

        start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        elapsed_time = (double)(start - last_clock) / 1000.f;
        if ( last_clock == 0) {
            elapsed_time = (1.f / 60.f);
        }
        last_clock = start;
        acc += elapsed_time;

        dbgr.draw_memory_debug();
        dbgr.draw_V_reg_debug();
        dbgr.draw_common_reg();
        dbgr.draw_info();

        if (!dbgr.breakpoint){
            chip8->cycle();
        }
        

        while (acc >= (1.f / 60.f))
        {

            if (chip8->delay_timer > 0)
            {
                chip8->delay_timer--;
            }
            if (chip8->sound_timer > 0)
            {
                sound.play();
                chip8->sound_timer--;
                if (chip8->sound_timer == 0)
                    sound.stop();
            }
            acc -= (1.f / 60.f);
        }

        if (chip8->draw)
        {
            chip8->convert_screen(pixels, 1);
            txt.update(pixels);
            sprite.setTexture(txt);
            sprite.scale(1.f, 1.f);
            chip8->draw = false;
        }
        
        window.draw(sprite);
        window.draw(&splits[0], splits.size(), sf::Lines);
        window.display();
        while (window.pollEvent(event))
        {

            chip8->pollKey(window, event);
            //if keypressed update keyboard
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::PageUp)
                {
                    if (PERIOD_US > 0)
                        PERIOD_US -= 500;
                }

                if (event.key.code == sf::Keyboard::PageDown)
                {
                    PERIOD_US += 500;
                }

                if (event.key.code == sf::Keyboard::Space)
                {
                    dbgr.set_breakpoint(!dbgr.breakpoint);
                }

                //step next instruction if breakpoint is active
                if (event.key.code == sf::Keyboard::Down)
                {
                    dbgr.next();
                }

                //reset
                if (event.key.code == sf::Keyboard::Home)
                {
                    chip8->reset();
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(PERIOD_US));
    }

    return 0;
}