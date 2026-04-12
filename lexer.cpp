#include "lexer.hpp"

#include <regex>
#include <array>

static const std::array keywords { "return", "func", "int", "string", "double", "let", "const" }; 

bool is_keyword (std::string s) {
    for (const std::string keyword : keywords)
        if (keyword == s) return true;
    return false;
}

Result<Token> lexInt (std::string in) {
    std::regex re("^[0-9]+");
    std::smatch m;
    if (std::regex_search(in, m, re)) {
        std::string s = m.str();
        return { std::tuple{Token(token_t::INT, std::stoi(s))}, in.substr(s.size()) };
    }
    return {std::nullopt, in};
}

Result<Token> lexDouble (std::string in) {
    std::regex re(R"(^[0-9]*\.[0-9]+)");
    std::smatch m;
    if (std::regex_search(in, m, re)) {
        std::string s = m.str();
        return { std::tuple{Token(token_t::DOUBLE, std::stod(s))}, in.substr(s.size()) };
    }
    return {std::nullopt, in};
}



Result<Token> lexID (std::string in) {
    std::regex re("^[a-zA-Z_][a-zA-Z0-9_]*");
    std::smatch m;
    if (std::regex_search(in, m, re)) {
        std::string s = m.str();
        token_t type =  is_keyword(s) ? token_t::KEYWORD : token_t::IDENTIFIER;
        return { std::tuple{Token(type, s)}, in.substr(s.size()) };
    }
    return {std::nullopt, in};
}

Result<Token> lexSymbol (std::string in) {
    if (in.empty()) return {std::nullopt, in};
    char c = in[0];
    if (c == '(') return { std::tuple{Token(token_t::PAR_OPN, "(")}, in.substr(1) };
    if (c == ')') return { std::tuple{Token(token_t::PAR_CLS, ")")}, in.substr(1) };
    if (c == '{') return { std::tuple{Token(token_t::CURLY_OPN, "{")}, in.substr(1) };
    if (c == '}') return { std::tuple{Token(token_t::CURLY_CLS, "}")}, in.substr(1) };
    if (c == ',') return { std::tuple{Token(token_t::COMMA, ",")}, in.substr(1) };
    if (c == ';') return { std::tuple{Token(token_t::SEMI, ";")}, in.substr(1) };
    if (c == '+') return { std::tuple{Token(token_t::PLUS, "+")}, in.substr(1) };
    if (c == '-') return { std::tuple{Token(token_t::MINUS, "-")}, in.substr(1) };
    if (c == '*') return { std::tuple{Token(token_t::MULT, "*")}, in.substr(1) };
    if (c == '/') return { std::tuple{Token(token_t::DIV, "/")}, in.substr(1) };
    if (c == '=') return { std::tuple{Token(token_t::EQ, "=")}, in.substr(1) };
    if (c == ':') return { std::tuple{Token(token_t::COLON, ":")}, in.substr(1) };
    return {std::nullopt, in};
}

Result<Token> lexString (std::string in) {
    std::regex re("^\"([^\"]*)\""); // Busca algo entre comillas
    std::smatch m;
    if (std::regex_search(in, m, re)) {
        return { std::tuple{Token(token_t::STR, m.str(1))}, in.substr(m.str(0).size()) };
    }
    return {std::nullopt, in};
}

Result<Token> lexer (std::string in) {
    static auto p = choice(lexDouble, lexID, lexInt, lexSymbol, lexString);
    return p(in);
} 