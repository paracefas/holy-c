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
        auto [res, rest] = lexer(trimmed); 
        
        if (res && std::get<0>(*res).token_type == expected) {
            return {res, rest};
        }
        return {std::nullopt, in};
    };
}

auto keyword = [](std::string word) {
    return [word](std::string in) -> Result<Token> {
        auto res = match(token_t::KEYWORD)(in);
        if (!res.first.has_value())
            return {std::nullopt, in};

        auto w = std::get<std::string>(std::get<0>(res.first.value()).token_value);
        if (w == word) 
            return res;

        return {std::nullopt, in};
    };
};

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

auto parsePlus = map(
    match(token_t::PLUS),
    [] (Token t) -> char  { return '+'; }
);

auto parseMinus = map(
    match(token_t::MINUS),
    [] (Token t) -> char  { return '-'; }
);

auto parseMult = map(
    match(token_t::MULT),
    [] (Token t) -> char  { return '*'; }
);

auto parseDiv = map(
    match(token_t::DIV),
    [] (Token t) -> char  { return '/'; }
);

auto parseOp = choice(parsePlus, parseMinus, parseMult, parseDiv);

auto parseBinaryOp = map(
    seq(
        match(token_t::INT),
        parseOp,
        match(token_t::INT)
    ),
    [](Token lhs, char op, Token rhs) -> Expr {
        std::println("Parsing binop");
        Expr a = std::get<int>(lhs.token_value);
        Expr b = std::get<int>(rhs.token_value);
        std::println("\t{} {} {}", std::get<int>(a), op, std::get<int>(b));
        return RecursiveWrapper<BinaryOp>{BinaryOp{a, b, op}};
    }
);

auto parseExpr = choice(
    parseBinaryOp,
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
    seq(keyword("return"), parseExpr),
    [](Token kw, auto val) -> Stmt {
        // 1. 'val' ya es de tipo Expr (gracias al retorno de parseExpr)
        Expr valr = val;

        // 2. Comprobación segura: ¿Es realmente una BinaryOp?
        // Usamos std::get_if para evitar que el programa explote si el return no es una suma
        if (auto* wrapper = std::get_if<RecursiveWrapper<BinaryOp>>(&valr)) {
            const BinaryOp& op = wrapper->get();
            
            // 3. lhs también es una Expr, hay que sacar el int de ella
            if (auto* l_val = std::get_if<int>(&op.lhs)) {
                std::println("Parsing return con primer operando: {}", *l_val);
            } else {
                std::println("Parsing return con operando izquierdo complejo.");
            }
        } else if (auto* i_val = std::get_if<int>(&valr)) {
            std::println("Parsing return simple: {}", *i_val);
        }

        return make_stmt(ReturnStmt{ valr });
    }
);

auto parseComma = map(
    match(token_t::COMMA),
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
    std::vector<FunctionArg> args;
    
    // 1. Intentar el primer argumento
    auto res1 = parseSingleArg(in);
    
    // Accedemos a .first para ver si el optional tiene valor
    if (!res1.first) {
        // Si no hay argumentos, devolvemos el vector vacío y el input original
        return { std::optional<std::tuple<std::vector<FunctionArg>>>{std::vector<FunctionArg>{}}, in };
    }

    // Si tuvo éxito, extraemos la data y el nuevo resto (res1.second)
    auto& first_data = *res1.first; 
    args.push_back(std::get<0>(first_data));
    std::string current_rest = res1.second;

    // 2. Definir el parser para los argumentos adicionales (, int x)
    auto nextArgsParser = many(
        map(
            seq(parseComma, parseSingleArg), 
            [](std::monostate, FunctionArg a) { return a; }
        )
    );

    // 3. Ejecutar el segundo parser sobre el resto actual
    auto res2 = nextArgsParser(current_rest);
    
    if (res2.first) {
        auto& others_vec = std::get<0>(*res2.first);
        for (auto& a : others_vec) {
            args.push_back(std::move(a));
        }
        // Retornamos el vector completo y el resto final tras la lista de argumentos
        return { std::optional<std::tuple<std::vector<FunctionArg>>>{args}, res2.second };
    }

    // Si no hay más argumentos, devolvemos lo que tenemos y el resto tras el primer arg
    return { std::optional<std::tuple<std::vector<FunctionArg>>>{args}, current_rest };
};

auto parseAssignation = map(
    match(token_t::EQ),
    [] (Token) -> char {
        return '=';
    }
);

auto parseFunc1 = map(
    seq(
        keyword("func"),
        parseID,
        parseAssignation,
        parseExpr
    ),
    [] (Token, std::string id, char, Expr ret) -> Stmt {
        std::println("Parsing {}", id);
        return make_stmt(FunctionDef{id, {}, {Stmt{ret}}});
    }
);

auto parseFunc2 = map(
    seq(
        keyword("func"), // func
        parseID, // function name
        parseParOpn, // (
        parseArgs,
        parseParCls, // )
        parseCurlyOpn, // {
        parseReturn, // return x
        parseCurlyCls // }
    ),
    [](Token kw, std::string id, std::monostate, std::vector<FunctionArg> args, std::monostate, std::monostate, Stmt s, std::monostate) -> Stmt {
        std::println("Parsing func");
        std::vector<Stmt> body;
        body.push_back(std::move(s));
        
        return make_stmt(FunctionDef{id, std::move(args), std::move(body)});
    }
);

auto parseFunc = choice(parseFunc2, parseFunc1);

auto parse = many(choice(parseFunc, parseReturn));