#include "ast.hpp"

#include <unordered_map>

std::unordered_map<token_t, std::string> _token_n = {
#define AS_MAP(id, name) { token_t::id, "token name: " name },
    TOKEN_LIST(AS_MAP)
#undef AS_MAP
};

Token::Token(token_t t, double v) 
    : token_type{t}, token_name{_token_n[t]}, token_value{v} {}

Token::Token(token_t t, int v) 
    : token_type{t}, token_name{_token_n[t]}, token_value{v} {}

Token::Token(token_t t, std::string v) 
    : token_type{t}, token_name{_token_n[t]}, token_value{v} {}

std::string ASTPrinter::operator()(const int i) const { return std::to_string(i); }
std::string ASTPrinter::operator()(const double d) const { return std::format("{:.2f}", d); }
std::string ASTPrinter::operator()(const std::string& s) const { return std::format("\"{}\"", s); }

std::string ASTPrinter::operator()(const RecursiveWrapper<FunctionCall>& cp) const {
    const auto& call = cp.get();
    std::string args;
    for (size_t i = 0; i < call.args.size(); ++i) {
        args += std::visit(*this, call.args[i]);
        if (i < call.args.size() - 1) args += ", ";
    }
    return std::format("{}({})", call.name, args);
}

std::string ASTPrinter::operator()(const RecursiveWrapper<ReturnStmt>& rp) const {
    return std::format("return {}", std::visit(*this, rp.get().value));
}

std::string ASTPrinter::operator()(const RecursiveWrapper<FunctionDef>& fp) const {
    const auto& fn = fp.get();
    
    // Formatear firma: (int x, double y)
    std::string sig_str;
    for (size_t i = 0; i < fn.signature.size(); ++i) {
        std::string type_name = _token_n[fn.signature[i].type];
        sig_str += std::format("{} {}", type_name, fn.signature[i].name);
        if (i < fn.signature.size() - 1) sig_str += ", ";
    }
    std::string body;
    for (const auto& s : fn.body) body += std::format("\n  {};", std::visit(*this, s));
    return std::format("func {}({}) {{{}\n}}", fn.name, sig_str, body);
}

// Para cuando una sentencia es solo una expresión (ej: una llamada a función suelta)
std::string ASTPrinter::operator()(const Expr& e) const {
    return std::visit(*this, e);
}

template<>
struct std::formatter<FunctionCall> : std::formatter<std::string> {
    auto format(const FunctionCall& fc, format_context& ctx) const {
        string s = ASTPrinter{}(RecursiveWrapper{fc});
        return formatter<string>::format(s, ctx);
    }
};

// Formatter para el variant Stmt (El más importante)
template<>
struct std::formatter<Stmt> : std::formatter<std::string> {
    auto format(const Stmt& stmt, format_context& ctx) const {
        string s = std::visit(ASTPrinter{}, stmt);
        return std::formatter<std::string>::format(s, ctx);
    }
};

// Si tienes FunctionDef como struct independiente
template<>
struct std::formatter<FunctionDef> : std::formatter<std::string> {
    auto format(const FunctionDef& fd, format_context& ctx) const {
        string s = ASTPrinter{}(RecursiveWrapper{fd});
        return std::formatter<std::string>::format(s, ctx);
    }
};