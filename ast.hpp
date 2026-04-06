#pragma once

#include <print>
#include <unordered_map>

#define TOKEN_LIST(X) \
    X(PAR_OPN, "PAR_OPN") \
	X(PAR_CLS, "PAR_CLS") \
	X(CURLY_OPN, "CURLY_OPN") \
	X(CURLY_CLS, "CURLY_CLS") \
	X(IDENTIFIER, "IDENTIFIER") \
	X(PLUS_OP, "PLUS_OP") \
	X(MINS_OP, "MINS_OP") \
	X(MULT_OP, "MULT_OP") \
	X(DIVI_OP, "DIVI_OP") \
	X(STR, "STR") \
	X(UNDEFINED, "UNDEFINED") \
	X(INT, "INT") \
    X(KEYWORD, "KEYWORD") \
    X(DOUBLE, "DOUBLE") \
    X(SYM, "Symbol") \
	X(EoF, "End of File")

enum class token_t {
#define AS_ENUM(id, name) id,
    TOKEN_LIST(AS_ENUM)
#undef AS_ENUM
};

// [ token_t::PAR_OP ] == "Token name: PAR_OP"
auto _token_n = std::unordered_map<token_t, std::string> {
#define AS_MAP(id, name) { token_t::id, "token name: " name },
    TOKEN_LIST(AS_MAP)
#undef AS_MAP
};

struct Token {
	std::string token_name = "undefined";
	token_t token_type = token_t::UNDEFINED;
	std::variant<double, int, std::string> token_value;
	Token (token_t t, double v) : token_type {t}, token_name { _token_n[t] }, token_value{v} {}
    Token (token_t t, int v) : token_type {t}, token_name { _token_n[t] }, token_value{v} {}
	Token (token_t t, std::string v) : token_type {t}, token_name { _token_n[t] }, token_value{v} {}

	Token update (token_t t, auto v) {
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
    RecursiveWrapper(T&& val) : ptr(std::make_unique<T>(std::move(val))) {}
    RecursiveWrapper(const T& val) : ptr(std::make_unique<T>(val)) {}
    
    RecursiveWrapper(const RecursiveWrapper& other) : ptr(std::make_unique<T>(*other.ptr)) {}
    RecursiveWrapper(RecursiveWrapper&&) = default;

    operator T&() { return *ptr; }
    operator const T&() const { return *ptr; }
    T& get() { return *ptr; }
    const T& get() const { return *ptr; }
};

struct FunctionCall;
struct ReturnStmt;
struct FunctionDef;
struct FunctionArg;

using Expr = std::variant<
    double,
    int, 
    std::string,
    RecursiveWrapper<FunctionCall>,
    RecursiveWrapper<ReturnStmt>
    >;

using Stmt = std::variant<
    RecursiveWrapper<ReturnStmt>,
    RecursiveWrapper<FunctionDef>,
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
    std::vector<Stmt> args;
};

struct FunctionDef {
    std::string name;
    std::vector<FunctionArg> signature;
    std::vector<Stmt> body;
};

struct FunctionArg {
    std::string name;
    token_t type;
};

struct ASTPrinter {
    std::string operator()(const int i) const { return std::to_string(i); }
    std::string operator()(const double d) const { return std::format("{:.2f}", d); }
    std::string operator()(const std::string& s) const { return std::format("\"{}\"", s); }

    std::string operator()(const RecursiveWrapper<FunctionCall>& cp) const {
        const auto& call = cp.get();
        std::string args;
        for (size_t i = 0; i < call.args.size(); ++i) {
            args += std::visit(*this, call.args[i]);
            if (i < call.args.size() - 1) args += ", ";
        }
        return std::format("{}({})", call.name, args);
    }

    std::string operator()(const RecursiveWrapper<ReturnStmt>& rp) const {
        return std::format("return {}", std::visit(*this, rp.get().value));
    }

    std::string operator()(const RecursiveWrapper<FunctionDef>& fp) const {
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
    std::string operator()(const Expr& e) const {
        return std::visit(*this, e);
    }
};

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