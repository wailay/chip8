#include "chip8.h"
#include "SDL.h"
const unsigned char C8::DIGITS[] = {
0xF0,0x90,0x90,0x90,0xF0,
0x20,0x60,0x20,0x20,0x70,
0xF0,0x10,0xF0,0x80,0xF0,
0xF0,0x10,0xF0,0x10,0xF0,
0x90,0x90,0xF0,0x10,0x10,
0xF0,0x80,0xF0,0x10,0xF0,
0xF0,0x80,0xF0,0x90,0xF0,
0xF0,0x10,0x20,0x40,0x40,
0xF0,0x90,0xF0,0x90,0xF0,
0xF0,0x90,0xF0,0x10,0xF0,
0xF0,0x90,0xF0,0x90,0x90,
0xE0,0x90,0xE0,0x90,0xE0,
0xF0,0x80,0x80,0x80,0xF0,
0xE0,0x90,0x90,0x90,0xE0,
0xF0,0x80,0xF0,0x80,0xF0,
0xF0,0x80,0xF0,0x80,0x80
};
C8::C8() {
    init();
}
void C8::init() {
    pc_ = 0x200;
    sp_ = -1;
    //store digits on top of memory
    for (int i = 0; i < 80; i++) {
        memory[i] = DIGITS[i];
    }

    delay_timer = 0x00;
    sound_timer = 0x00;

    std::srand(std::time(0));

}

unsigned short C8::fetch() {

    unsigned short opc = memory[pc_] >> 8 | memory[pc_ + 1];
    pc_ += 2;
    return opc;
}

void C8::decode(unsigned short opcode) {

    unsigned char x = opcode & 0x0F00;
    unsigned char y = opcode & 0x00F0;
    unsigned char constant;

    switch (opcode & 0xF000)
    {
    case 0x0000: 
        //0nnn - 00E0 - 00EE
        switch (opcode & 0x00FF)
        {
        case 0x00E0: 
            //clear the display - CLS
            break;
        case 0x00EE: 
            // Return from subroutine - RET
            break;
        }
        break;
    case 0x1000: 
        // Jump to Xnnn - JP addr
        pc_ = opcode & 0x0FFF;
        break;
    case 0x2000: 
        // CALL addr
        sp_++;
        //save old pc on top of stack
        stack[sp_] = pc_; 
        pc_ = opcode & 0x0FFF;
        break;
    case 0x3000: 
        // Skip instruction if Vx == XXkk - SE Vx, XXkk
        constant = opcode & 0x00FF;
        if (V[x] == constant) {
            pc_ += 2;
        }
        break;
    case 0x4000:
        //Skip next instruction if Vx != XXkk - SNE Vx, byte
        constant = opcode & 0x00FF;
        if (V[x] != constant) {
            pc_ += 2;
        }
        break;
    case 0x5000:
        //Skip instruction if Vx == Vy - SE Vx, Vy
        if (V[x] == V[y]) {
            pc_ += 2;
        }
        break;
    case 0x6000:
        //Put XXkk in Vx - LD Vx, byte
        V[x] = opcode & 0x00FF;
        break;
    case 0x7000:
        //Set Vx = Vx + XXkk - ADD Vx, XXkk
        constant = opcode & 0x00FF;
        V[x] = V[x] + constant;
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {
          
        case 0x0000:
            //Set Vx = Vy - LD Vx, Vy
            V[x] = V[y];
            break;
        case 0x0001:
            //Set Vx = Vx OR Vy - OR Vx, Vy
            V[x] = V[x] | V[y];
            break;
        case 0x0002:
            //Set Vx = Vx AND Vy - AND Vx, Vy
            V[x] = V[x] & V[y];
            break;
        case 0x0003:
            //Set Vx = Vx XOR Vy - XOR Vx, Vy
            V[x] = V[x] ^ V[y];
            break;
        case 0x0004:
        {
            //Set Vx = Vx + Vy - ADD Vx, Vy
            int sum = V[x] + V[y];
            V[0xF] = 0;
            if (sum > 0xFF) {
                V[0xF] = 1;
                break;
            }
            V[x] = (sum & 0xFF);
            break;
        }
        case 0x0005:
            //Set Vx = Vx - Vy - SUB Vx, Vy
            V[0xF] = 0;
            if (V[x] > V[y]) {
                V[0xF] = 1;
            }
            V[x] = V[x] - V[y];
            break;
        
        case 0x0006:
            // SHR Vx {, Vy}
            V[0xF] = V[x] & 0x01;
            V[x] = V[x] >> 1;
            break;
        case 0x0007:
            // SUBN Vx, Vy
            V[0xF] = 0;
            if (V[y] > V[x]) {
                V[0xF] = 1;
            }
            V[x] = V[y] - V[x];
            break;
        case 0x000E:
            // SHL Vx {, Vy}
            V[0xF] = V[x] & 0x01;
            V[x] = V[x] << 1;
            break;

        }
        break;
    case 0x9000:
        // Skip next instr. - SNE Vx, Vy
        if (V[x] != V[y]) pc_ += 2;
        break;
    case 0xA000:
        // LD I, addr
        I_ = opcode & 0x0FFF;
        break;
    case 0xB000:
        // JP V0, addr
        pc_ = (opcode & 0x0FFF) + V[0];
        break;
    case 0xC000:
        // RND Vx, byte
        V[x] = std::rand() | (opcode & 0xFF);
        break;
    case 0xD000:
    {
        //DRW Vx, Vy, nibble
        
        break;
    }
    case 0xE000:
    
        //keyboard

        break;
    case 0xF000:

        switch (opcode & 0x00FF)
        {
        case 0x0007:
            //LD Vx, DT
            V[x] = delay_timer;
            break;
        case 0x000A:
            //LD Vx, K - wait for a keypress, all executions stop

            break;
        case 0x0015:

            break;
        case 0x0018:
            break;
        case 0x001E:
            break;
        case 0x0029:
            break;
        case 0x0033:
            break;
        case 0x0055:
            break;
        case 0x0065:
            break;
        default:
            break;
        }

        break;

    default:
        break;
    }
}

void C8::load(const char* ROM) {
    std::ifstream rom_bin_file;
    char* temp_buff;
    rom_bin_file.open(ROM, std::ios::binary | std::ios::ate);

    std::streampos size;
    if (rom_bin_file.is_open()) {
        size = rom_bin_file.tellg();
        temp_buff = new char[size];
        rom_bin_file.seekg(0, std::ios::beg);
        rom_bin_file.read(temp_buff, size);

        for (int i = 0; i < size; i++) {
            memory[0x200 + i] = temp_buff[i];
        }

        delete[] temp_buff;
        rom_bin_file.close();
    }


}
