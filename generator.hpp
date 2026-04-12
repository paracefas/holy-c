#pragma once

#include "./ast.hpp"

#include <variant>

struct Reg { std::string name; };
struct Imm { int value; };

using Operand = std::variant<Reg, Imm, std::string>;

struct Inst {
    std::string op;
    std::vector<Operand> args;
};

struct OperandPrinter {
    std::string operator()(const Reg& r) const { return r.name; }
    std::string operator()(const Imm& i) const { return std::to_string(i.value); }
    std::string operator()(const std::string& label) const { return label; }
};

std::string to_string(const Inst& i) {
    std::string s = "    " + i.op + " ";
    for (size_t idx = 0; idx < i.args.size(); ++idx) {
        std::string arg_str = std::visit(OperandPrinter{}, i.args[idx]);
        
        s += arg_str;
        
        if (idx < i.args.size() - 1) {
            s += ", ";
        }
    }
    return s + "\n";
}

using AsmStream = std::vector<Inst>;

struct FasmGenerator {
    AsmStream operator()(const RecursiveWrapper<ReturnStmt>& r) const {
        AsmStream code = std::visit(*this, r.get().value); 
        code.push_back({"call", {std::string("ExitProcess"), {}}});
        return code;
    }

    AsmStream operator()(const std::vector<Stmt>& program) const {
        AsmStream total_code;
        for (const auto& node : program) {
            auto code = std::visit(*this, node);
            total_code.insert(total_code.end(), code.begin(), code.end());
        }
        return total_code;
    }

    AsmStream operator()(const RecursiveWrapper<FunctionDef>& f) const {
        const auto& fn = f.get();
        AsmStream code;
        
        std::string label_name = (fn.name == "main") ? "start" : fn.name;

        code.push_back({"label", {label_name}});
        code.push_back({"sub",   {Reg{"rsp"}, Imm{40}}}); // Shadow space + align

        for (const auto& stmt : fn.body) {
            auto stmt_code = std::visit(*this, stmt);
            code.insert(code.end(), stmt_code.begin(), stmt_code.end());
        }
        
        return code;
    }

    AsmStream operator()(const Expr& e) const {
        return std::visit(*this, e);
    }

    AsmStream operator()(int i) const {
        return { {"mov", {Reg{"rcx"}, Imm{i}}} };
    }

    AsmStream operator()(double d) const {
        return { {"; literal double no soportado aun", {}} };
    }

    AsmStream operator()(const std::string& s) const {
        return { {"; literal string no soportado aun", {}} };
    }

    AsmStream operator()(const RecursiveWrapper<FunctionCall>& fc) const {
        return { {"; llamada a funcion no soportada aun", {}} };
    }
};

#include <sstream>

std::string finalize_fasm_code(const AsmStream& instructions) {
    std::stringstream ss;

    ss << "format PE64 CONSOLE\n";
    ss << "entry start\n";

    ss << "section '.text' code readable executable\n";
    
    for (const auto& inst : instructions) {
        if (inst.op == "label") {
            ss << std::visit(OperandPrinter{}, inst.args[0]) << ":\n";
        } 
        else if (inst.op == "call") {
            ss << "    call [" << std::visit(OperandPrinter{}, inst.args[0]);
            ss << "]\n";
        }
        else {
            ss << "    " << inst.op << " ";
            for (size_t i = 0; i < inst.args.size(); ++i) {
                ss << std::visit(OperandPrinter{}, inst.args[i]);
                if (i < inst.args.size() - 1) ss << ", ";
            }
            ss << "\n";
        }
    }

    ss << "\nsection '.idata' import data readable\n";

    ss << "\tdd 0,0,0, RVA kernel_name, RVA kernel_table\n";
    ss << "\tdd 0,0,0,0,0\n";
    ss << "\tkernel_table:\n";
    ss << "\t\tExitProcess dq RVA _ExitProcess\n";
    ss << "\t\tdq 0\n";
    ss << "\tkernel_name db 'KERNEL32.DLL',0\n";
    ss << "\t_ExitProcess dw 0\n";
    ss << "\t\tdb 'ExitProcess',0\n";

    return ss.str();
}