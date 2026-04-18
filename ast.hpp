#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <format>
#include <vector>
#include <memory>
#include <iostream>

#define TOKEN_LIST(X) \
    X(PAR_OPN, "PAR_OPN") \
	X(PAR_CLS, "PAR_CLS") \
	X(CURLY_OPN, "CURLY_OPN") \
	X(CURLY_CLS, "CURLY_CLS") \
	X(IDENTIFIER, "IDENTIFIER") \
	X(STR, "STR") \
	X(UNDEFINED, "UNDEFINED") \
	X(INT, "INT") \
    X(KEYWORD, "KEYWORD") \
    X(DOUBLE, "DOUBLE") \
    X(INFER, "INFER") \
    X(SYM, "Symbol") \
    X(COMMA, "COMMA") \
    X(SEMI, "SEMICOLON") \
    X(PLUS, "PLUS") \
    X(MINUS, "MINUS") \
    X(DIV, "DIV") \
    X(MULT, "MULT") \
    X(EQ, "EQ") \
    X(COLON, "COLON") \
    X(INT_TYPE, "INT_TYPE") \
    X(DOUBLE_TYPE, "DOUBLE_TYPE") \
    X(STR_TYPE, "STR_TYPE") \
    X(LET, "LET") \
    X(CONST, "CONST") \
    X(COLON_ASSIGN, "COLON_ASSIGN") \
	X(EoF, "End of File")

enum class token_t {
#define AS_ENUM(id, name) id,
    TOKEN_LIST(AS_ENUM)
#undef AS_ENUM
};

extern std::unordered_map<token_t, std::string> _token_n;

struct Token {
    std::string token_name;
    token_t token_type;
    std::variant<double, int, std::string> token_value;

    Token(token_t t, double v);
    Token(token_t t, int v);
    Token(token_t t, std::string v);

    Token& update(token_t t, auto v) {
        this->token_name = _token_n[t];
        this->token_type = t;
        this->token_value = v;
        return *this;
    }
};

template <>
struct std::formatter<Token> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Token& t, FormatContext& ctx) const {
        return std::visit([&](const auto& val) {
            return std::format_to(ctx.out(),
                "[{}, \"{}\"]",
                t.token_name,
                val
            );
        }, t.token_value);
    }
};

using Tokens = std::vector<Token>;

template<typename T>
class RecursiveWrapper {
    std::unique_ptr<T> ptr;
public:
    // Constructores (solo declaración)
    RecursiveWrapper(T&& val);
    RecursiveWrapper(const T& val);
    RecursiveWrapper(const RecursiveWrapper& other);
    RecursiveWrapper(RecursiveWrapper&&) = default;

    // Destructor (solo declaración - ESTO ES LO MÁS IMPORTANTE)
    ~RecursiveWrapper();

    // Operadores (pueden quedarse aquí porque no borran T)
    operator T&() { return *ptr; }
    operator const T&() const { return *ptr; }
    T& get() { return *ptr; }
    const T& get() const { return *ptr; }
    
    RecursiveWrapper& operator=(const RecursiveWrapper& other) {
        if (this != &other) ptr = std::make_unique<T>(*other.ptr);
        return *this;
    }
    RecursiveWrapper& operator=(RecursiveWrapper&&) = default;
};

struct FunctionCall;
struct ReturnStmt;
struct FunctionDef;
struct FunctionArg;
struct BinaryOp;
struct Let;
struct Const;

using Expr = std::variant<
    std::monostate,
    double,
    int, 
    std::string,
    RecursiveWrapper<FunctionCall>,
    RecursiveWrapper<ReturnStmt>,
    RecursiveWrapper<BinaryOp>,
    RecursiveWrapper<Let>,
    RecursiveWrapper<Const>
    >;

using Stmt = std::variant<
    std::monostate,
    RecursiveWrapper<FunctionDef>,
    RecursiveWrapper<ReturnStmt>,
    Expr
>;

template<typename T>
Stmt make_stmt(T&& val) {
    return Stmt(RecursiveWrapper<std::decay_t<T>>(std::forward<T>(val)));
}

struct ReturnStmt {
    Expr value;
};

struct FunctionCall {
    std::string name;
    std::vector<Expr> args;
};

struct FunctionDef {
    std::string name;
    std::vector<FunctionArg> signature;
    std::vector<Stmt> body;
};

struct FunctionArg {
    std::string name;
    token_t type;
    Expr value = std::monostate{};
};

struct BinaryOp {
    Expr lhs, rhs;
    char op;
};

struct Let {
    std::string id;
    token_t type;
    Expr value;
};

struct Const {
    std::string id;
    token_t type;
    const Expr value;
};

struct ASTPrinter {
    std::string operator()(const int i) const;
    std::string operator()(const double d) const;
    std::string operator()(const std::string& s) const;

    std::string operator()(const RecursiveWrapper<FunctionCall>& cp) const;
    std::string operator()(const RecursiveWrapper<ReturnStmt>& rp) const;
    std::string operator()(const RecursiveWrapper<FunctionDef>& fp) const;
    std::string operator()(const Expr& e) const;
};

// --- Al final de ast.hpp, después de que BinaryOp, Let, etc. estén definidos ---

template<typename T>
RecursiveWrapper<T>::RecursiveWrapper(T&& val) : ptr(std::make_unique<T>(std::move(val))) {}

template<typename T>
RecursiveWrapper<T>::RecursiveWrapper(const T& val) : ptr(std::make_unique<T>(val)) {}

template<typename T>
RecursiveWrapper<T>::RecursiveWrapper(const RecursiveWrapper& other) : ptr(std::make_unique<T>(*other.ptr)) {}

template<typename T>
RecursiveWrapper<T>::~RecursiveWrapper() = default; // Aquí ya sabe cómo borrar T