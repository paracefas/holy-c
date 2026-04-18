#include "parser.hpp"

#include "util.hpp"
#include "lexer.hpp"

ParserFunc match (token_t expected) {
    return [=](std::string in) -> Result<Token> {
        std::string trimmed = in;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        
        if (trimmed.empty()) return {std::nullopt, in};
        auto [res, rest] = lexer(trimmed); 
        
        if (res && std::get<0>(*res).token_type == expected) {
            return {res, rest};
        }
        return {std::nullopt, in};
    };
}

ParserFunc keyword (std::string word) {
    return [word](std::string in) -> Result<Token> {
        Result<Token> res = match(token_t::KEYWORD)(in);
        if (!res.first.has_value())
            return {std::nullopt, in};

        std::string w = std::get<std::string>(std::get<0>(res.first.value()).token_value);
        if (w == word) 
            return res;

        return {std::nullopt, in};
    };
};

Result<int> parseInt (std::string in) { 
    static auto p = map(
        match(token_t::INT), 
        [](Token t) -> int {
            return std::get<int>(t.token_value); 
        }
    );

    return p(in);
}

Result<double> parseDouble (std::string in) { 
    static auto p = map(
        match(token_t::DOUBLE),
        [](Token t) -> double {
            return std::get<double>(t.token_value);
        } 
    );

    return p(in);
}

Result<std::string> parseStr (std::string in) {
    static auto p = map(
        match(token_t::STR),
        [](Token t) -> std::string {
            return std::get<std::string>(t.token_value);
        } 
    );

    return p(in);
}

Result<char> parsePlus (std::string in) {
    static auto p = map(
        match(token_t::PLUS),
        [] (Token) -> char  { return '+'; }
    );

    return p(in);
}

Result<char> parseMinus (std::string in) {
    static auto p = map(
        match(token_t::MINUS),
        [] (Token) -> char  { return '-'; }
    );

    return p(in);
}

Result<char> parseMult (std::string in) {
    static auto p = map(
        match(token_t::MULT),
        [] (Token) -> char  { return '*'; }
    );

    return p(in);
}

Result<char> parseDiv (std::string in) {
    static auto p = map(
        match(token_t::DIV),
        [] (Token) -> char  { return '/'; }
    );

    return p(in);
}

Result<char> parseOp (std::string in) {
    static auto p = choice(parsePlus, parseMinus, parseMult, parseDiv);

    return p(in);
} 

// Result<FunctionArg> parseSingleArgCall (std::string in) {
//     static auto p = 
//     return p(in);
// }

Result<std::vector<Expr>> parseArgsCall (std::string in) {
    static auto p = many(seq(parseExpr, parseComma));
    return p(in);
}

// f(...)
Result<Expr> parseFuncCall (std::string in) {
    static auto p = map(
        seq(
            parseID,
            parseParOpn,
            parseArgsCall,
            parseParCls
        ),
        [] (std::string id, std::monostate, std::vector<Expr> args, std::monostate) -> Expr {
            PRINT("Parsing function calling... {}", id);
            return FunctionCall{id, args};
        }
    );
    return p(in);
}

Result<Expr> parseBinaryExpr (std::string in);

Result<Expr> parsePrimaryExpr(std::string in) {
    static auto p = choice(
        parseFuncCall, 
        map(parseDouble, [](double d) -> Expr { return d; }),
        map(parseInt, [](int i) -> Expr { return i; }),
        map(parseStr, [](std::string s) -> Expr { return s; })
    );
    return p(in);
}

Result<Expr> parseBinaryExpr (std::string in) {
    static auto p = map(
        seq(
            parsePrimaryExpr, 
            parseOp, 
            parseExpr
        ),
        [] (Expr lhs, char op, Expr rhs) -> Expr {
            return BinaryOp{lhs, rhs, op}; 
        }
    );
    return p(in);
}

Result<Expr> parseExpr (std::string in) {
    static auto p = choice(parseBinaryExpr, parsePrimaryExpr);
    return p(in);
}

/**
 * (Expr) + (Expr)
 * (Expr)
 */


Result<std::string> parseID (std::string in) { 
    static auto p = map(
        match(token_t::IDENTIFIER),
        [](Token t) -> std::string {
            return std::get<std::string>(t.token_value);
        }
    );
    return p(in);
}

Result<std::monostate> parseCurlyOpn (std::string in) {
    static auto p = map(
    match(token_t::CURLY_OPN),
    [](Token) -> std::monostate { return {}; });
    return p(in);
}
Result<std::monostate> parseCurlyCls (std::string in) {
    static auto p = map(
    match(token_t::CURLY_CLS),
    [](Token) -> std::monostate { return {}; });
    return p(in);
}

Result<std::monostate> parseParOpn (std::string in) {
    static auto p = map(
    match(token_t::PAR_OPN),
    [](Token) -> std::monostate { return {}; });
    return p(in);
}
Result<std::monostate> parseParCls (std::string in) {
    static auto p = map(
    match(token_t::PAR_CLS),
    [](Token) -> std::monostate { return {}; });
    return p(in);
}

Result<Stmt> parseReturn (std::string in) {
    static auto p =  map(
        seq(keyword("return"), parseExpr),
        [](Token, auto val) -> Stmt {
            Expr valr = val;
            //PRINT("Parsing return {} '{}' {}", lhs, op, rhs);
            return make_stmt(ReturnStmt{ valr });
        }
    );
    return p(in);
}

Result<std::monostate> parseComma (std::string in) { 
    static auto p = map(
        match(token_t::COMMA),
        [](Token) -> std::monostate { return {}; }
    );
    return p(in);
}

