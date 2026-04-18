#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

#include <map>
#include <memory>

#include "ast.hpp"

class LLVMGenerator {
public:
    // --- MANEJO DE VARIANTES Y WRAPPERS (Crucial para que compile) ---
    LLVMGenerator() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("paracefas_module", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }
    // 1. Maneja variantes anidadas (Ej: cuando un Stmt contiene un Expr)
    template<typename... Ts>
    llvm::Value* operator()(const std::variant<Ts...>& v) const {
        return std::visit(*this, v);
    }

    // 2. Maneja los RecursiveWrapper (Desenvuelve el tipo antes de procesar)
    template<typename T>
    llvm::Value* operator()(const RecursiveWrapper<T>& wrapper) const {
        return (*this)(wrapper.get()); 
    }

    // 3. El despachador principal
    llvm::Value* gen(const auto& node) const {
        return std::visit(*this, node);
    }

    // --- IMPLEMENTACIÓN DE TIPOS REALES ---

    llvm::Value* operator()(std::monostate) const { return nullptr; }

    llvm::Value* operator()(int i) const {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i, true);
    }

    llvm::Value* operator()(double d) const {
        return llvm::ConstantFP::get(*context, llvm::APFloat(d));
    }

    // El error de 'std::basic_string' se arregla asegurando que sea const&
    llvm::Value* operator()(const std::string& var_name) const {
        if (namedValues.contains(var_name)) return namedValues[var_name];
        return nullptr;
    }

    llvm::Value* operator()(const BinaryOp& b) const {
        llvm::Value* L = gen(b.lhs);
        llvm::Value* R = gen(b.rhs);
        if (!L || !R) return nullptr;
        
        switch (b.op) {
            case '+': return builder->CreateAdd(L, R, "addtmp");
            case '-': return builder->CreateSub(L, R, "subtmp");
            case '*': return builder->CreateMul(L, R, "multmp");
            case '/': return builder->CreateSDiv(L, R, "divtmp");
            default: return nullptr;
        }
    }

    llvm::Value* operator()(const ReturnStmt& r) const {
        llvm::Value* val = gen(r.value);
        if (val) return builder->CreateRet(val);
        return nullptr;
    }

    llvm::Value* operator()(const FunctionDef& f) const {
        // 1. Definir el tipo de la función (ejemplo: siempre devuelve i32 por ahora)
        llvm::FunctionType* funcType = llvm::FunctionType::get(builder->getInt32Ty(), false);
        
        // 2. Crear la función en el módulo
        llvm::Function* func = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, f.name, module.get());

        // 3. Crear el bloque de entrada
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "entry", func);
        builder->SetInsertPoint(bb);

        // 4. Generar el cuerpo de la función (el vector de Stmts)
        // Suponiendo que f.body es un std::vector<Stmt>
        for (const auto& stmt : f.body) {
            gen(stmt);
        }

        return func;
    }

    // Agrega Let, Const y FunctionCall siguiendo el mismo patrón...
    llvm::Value* operator()(const Let& l) const { return nullptr; }
    llvm::Value* operator()(const Const& c) const { return nullptr; }
    llvm::Value* operator()(const FunctionCall& fc) const { return nullptr; }

    // --- SOPORTE PARA VECTORES ---
    llvm::Value* operator()(const std::vector<Stmt>& nodes) const {
        llvm::Value* last = nullptr;
        for (const auto& n : nodes) last = gen(n);
        return last;
    }

    void dump(llvm::raw_ostream& os) {
        if (module) {
            module->print(os, nullptr);
        }
    }

private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    mutable std::map<std::string, llvm::Value*> namedValues;
};