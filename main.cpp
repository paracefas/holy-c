#include "./holy-c.hpp"

int main() {
    auto input = load_source("examples/hello_world.hc");
    if (input) {
        std::println("Leído con éxito: {} caracteres", input->size());
    } else {
        std::println(stderr, "Error código: {}", (int)input.error());
        return -1;
    }
    auto [res, rest] = parseFunc(*input);

    if (res) {
        Stmt mi_sentencia = std::get<0>(*res);
        std::println("¡Parseo exitoso!");

        std::println("{}", mi_sentencia);
    } else {
        std::println("Error: No se pudo parsear. Resto del input: '{}'", rest);
    }
    return 0;
}