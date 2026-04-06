#pragma once
// --- parsers de ejemplo ---
auto parse_token(std::string input, const std::string& literal, const token_t& t) -> Result<Token> {
    if (!input.starts_with(literal)) return {std::nullopt, input};
    return {Token{t, literal}, input.substr(literal.size())};
}

auto parse_par_opn(std::string in) { return parse_token(in, "(", token_t::PAR_OPN); }
auto parse_hello(std::string in) { return parse_token(in, "Hello", token_t::STR); }
auto parse_par_cls(std::string in) { return parse_token(in, ")", token_t::PAR_CLS); }

auto match (token_t expected) {
    return [=](std::string in) -> Result<Token> {
        std::string trimmed = in;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
        
        if (trimmed.empty()) return {std::nullopt, in};
        auto [res, rest] = lexer(trimmed); // El lexer funcional hace el trabajo sucio
        
        if (res && std::get<0>(*res).token_type == expected) {
            return {res, rest};
        }
        return {std::nullopt, in};
    };
}

auto parseInt = map(
    match(token_t::INT), 
    [](Token t) -> int {
        return std::get<int>(t.token_value); 
    }
);

auto parseDouble = map(
    match(token_t::DOUBLE),
    [](Token t) -> double {
        return std::get<double>(t.token_value);
    } 
);

auto parseStr = map(
    match(token_t::STR),
    [](Token t) -> std::string {
        return std::get<std::string>(t.token_value);
    } 
);

auto parseExpr = choice(
    map(parseDouble, [](double d) -> Expr {return d;}),
    map(parseInt, [](int i) -> Expr {return i;}),
    map(parseStr, [](std::string s) -> Expr {return s;})
);

auto parseID = map(
    match(token_t::IDENTIFIER),
    [](Token t) -> std::string {
        return std::get<std::string>(t.token_value);
    }
);

auto parseCurlyOpn = map(
    match(token_t::CURLY_OPN),
    [](Token) -> std::monostate { return {}; });
auto parseCurlyCls = map(
    match(token_t::CURLY_CLS),
    [](Token) -> std::monostate { return {}; });

auto parseParOpn = map(
    match(token_t::PAR_OPN),
    [](Token) -> std::monostate { return {}; });
auto parseParCls = map(
    match(token_t::PAR_CLS),
    [](Token) -> std::monostate { return {}; });

auto parseReturn = map(
    seq(match(token_t::KEYWORD), parseExpr),
    [](Token kw, auto val) -> Stmt {
        if (std::get<std::string>(kw.token_value) == "return") {
            Expr valr = val;
            return make_stmt(ReturnStmt{ valr }); // Se envuelve en RecursiveWrapper automáticamente
        }
        throw std::runtime_error("Se esperaba 'return' pero se encontró otra cosa");
    }
);

auto parseComma = map(
    match(token_t::SYM),
    [](Token t) -> std::monostate { return {}; }
);

auto parseSingleArg = map(
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

auto parseArgs = [](std::string in) -> Result<std::vector<FunctionArg>> {
    auto [first, rest1] = parseSingleArg(in);
    if (!first) return { std::vector<FunctionArg>{}, in }; 
    std::vector<FunctionArg> args;
    args.push_back(std::get<0>(*first));

    auto nextArgsParser = many(map(seq(parseComma, parseSingleArg), [](auto, FunctionArg a) { return a; }));
    auto [others, rest2] = nextArgsParser(rest1);
    
    if (others) {
        for (auto& a : std::get<0>(*others)) args.push_back(std::move(a));
    }

    return { std::tuple{args}, rest2 };
};

auto parseFunc = map(
    seq(
        match(token_t::KEYWORD), // func
        parseID, // function name
        parseParOpn, // (
        parseArgs,
        parseParCls, // )
        parseCurlyOpn, // {
        parseReturn, // return x
        parseCurlyCls // }
    ),
    [](Token kw, std::string id, std::monostate, std::vector<FunctionArg> args, std::monostate, std::monostate, Stmt s, std::monostate) -> Stmt {
        if (std::get<std::string>(kw.token_value) != "func") throw std::runtime_error("Se esperaba 'func' pero se encontró otra cosa");
        
        std::vector<Stmt> body;
        body.push_back(std::move(s));
        
        return make_stmt(FunctionDef{id, std::move(args), std::move(body)});
    }
);