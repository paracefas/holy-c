#include "./holy-c.hpp"


int main(int argc, char const** argv) {
    auto input = load_source(argv[1]);
    auto [ast, rest] = parse(*input); 

    if (ast) {
        auto& [program] = *ast;

        PRINT("Exito");
        PRINT("{}", rest);
    } else {
        PRINT("Error de parseo.");
    }
    return 0;
}