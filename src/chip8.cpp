#include "chip8.h"

const unsigned char C8::DIGITS[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80
        };
C8::C8()
{

    keyMap = {
        {sf::Keyboard::Key::Num1, 0},
        {sf::Keyboard::Key::Num2, 1},
        {sf::Keyboard::Key::Num3, 2},
        {sf::Keyboard::Key::Num4, 3},
        {sf::Keyboard::Key::Q, 4},
        {sf::Keyboard::Key::W, 5},
        {sf::Keyboard::Key::E, 6},
        {sf::Keyboard::Key::R, 7},
        {sf::Keyboard::Key::A, 8},
        {sf::Keyboard::Key::S, 9},
        {sf::Keyboard::Key::D, 10},
        {sf::Keyboard::Key::F, 11},
        {sf::Keyboard::Key::Z, 12},
        {sf::Keyboard::Key::X, 13},
        {sf::Keyboard::Key::C, 14},
        {sf::Keyboard::Key::V, 15},
    };
    init();
}
void C8::init()
{
    pc_ = 0x200;
    sp_ = -1;
    //store digits on top of memory
    for (int i = 0; i < 80; i++)
    {
        memory[i] = DIGITS[i];
    }

    delay_timer = 0x00;
    sound_timer = 0x00;
    isKeyPressed = false;

    std::srand(std::time(0));
}

void C8::reset() {
    pc_ = 0x200;
    sp_ = -1;
    I_ = 0x0000;
    
    for (int i =0 ; i < 16;i ++) {
        V[i] = 0;
        stack[i] = 0;
    }
    for (int i =0 ; i < 2048;i ++) {
        screen[i] = 0;
    }
    
    delay_timer = 0x00;
    sound_timer = 0x00;
    isKeyPressed = false;
    std::srand(std::time(0));

}
void C8::cycle(){
    fetch();
    decode();
}
void C8::fetch()
{
    unsigned short opc = memory[pc_] << 8 | memory[pc_ + 1];
    opcode_ = opc;
}


void C8::convert_screen(unsigned char *pixels, int multiplier) {
        
        for (int i=0; i < W * H; i++){
            
    
            pixels[i * 4] = screen[i] * 255; //R
            pixels[1 + (i * 4)] = screen[i] * 255; //G
            pixels[2 + (i*4)] = screen[i] * 255; //B
            pixels[3 + (i*4)] = 255; // A

        }
}

