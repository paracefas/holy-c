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

const std::vector<Operand> X64_REGS = {Reg{"rcx"}, Reg{"rdx"}, Reg{"r8"}, Reg{"r9"}};

struct FasmGenerator {
    AsmStream operator()(const RecursiveWrapper<ReturnStmt>& r) const {
        AsmStream code = std::visit(*this, r.get().value); 
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
        
        if (fn.name == "main") {
            code.push_back({"label", {std::string("start")}});
            code.push_back({"sub", {Reg{"rsp"}, Imm{40}}}); 
        } else {
            code.push_back({"label", {fn.name}});
        }

        for (const auto& stmt : fn.body) {
            auto stmt_code = std::visit(*this, stmt);
            code.insert(code.end(), stmt_code.begin(), stmt_code.end());
        }

        if (fn.name == "main") code.push_back({"invoke", {std::string("ExitProcess"), Reg{"rax"}}});
        else code.push_back({"ret", {}});
        
        return code;
    }

    AsmStream operator()(const RecursiveWrapper<FunctionCall>& fc) const {
        AsmStream code;
        const auto& call = fc.get();

        for (size_t i = 0; i < call.args.size(); ++i) {
            AsmStream arg_expr = std::visit(*this, call.args[i]);
            code.insert(code.end(), arg_expr.begin(), arg_expr.end());

            if (i < X64_REGS.size()) code.push_back({"mov", {X64_REGS[i], "rax"}});
            else code.push_back({"push", {"rax"}}); 
        }

        std::vector<Operand> invoke_args;
        invoke_args.push_back(call.name);
        
        for (size_t i = 0; i < call.args.size() && i < X64_REGS.size(); ++i)
            invoke_args.push_back(X64_REGS[i]);

        if (call.name == "ExitProcess") code.push_back(Inst{"invoke", invoke_args});
        else code.push_back(Inst{"call", invoke_args});
        
        return code;
    }

    AsmStream operator()(const Expr& e) const {
        return std::visit(*this, e);
    }

    AsmStream operator()(int i) const {
        return { {"mov", {Reg{"rax"}, Imm{i}}} };
    }

    AsmStream operator()(double d) const {
        return { {"; literal double no soportado aun", {}} };
    }

    AsmStream operator()(const std::string& s) const {
        return { {"; literal string no soportado aun", {}} };
    }
};

#include <sstream>

std::string finalize_fasm_code(const AsmStream& instructions) {
    std::stringstream ss;

    ss << "format PE64 CONSOLE\n";
    ss << "include 'win64a.inc'\n";
    ss << "entry start\n";

    ss << "section '.text' code readable executable\n";
    
    for (const auto& inst : instructions) {
        if (inst.op == "label") {
            ss << std::visit(OperandPrinter{}, inst.args[0]) << ":\n";
        } 
        else if (inst.op == "invoke") {
            ss << "    " << inst.op << " ";
            for (size_t i = 0; i < inst.args.size(); ++i) {
                ss << std::visit(OperandPrinter{}, inst.args[i]);
                if (i < inst.args.size() - 1) ss << ", ";
            }
            ss << '\n';
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
    ss << "library kernel32,'KERNEL32.DLL'\n";
    ss << "import kernel32,ExitProcess,'ExitProcess'\n";

    return ss.str();
}