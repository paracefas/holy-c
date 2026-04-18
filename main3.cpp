#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <iostream>

int main() {
    llvm::LLVMContext Context;
    llvm::IRBuilder<> Builder(Context);
    auto Module = std::make_unique<llvm::Module>("MiGranCompilador", Context);

    std::cout << "¡LLVM en VS Code funcionando!" << std::endl;
    Module->print(llvm::outs(), nullptr); // Imprime el IR vacío
    return 0;
}