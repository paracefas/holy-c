#include "./holy-c.hpp"

#include <fstream>


int main(int argc, char const** argv) {
    auto input = load_source(argv[1]);
    auto [ast, rest] = parse(*input); 

    if (ast) {
        auto& [program] = *ast;
        std::println("Exito: archivo {} parsed: {}", argv[1], program.size());
        
    } else {
        std::println("Error de parseo.");
    }
    return 0;
}