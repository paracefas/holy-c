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

static const std::vector<Operand> X64_REGS = {Reg{"rax"}, Reg{"rcx"}, Reg{"rdx"}, Reg{"r8"}, Reg{"r9"}, Reg{"r10"}, Reg{"r11"}};

struct FasmGenerator {
    AsmStream operator()(const RecursiveWrapper<ReturnStmt>& r, size_t reg_idx) const {
        AsmStream code = gen(r.get().value, reg_idx); 
        if(reg_idx) // if it's not rax, move to rax
            code.push_back({"mov", {X64_REGS[0], X64_REGS[reg_idx]}});
        return code;
    }

    AsmStream operator()(const std::vector<Stmt>& program) const {
        AsmStream total_code;
        for (const auto& node : program) {
            auto code = gen(node, 0);
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
            auto stmt_code = gen(stmt, 0);
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

    AsmStream operator()(const RecursiveWrapper<BinaryOp>& rb, size_t reg_idx) const {
        const BinaryOp& b = rb.get();
        AsmStream ret, lhs, rhs;
        // Rax, Imm (lhs)
        lhs = gen(b.lhs, reg_idx);
        
        ret.insert(ret.end(), lhs.begin(), lhs.end());

        rhs = gen(b.rhs, reg_idx+1);

        ret.insert(ret.end(), rhs.begin(), rhs.end());
        
        Operand target = X64_REGS[reg_idx];
        Operand source = X64_REGS[reg_idx+1];

        if (b.op == '+') 
            ret.push_back({"add", {target, source}});
        else if (b.op == '-')
            ret.push_back({"sub", {target, source}});
        else if (b.op == '/')
            ret.push_back({"div", {target, source}});
        else if (b.op == '*')
            ret.push_back({"imul", {target, source}});

        return ret;
    }

    AsmStream operator()(const Expr& e) const {
        return std::visit(*this, e);
    }

    AsmStream operator()(int i, size_t reg_idx) const {
        return { {"mov", {X64_REGS[reg_idx], Imm{i}}} };
    }

    AsmStream operator()(double d) const {
        return { {"; literal double no soportado aun", {}} };
    }

    AsmStream operator()(const std::string& s) const {
        return { {"; literal string no soportado aun", {}} };
    }
private:
    AsmStream gen (const auto& node, size_t reg_idx) const {
        return std::visit([&](const auto& n) {
            if constexpr (requires { (*this)(n, reg_idx); })
                return (*this)(n, reg_idx);
            return (*this)(n);
        }, node);
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