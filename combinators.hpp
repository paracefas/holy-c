#pragma once

template<typename... Ts>
using Result = std::pair<std::optional<std::tuple<Ts...>>, std::string>;

template <typename T>
struct std::formatter<Result<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Result<T>& r, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "[{}, \"{}\"]", r.first, r.second);
    }
};

auto seq1(auto P1, auto P2) { 
    return [=] (std::string in) {
        auto [res, rest] = P1(in);
        
        // Obtenemos el tipo de los "Optionals" (lo que hay dentro de .first)
        using A = typename decltype(res)::value_type; 
        
        // Usamos invoke_result para saber qué devuelve P2 al pasarle un string
        using P2Result = std::invoke_result_t<decltype(P2), std::string>;
        using B = typename P2Result::first_type::value_type;

        using ABtuple = decltype(std::tuple_cat(std::declval<A>(), std::declval<B>()));
        using ReturnT = std::pair<std::optional<ABtuple>, std::string>;

        if (!res) return ReturnT{std::nullopt, in}; 

        auto [res2, rest2] = P2(rest);
        if (!res2) return ReturnT{std::nullopt, in}; 

        return ReturnT{std::tuple_cat(std::move(*res), std::move(*res2)), rest2};
    };
}

auto seq(auto P) { return P; }

auto seq(auto P1, auto... Ps) {
    return seq1(P1, seq(Ps...));
}

template<typename P, typename F>
auto map(P parser, F func) {
    return [=](std::string in) {
        auto [res, rest] = parser(in);
        using ReturnValueT = decltype(std::apply(func, *res));
        using ReturnResultT = Result<ReturnValueT>;

        if (!res) return ReturnResultT{std::nullopt, in};

        return ReturnResultT{ std::apply(func, *res), rest };
    };
}



template<typename... Ps>
auto choice(Ps... ps) {
    return [=](std::string in) {
        // Quitamos espacios primero (Lexer skip whitespace)
        std::string_view sv = in;
        sv.remove_prefix(std::min(sv.find_first_not_of(" \t\n\r"), sv.size()));
        std::string trimmed(sv);

        using ReturnType = std::invoke_result_t<
            typename std::tuple_element<0, std::tuple<Ps...>>::type, 
            std::string
        >;
        // Usamos un optional como "contenedor temporal" para el éxito
        std::optional<ReturnType> success;

        // Fold expression: Intentamos cada parser. 
        // Si uno tiene éxito, guardamos el resultado y paramos.
        ([&] {
            if (success) return true; // Si ya tenemos éxito, saltamos los demás
            
            auto temp = ps(in);
            if (temp.first.has_value()) {
                success.emplace(std::move(temp)); // Construcción directa por movimiento
                return true;
            }
            return false;
        }() || ...);

        // Si hubo éxito, lo movemos hacia afuera
        if (success) {
            return std::move(*success);
        }

        // Si no, devolvemos el error estándar (nada encontrado)
        return ReturnType{ std::nullopt, in };
    };
}

auto choice (auto P1) { return P1; }

template<typename P>
auto many(P parser) {
    return [parser](std::string in) {
        using R = std::invoke_result_t<P, std::string>;
        using TupleT = typename R::first_type::value_type;
        using T = std::tuple_element_t<0, TupleT>;

        std::vector<T> results;
        std::string current_in = in;

        while (true) {
            auto [res, rest] = parser(current_in);
            if (!res) break;
            
            results.push_back(std::get<0>(*res));
            
            if (current_in == rest) break; 
            current_in = rest;
        }
        return Result<std::vector<T>>{ std::tuple{results}, current_in };
    };
}