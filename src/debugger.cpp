#include "debugger.h"

Debugger::Debugger(sf::RenderWindow &window, C8 &chip8, sf::Font &font) : window(window), chip8(chip8), font(font)  {
    // disas_instruction.setFont(font_);
    // disas_instruction.setCharacterSize(12);
    fontSize = 12;
    breakpoint = false;
    init_memory_debug();
}

void Debugger::init_memory_debug() {
    unsigned short pc = 0x200;
    unsigned short opcode;
    std::string dis_str;
    
    for(int i = 0; i < chip8.loaded_program_size; i++) {
        opcode = chip8.memory[pc] << 8 | chip8.memory[pc + 1];
        dis_str = disassemble(opcode, pc );
        sf::Text dis_text(dis_str, font, fontSize);
        dis_text.setFillColor(sf::Color::White);
        memory_debug.push_back(dis_text);
        pc += 2;
    }
}

void Debugger::draw_memory_debug() {
    int pos = 0;
    sf::Text instructionText;

    //scroll memory text
    int lowerBound = 0;
    int upperBound = 16;
    int currentInstruction = (chip8.pc_ - 0x200)/2;
    if (currentInstruction != 0 && currentInstruction > lowerBound + 16) {
        lowerBound = currentInstruction;
        upperBound = currentInstruction + 16;
    }

    for (unsigned short i = lowerBound; i < upperBound; i++) {
        instructionText = memory_debug[i];
        instructionText.setPosition(650.f, pos);
        window.draw(instructionText);
        pos += 20;

        if (i == currentInstruction) {
            sf::RectangleShape rect(sf::Vector2f(300.f, 16.f));
            rect.setFillColor(sf::Color(255, 107, 105, 100));
            window.draw(rect, instructionText.getTransform());
        }
    }
    
}
    
void Debugger::draw_V_reg_debug() {
    int pos = 0;
    float offset;
    float xOffset = 635.f;
    for (int i = 0 ; i < 16; i++ ) {
        std::stringstream stream;
        stream << "V" << i << " = ";
        stream << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.V[i]; 
        sf::Text registerText(stream.str(), font, fontSize);
        if (i == 16 / 2) {
            offset = 100.f;
            pos = 0;
        }
        
        registerText.setPosition(offset + xOffset, 330.f + pos );
        window.draw(registerText);
        pos += 20;

    }
}

void Debugger::draw_common_reg() {
    std::stringstream stream;
    sf::Text *text;
    float xOffset = 840.f;
    //I
    stream << "I = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.I_; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 330.f);
    window.draw(*text);

    //opcode 
    stream.str("");
    stream << "Opcode = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.opcode_; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 350.f);
    window.draw(*text);

    //pc
    stream.str("");
    stream << "PC = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.pc_; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 370.f);
    window.draw(*text);

    //SP
    stream.str("");
    stream << "SP = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.sp_; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 390.f);
    window.draw(*text);

    //DT
    stream.str("");
    stream << "DT = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.delay_timer; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 410.f);
    window.draw(*text);

    //ST
    stream.str("");
    stream << "ST = " << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (int) chip8.sound_timer; 
    text = new sf::Text(stream.str(), font, fontSize);
    text->setPosition(xOffset, 430.f);
    window.draw(*text);
}

void Debugger::draw_info() {
    std::stringstream stream;
    stream << "############################# CHIP8 INTERPRETER ############################" << "\n";
    stream << "                                                                            " << "\n";
    stream << "                     USAGE                                                  " << "\n";
    stream << "                                                                            " << "\n";
    stream << " Backspace   Set/Unset a breakpoint from the                                " << "\n";
    stream << "             current instruction being executed          Keyboard Layout    " << "\n";
    stream << "                                                                            " << "\n";
    stream << " Down arrow  Move to the next instruction              1 2 3 C      1 2 3 4 " << "\n";
    stream << "                                                       4 5 6 D  ->  Q W E R " << "\n";
    stream << " PageDown    Increase clock frequency                  7 8 9 E      A S D F " << "\n";
    stream << " PageUp      Decrease clock frequency                  A 0 B F      Z X C V " << "\n";
    stream << "                                                                            " << "\n";
    stream << " Home        Reset the program                                              " << "\n";
    stream << "                                                                            " << "\n";
    stream << "############################################################################" << "\n";

    sf::Text text(stream.str(), font, fontSize);
    text.setPosition(10.f, 330.f);
    window.draw(text);

    //draw breakpoint info 
    if (breakpoint) {
        stream.str("");
        stream << "*Breakpoint set*";
        text.setString(stream.str());
        text.setFillColor(sf::Color::Red);
        text.setPosition(500.f, 330.f);
        window.draw(text);
    }
    

}

void Debugger::next() {
    if (breakpoint) {
        chip8.cycle();
    }
}

void Debugger::set_breakpoint(bool bp) {
    breakpoint = bp;
}

