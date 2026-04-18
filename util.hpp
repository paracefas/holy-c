#pragma once

#include <format>
#include <tuple>
#include <vector>
#include <expected>
#include <filesystem>
#include <string>

template <typename T>
struct std::formatter<std::optional<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::optional<T>& op, FormatContext& ctx) const {
        if (!op.has_value()) return std::format_to(ctx.out(), "[Nothing]");
		return std::format_to(ctx.out(), "[Just \"{}\"]", op.value());
    }
};

template <typename T>
struct std::formatter<std::vector<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::vector<T>& v, FormatContext& ctx) const {
        auto out = ctx.out();

        std::format_to(out, "[");

        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0)
                std::format_to(out, ", ");

            std::format_to(out, "{}", v[i]);
        }

        return std::format_to(out, "]");
    }
};

// Especialización de std::formatter para std::tuple
template<typename... Ts>
struct std::formatter<std::tuple<Ts...>> {
    
    // Define cómo parsear el formato (podemos dejarlo simple: "{}")
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    // Define cómo escribir la tupla en el output
    auto format(const std::tuple<Ts...>& t, format_context& ctx) const {
        std::format_to(ctx.out(), "(");
        format_tuple_elements(t, std::make_index_sequence<sizeof...(Ts)>{}, ctx);
        return std::format_to(ctx.out(), ")");
    }
};

enum class FileError { NotFound, ReadFailed };

std::expected<std::string, FileError> load_source(std::filesystem::path path);
#ifdef DEBUG_MODE
    #define PRINT(fmt_str, ...) std::print("[DEBUG {} ({})]: " fmt_str "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define PRINT(...) 
#endif