void C8::pollKey(sf::RenderWindow &window, sf::Event &event)
{
    
    
        //if keypressed update keyboard
        if (event.type == sf::Event::KeyPressed)
        {
            isKeyPressed = true;
            std::map<sf::Keyboard::Key, int>::iterator iter = keyMap.find(event.key.code);
            if (iter != keyMap.end()){
                keyPressed = iter->second;
                keyboard[iter->second] = 1;
            }
        }

        //if key released update keyboard
        if (event.type == sf::Event::KeyReleased)
        {
            isKeyPressed = false;
            std::map<sf::Keyboard::Key, int>::iterator iter = keyMap.find(event.key.code);
            if (iter != keyMap.end())
                keyboard[iter->second] = 0;
        }

        if (event.type == sf::Event::Closed)
        {
            window.close();
            exit(1);
        }
    
}
void C8::decode()
{

    unsigned char x = (opcode_ & 0x0F00) >> 8;
    unsigned char y = (opcode_& 0x00F0) >> 4;
    unsigned char constant;

    switch (opcode_ & 0xF000)
    {
    case 0x0000:
        //0nnn - 00E0 - 00EE
        switch (opcode_ & 0x00FF)
        {
        case 0x00E0:
            //clear the display - CLS
            for( int i = 0; i < W * H; i++) {
                screen[i] = 0;
            }
            pc_ += 2;
            draw = true;
            break;
        case 0x00EE:
            // Return from subroutine - RET
            pc_ = stack[sp_--];
            pc_ += 2;
            break;
        }
        break;
    case 0x1000:
        // Jump to Xnnn - JP addr
        pc_ = opcode_ & 0x0FFF;
        break;
    case 0x2000:
        // CALL addr        
        //save old pc on top of stack
        stack[++sp_] = pc_;
        pc_ = opcode_ & 0x0FFF;
        break;
    case 0x3000:
        // Skip instruction if Vx == XXkk - SE Vx, XXkk
        constant = opcode_ & 0x00FF;
        if (V[x] == constant)
        {
            pc_ += 4;
            break;
        }
        pc_ += 2;
        break;
    case 0x4000:
        //Skip next instruction if Vx != XXkk - SNE Vx, byte
        constant = opcode_ & 0x00FF;
        if (V[x] != constant)
        {
            pc_ += 4;
            break;
        }
        pc_ += 2;
        break;
    case 0x5000:
        //Skip instruction if Vx == Vy - SE Vx, Vy
        if (V[x] == V[y])
        {
            pc_ += 4;
            break;
        }
        pc_ += 2;
        break;
    case 0x6000:
        //Put XXkk in Vx - LD Vx, byte
        V[x] = opcode_ & 0x00FF;
        pc_ += 2;
        break;
    case 0x7000:
        //Set Vx = Vx + XXkk - ADD Vx, XXkk
        constant = opcode_ & 0x00FF;
        V[x] = V[x] + constant;
        pc_ += 2;
        break;
    case 0x8000:
        switch (opcode_ & 0x000F)
        {

        case 0x0000:
            //Set Vx = Vy - LD Vx, Vy
            V[x] = V[y];
            pc_ += 2;
            break;
        case 0x0001:
            //Set Vx = Vx OR Vy - OR Vx, Vy
            V[x] = V[x] | V[y];
            pc_ += 2;
            break;
        case 0x0002:
            //Set Vx = Vx AND Vy - AND Vx, Vy
            V[x] = V[x] & V[y];
            pc_ += 2;
            break;
        case 0x0003:
            //Set Vx = Vx XOR Vy - XOR Vx, Vy
            V[x] = V[x] ^ V[y];
            pc_ += 2;
            break;
        case 0x0004:
        {
            //Set Vx = Vx + Vy - ADD Vx, Vy
            int sum = V[x] + V[y];
            V[0xF] = 0;
            if (sum > 0xFF)
            {
                V[0xF] = 1;
            }
            V[x] = (sum & 0xFF);
            pc_ += 2;
            break;
        }
        case 0x0005:
            //Set Vx = Vx - Vy - SUB Vx, Vy
            V[0xF] = 0;
            if (V[x] > V[y])
            {
                V[0xF] = 1;
            }
            V[x] = V[x] - V[y];
            pc_ += 2;
            break;

        case 0x0006:
            // SHR Vx {, Vy}
            V[0xF] = V[x] & 0x01;
            V[x] = V[x] >> 1;
            pc_ += 2;
            break;
        case 0x0007:
            // SUBN Vx, Vy
            V[0xF] = 0;
            if (V[y] > V[x])
            {
                V[0xF] = 1;
            }
            V[x] = V[y] - V[x];
            pc_ += 2;
            break;
        case 0x000E:
            // SHL Vx {, Vy}
            V[0xF] = V[x] & 0x01;
            V[x] = V[x] << 1;
            pc_ += 2;
            break;
        }
        break;
    case 0x9000:
        // Skip next instr. - SNE Vx, Vy
        if (V[x] != V[y]) {
            pc_ += 4;
            break;
        }
        pc_ += 2;
        break;
    case 0xA000:
        // LD I, addr
        I_ = opcode_ & 0x0FFF;
        pc_ += 2;
        break;
    case 0xB000:
        // JP V0, addr
        pc_ = (opcode_ & 0x0FFF) + V[0];
        break;
    case 0xC000:
        // RND Vx, byte
        V[x] = (std::rand() % 255) & (opcode_ & 0xFF);
        pc_ += 2;
        break;
    case 0xD000:
    {
        //DRW Vx, Vy, nibble TODO
        V[0xF] = 0;
        unsigned char n = opcode_ & 0x000F;
        for (int i = 0; i < n; i++ ) {
            unsigned char byte_line = memory[I_ + i];
            for (int j=0; j < 8; j++) {

            // xor only if the pixel in the sprite line is 1
            if ((byte_line & (0x80 >> j))) {
                if (screen[((V[y] + i) * W) + (V[x] + j)] == 1) {
                    V[0xF] = 1;
                }
                screen[((V[y] + i) * W) + (V[x] + j)] ^= 1;
                }
            }
        }
        pc_ += 2;
        draw = true;
        break;
    }
    case 0xE000:

        //keyboard
        switch(opcode_ & 0x00FF) {
            case 0x009E: 
                //SKP Vx
                if(keyboard[V[x]]) {
                    pc_ += 4;
                    break;
                }
                pc_ += 2;
                break;
            case 0x00A1:
                //SKNP Vx
                if (!keyboard[V[x]]) {
                    pc_ += 4;
                    break;
                }
                pc_ += 2;
                break;
        }
        break;
    case 0xF000:

        switch (opcode_ & 0x00FF)
        {
        case 0x0007:
            //LD Vx, DT
            V[x] = delay_timer;
            pc_ += 2;
            break;
        case 0x000A:
            //LD Vx, K - wait for a keypress, all executions stop
            if (!isKeyPressed) {
                break;
            }

            V[x] = keyPressed;
            pc_ += 2;
            break;
        case 0x0015:
            //LD DT, Vx
            delay_timer = V[x];
            pc_ += 2;
            break;
        case 0x0018:
            //LD ST, Vx
            sound_timer = V[x];
            pc_ += 2;
            break;
        case 0x001E:
        {
            //ADD I, Vx
            int sum = I_ + V[x];
            V[0xF] = 0;
            if (sum > 0xFF) {
                V[0xF] = 1;
            } 
            I_ = (sum & 0xFFFF);
            pc_ += 2;
            break;
        }
        case 0x0029:
            //LD F, Vx TODO
            I_ = V[x] * 0x5;
            pc_ += 2;
            break;
        case 0x0033:
        {
            //LD B, Vx
            memory[I_] = (V[x] / 100);
            memory[I_ + 1] = (V[x] / 10) % 10;
            memory[I_ + 2] = V[x] % 10;
            pc_ += 2;
            break;
        }
        case 0x0055:
            //LD [I], Vx
            for (int i = 0; i <= x; i++){
                memory[I_ + i] = V[i];
            }

            I_ += V[x] + 1;
            pc_ += 2;
            break;
        case 0x0065:
            //LD Vx, [I]
            for(int i = 0; i <= x; i++) {
                V[i] = memory[I_ + i];
            }
            I_ += V[x] + 1;
            pc_ += 2;
            break;
        default:
            break;
        }

        break;

    default:
        std::cout << "Instruction not supported !\n";
        break;
    }
}

void C8::load(const char *ROM)
{
    std::ifstream rom_bin_file;
    char *temp_buff;
    rom_bin_file.open(ROM, std::ios::binary | std::ios::ate);

    std::streampos size;
    
    if (rom_bin_file.is_open())
    {
        size = rom_bin_file.tellg();
        loaded_program_size = size;
        temp_buff = new char[size];
        rom_bin_file.seekg(0, std::ios::beg);
        rom_bin_file.read(temp_buff, size);

        for (int i = 0; i < size; i++)
        {
            memory[0x200 + i] = temp_buff[i];
        }

        delete[] temp_buff;
        rom_bin_file.close();
    } else {
        fprintf(stderr, "Rom not found \n");
        exit(0);
    }
}