Result<FunctionArg> parseSingleArg (std::string in) {
    static auto p = map(
        seq(match(token_t::KEYWORD), parseID),
        [](Token type_tok, std::string name) -> FunctionArg {
            std::string type_name = std::get<std::string>(type_tok.token_value);
            
            token_t t;
            if (type_name == "int") t = token_t::INT;
            else if (type_name == "double") t = token_t::DOUBLE;
            else if (type_name == "string") t = token_t::STR;
            else throw std::runtime_error("Tipo desconocido: " + type_name);

            return FunctionArg{ name, t };
        } 
    );

    return p(in);
}

Result<std::vector<FunctionArg>> parseArgs (std::string in) {
    std::vector<FunctionArg> args;
    auto res1 = parseSingleArg(in);
    if (!res1.first) {
        return { std::optional<std::tuple<std::vector<FunctionArg>>>{std::vector<FunctionArg>{}}, in };
    }

    auto& first_data = *res1.first; 
    args.push_back(std::get<0>(first_data));
    std::string current_rest = res1.second;

    static auto nextArgsParser = many(
        map(
            seq(parseComma, parseSingleArg), 
            [](std::monostate, FunctionArg a) { return a; }
        )
    );

    auto res2 = nextArgsParser(current_rest);
    
    if (res2.first) {
        auto& others_vec = std::get<0>(*res2.first);
        for (auto& a : others_vec) {
            args.push_back(std::move(a));
        }
        return { std::optional<std::tuple<std::vector<FunctionArg>>>{args}, res2.second };
    }

    return { std::optional<std::tuple<std::vector<FunctionArg>>>{args}, current_rest };
};

Result<char> parseAssignation (std::string in) {
    static auto p = map(
        match(token_t::EQ),
        [] (Token) -> char {
            return '=';
        }
    );

    return p(in);
}

Result<char> parseColon (std::string in) {
    static auto p = map(
        match(token_t::COLON),
        [] (Token) -> char {
            return ':';
        }
    );

    return p(in);
}

Result<token_t> parseType (std::string in) {
    static auto p = choice(
        map(keyword("int"), [](Token) -> token_t { return token_t::INT_TYPE; }),
        map(keyword("double"), [](Token) -> token_t { return token_t::DOUBLE_TYPE; }),
        map(keyword("string"), [](Token) -> token_t { return token_t::STR_TYPE; })
    );
    return p(in);
}

Result<Stmt> parseLet (std::string in) {
    static auto p = map(
        seq(
            keyword("let"),
            parseID,
            parseColon,
            parseType,
            parseAssignation,
            parseExpr
        ),
        [] (Token, std::string id, char, token_t t, char, Expr expr) -> Stmt {
            return make_stmt(Expr{Let{id, t, expr}});
        }
    );

    return p(in);
}

Result<std::string> parseColonAssign (std::string in) {
    static auto p = map(
        match(token_t::COLON_ASSIGN),
        [] (Token) -> std::string { return ":="; }
    );

    return p(in);
}

// Parsing x := 3 <=> const x : int = 3
Result<Stmt> parseConst1 (std::string in) {
    static auto p = map(
        seq(
            parseID,
            parseColonAssign,
            parseExpr
        ),
        [] (std::string id, std::string, Expr expr) -> Stmt {
            return make_stmt(Expr{Const{id, token_t::INFER, expr}});
        }
    );

    return p(in);
}

Result<Stmt> parseConst0 (std::string in) {
    static auto p = map(
        seq(
            keyword("const"),
            parseID,
            parseColon,
            parseType,
            parseAssignation,
            parseExpr
        ),
        [] (Token, std::string id, char, token_t t, char, Expr expr) -> Stmt {
            return make_stmt(Expr{Const{id, t, expr}});
        }
    );

    return p(in);
}

Result<Stmt> parseConst (std::string in) {
    static auto p = choice(parseConst0, parseConst1);
    return p(in);
}

Result<Stmt> parseStmt (std::string in) {
    static auto p = choice(parseLet, parseConst, parseReturn, parseFuncCall);
    return p(in);
}

Result<std::vector<Stmt>> parseBlock (std::string in) {
    static auto p = map(
        seq(
            parseCurlyOpn,
            many(parseStmt),
            parseCurlyCls
        ),
        [] (std::monostate, std::vector<Stmt> stmts, std::monostate) -> std::vector<Stmt> {
            return stmts;
        }
    );

    return p(in);
}

Result<Stmt> parseFunc1 (std::string in) {
    static auto p = map(
        seq(
            keyword("func"),
            parseID,
            parseAssignation,
            parseExpr
        ),
        [] (Token, std::string id, char, Expr ret) -> Stmt {
            ReturnStmt rs {ret};
            RecursiveWrapper<ReturnStmt> wrs{rs};
            std::vector<Stmt> body;
            body.push_back(Stmt{wrs});
            return make_stmt(FunctionDef{id, {}, body});
        }
    );

    return p(in);
}

Result<Stmt> parseFunc2 (std::string in) {
    static auto p = map(
        seq(
            keyword("func"), // func
            parseID, // function name
            parseParOpn, // (
            parseArgs,
            parseParCls, // )
            parseBlock
        ),
        [](Token, std::string id, std::monostate, std::vector<FunctionArg> args, std::monostate, std::vector<Stmt> body) -> Stmt {
            PRINT("arg(0): {}, typeof(arg(0)): {}", args[0].name, _token_n[args[0].type]);
            return make_stmt(FunctionDef{id, std::move(args), std::move(body)});
        }
    );

    return p(in);
}

Result<Stmt> parseFunc (std::string in) {
    static auto p = choice(parseFunc2, parseFunc1);
    return p(in);
}

Result<std::vector<Stmt>> parse (std::string in) {
    static auto p = many(choice(parseLet, parseConst, parseFunc, parseReturn));
    return p(in);
}