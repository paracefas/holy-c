#include "./holy-c.hpp"

#include <fstream>


int main(int argc, char const** argv) {
    auto input = load_source(argv[1]);
    auto [ast, rest] = parse(*input); 

    if (ast) {
        auto& [program] = *ast;
        
        std::ofstream out{"out.s"};
        if(!out) {
            PRINT("Error: No se pudo crear el archivo");
            return 1;
        }
        out << ";out.s\n";
        out.close();
        FasmGenerator fasm;
        // AsmStream strm{fasm(program)};
        // out << finalize_fasm_code(strm);
        
        
        PRINT("Exito");
        PRINT("Program: {}", program.size());
    } else {
        PRINT("Error de parseo.");
    }
    return 0;
}