std::string Debugger::disassemble(unsigned short opcode, unsigned short pos) {

    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char y = (opcode & 0x00F0) >> 4;
    std::stringstream fmt;
    fmt << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << pos << "\t";
    fmt << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << opcode << "\t";
    switch (opcode & 0xF000)
    {
    case 0x0000:
        //0nnn - 00E0 - 00EE
        switch (opcode & 0x00FF)
        {
        case 0x00E0:
            
            fmt << "CLS" << "\n";
            break;
        case 0x00EE:
            // Return from subroutine - RET
            fmt << "RET" << "\n";
            break;
        }
        break;
    case 0x1000:
        // Jump to Xnnn - JP addr
        fmt << "JP " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x0FFF) << "\n";
        break;
    case 0x2000:
        // CALL addr        
        //save old pc on top of stack
        fmt << "CALL " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x0FFF) << "\n";
        break;
    case 0x3000:
        // Skip instruction if Vx == XXkk - SE Vx, XXkk
        fmt << "SE V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x00FF) << "\n";
        break;
    case 0x4000:
        //Skip next instruction if Vx != XXkk - SNE Vx, byte
        fmt << "SNE V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x00FF) << "\n";
        break;
    case 0x5000:
        //Skip instruction if Vx == Vy - SE Vx, Vy
        fmt << "SE V" << (int) x << ", V" << (int) y << "\n";
        break;
        break;
    case 0x6000:
        //Put XXkk in Vx - LD Vx, byte
        fmt << "LD V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x00FF) << "\n";
        break;
    case 0x7000:
        //Set Vx = Vx + XXkk - ADD Vx, XXkk
        fmt << "ADD V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x00FF) << "\n";
        break;
    case 0x8000:
        switch (opcode & 0x000F)
        {

        case 0x0000:
            //Set Vx = Vy - LD Vx, Vy
            fmt << "SE V" << (int) x << ", V" << (int) y << "\n";
            break;
            break;
        case 0x0001:
            //Set Vx = Vx OR Vy - OR Vx, Vy
            fmt << "OR V" << (int) x << ", V" << (int) y << "\n";
            break;
        case 0x0002:
            //Set Vx = Vx AND Vy - AND Vx, Vy
            fmt << "AND V" << (int) x << ", V" << (int) y << "\n";
            break;
        case 0x0003:
            //Set Vx = Vx XOR Vy - XOR Vx, Vy
            fmt << "XOR V" << (int) x << ", V" << (int) y << "\n";
            break;
        case 0x0004:
        {
            //Set Vx = Vx + Vy - ADD Vx, Vy
            fmt << "ADD V" << (int) x << ", V" << (int) y << "\n";
            break;
        }
        case 0x0005:
            //Set Vx = Vx - Vy - SUB Vx, Vy
            fmt << "SUB V" << (int) x << ", V" << (int) y << "\n";
            break;

        case 0x0006:
            // SHR Vx {, Vy}
            fmt << "SHR V" << (int) x << "\n";
            break;
        case 0x0007:
            // SUBN Vx, Vy
            fmt << "SUBN V" << (int) x << ", V" << (int) y << "\n";
            break;
            
        case 0x000E:
            // SHL Vx {, Vy}
            fmt << "SHL V" << (int) x << "\n";
            break;
        }
        break;
    case 0x9000:
        // Skip next instr. - SNE Vx, Vy
        fmt << "SNE V" << (int) x << ", V" << (int) y << "\n";
        break;
    case 0xA000:
        // LD I, addr
        fmt << "LD I" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x0FFF) << "\n";
        break;
    case 0xB000:
        // JP V0, addr
        fmt << "JP V0, " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x0FFF) << "\n";
        break;
    case 0xC000:
        // RND Vx, byte
        fmt << "RND V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x00FF) << "\n";
        break;
        break;
    case 0xD000:
    {
        //DRW Vx, Vy, nibble TODO
        fmt << "DRW V" << (int) x << ", " << "V" << (int) y << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << (opcode & 0x000F) << "\n";
        break;
        break;
    }
    case 0xE000:

        //keyboard
        switch(opcode & 0x00FF) {
            case 0x009E: 
                //SKP Vx
                fmt << "SKP V" << (int) x <<  "\n";
                break;
            case 0x00A1:
                //SKNP Vx
                fmt << "SKNP V" << (int) x <<  "\n";
                break;
        }
        break;
    case 0xF000:

        switch (opcode & 0x00FF)
        {
        case 0x0007:
            //LD Vx, DT
            fmt << "LD V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << chip8.delay_timer;
            break;
        case 0x000A:
            //LD Vx, K - wait for a keypress, all executions stop
            fmt << "LD V" << (int) x << ", " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << chip8.keyPressed;
            break;
            
        case 0x0015:
            //LD DT, Vx
            fmt << "LD " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << chip8.delay_timer << ", V" << (int) x << "\n" ;
            break;
        case 0x0018:
            //LD ST, Vx
            fmt << "LD " << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << chip8.sound_timer << ", V" << (int) x << "\n" ;
            break;
        case 0x001E:
        {
            //ADD I, Vx
            fmt << "ADD I, " << "V " << (int) x << "\n" ;
            break;
        }
        case 0x0029:
            //LD F, Vx TODO
            fmt << "LD F, " << "V " << (int) x << "\n" ;
            break;
        case 0x0033:
        {
            //LD B, Vx
            fmt << "LD B, " << "V " << (int) x << "\n" ;
            break;
        }
        case 0x0055:
            //LD [I], Vx
            fmt << "LD [I], " << "V " << (int) x << "\n" ;
            break;
        case 0x0065:
            //LD Vx, [I]
            fmt << "LD V" << (int) x << ", [I]" << "\n" ;
            break;
        }

        break;

    
    }
    
    return fmt.str();
}