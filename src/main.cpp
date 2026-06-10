#include "Assembler.hpp"
#include "CPU.hpp"
#include "Loader.hpp"
#include <iostream>
#include <string>
 
static bool endsWith(const std::string& s, const std::string& suffix) {
    return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}
 
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: emulator <file.asm|file.bin>\n";
        return 1;
    }
 
    CPU cpu;
    std::string path = argv[1];
 
    try {
        if (endsWith(path, ".txt") || endsWith(path, ".asm")) {
            auto bytes = Assembler::assemble(path);
            Loader::loadBytes(cpu, bytes);
        } else {
            Loader::load(cpu, path);
        }
        cpu.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
 
    return 0;
}