#include "./holy-c.hpp"

#include <fstream>


int main(int argc, char const** argv) {
    auto input = load_source(argv[1]);
    auto [ast, rest] = parse(*input); 

    if (ast) {
        auto& [program] = *ast;
        
        LLVMGenerator ir;
        ir(program);
        std::error_code ec;
        llvm::raw_fd_ostream dest("output.ll", ec);

        if (ec) {
            std::println("Error abriendo archivo para escribir: {}", ec.message());
            return 1;
        }

        // Suponiendo que tu módulo se llama 'module' dentro de LLVMGenerator
        // y tienes un método público para obtenerlo o imprimirlo:
        ir.dump(dest); 

        std::println("Exito: archivo output.ll generado.");
    } else {
        std::println("Error de parseo.");
    }
    return 0;
}