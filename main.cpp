#include "./holy-c.hpp"


int main(int argc, char const** argv) {
    auto input = load_source(argv[1]);
    auto [ast, rest] = parse(*input); 

    if (ast) {
        auto& [program] = *ast;
        std::println("Exito");
        std::println("{}", rest);
    } else {
        std::println("Error de parseo.");
    }
    return 0;
}