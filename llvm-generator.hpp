#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/TargetParser/Host.h>

#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>

#include <map>
#include <memory>

#include "ast.hpp"

class LLVMGenerator {
public:
    LLVMGenerator() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("paracefas_module", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
        module->setTargetTriple(llvm::Triple{llvm::sys::getProcessTriple()});
    }
  
    template<typename... Ts>
    llvm::Value* operator()(const std::variant<Ts...>& v) const {
        return std::visit(*this, v);
    }

    template<typename T>
    llvm::Value* operator()(const RecursiveWrapper<T>& wrapper) const {
        return (*this)(wrapper.get()); 
    }

    llvm::Value* gen(const auto& node) const {
        return std::visit(*this, node);
    }

    llvm::Value* operator()(std::monostate) const { return nullptr; }

    llvm::Value* operator()(int i) const {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), i, true);
    }

    llvm::Value* operator()(double d) const {
        return llvm::ConstantFP::get(*context, llvm::APFloat(d));
    }

    llvm::Value* operator()(const std::string& var_name) const {
        if (namedValues.contains(var_name)) {
            llvm::Value* val = namedValues[var_name];
            
            if (llvm::isa<llvm::AllocaInst>(val)) {
                return builder->CreateLoad(builder->getInt32Ty(), val, var_name);
            }
            return val;
        }
        std::println("Error: Variable {} no definida", var_name);
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

        namedValues.clear();

        // 4. Generar el cuerpo de la función (el vector de Stmts)
        // Suponiendo que f.body es un std::vector<Stmt>
        for (const auto& stmt : f.body) {
            gen(stmt);
        }

        return func;
    }

    llvm::Value* operator()(const Let& l) const { 
        llvm::Value* init_val = gen(l.value);
        if (!init_val) return nullptr;
        llvm::AllocaInst* alloca = builder->CreateAlloca(builder->getInt32Ty(), nullptr, l.id);
        
        builder->CreateStore(init_val, alloca);
        
        namedValues[l.id] = alloca;
        
        return init_val; 
    }
    llvm::Value* operator()(const Const& c) const { 
        llvm::Value* init_val = gen(c.value);
        llvm::AllocaInst* alloca = builder->CreateAlloca(builder->getInt32Ty(), nullptr, c.id);
        
        builder->CreateStore(init_val, alloca);
        
        namedValues[c.id] = alloca;
        
        return init_val;         
    }
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

    void emitObjectCode(std::string filename) {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        // 1. Crear el objeto Triple explícitamente
        llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
        
        std::string Error;
        auto Target = llvm::TargetRegistry::lookupTarget(targetTriple.getTriple(), targetTriple, Error);

        if (!Target) {
            std::println("Error buscando el target: {}", Error);
            return;
        }

        // 2. Configurar la máquina pasando el objeto Triple directamente
        auto CPU = "generic";
        auto Features = "";
        llvm::TargetOptions opt;
        
        // Aquí estaba el error: pasamos el objeto 'targetTriple' directamente, no el string
        auto TargetMachine = Target->createTargetMachine(
            targetTriple, 
            CPU, 
            Features, 
            opt, 
            llvm::Reloc::PIC_
        );

        module->setDataLayout(TargetMachine->createDataLayout());
        module->setTargetTriple(targetTriple); 

        // 3. Abrir archivo (.obj)
        std::error_code EC;
        // En versiones nuevas, si F_None falla, usa llvm::sys::fs::OF_None 
        // o simplemente 0 si quieres el default.
        llvm::raw_fd_ostream dest(filename, EC);

        if (EC) {
            std::println("Error al abrir archivo: {}", EC.message());
            return;
        }

        // 4. Emitir el archivo
        llvm::legacy::PassManager pass;
        auto FileType = llvm::CodeGenFileType::ObjectFile;

        if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
            std::println("El TargetMachine no puede emitir un archivo objeto");
            return;
        }

        pass.run(*module);
        dest.flush();
        std::println("Archivo objeto {} generado con éxito.", filename);
    }

private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    mutable std::map<std::string, llvm::Value*> namedValues;
};