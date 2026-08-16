#pragma once

#include <filesystem>
#include <fstream>
#include <meta>
#include <ranges>

namespace lox::utils {
    namespace fs = std::filesystem;

    inline std::ifstream open(
        const fs::path& path,
        std::ios_base::openmode mode = std::ios_base::in) {
        std::ifstream file{path, mode};
        if (!file.is_open()) {
            std::string err{"Unable to open file "};
            err.append(path);
            throw std::runtime_error(err);
        }
        file.exceptions(std::ios::badbit);
        return file;
    }

    inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::ranges::find_if(s, [](unsigned char c) {
                    return !std::isspace(c);
                }));
    }

    inline void rtrim(std::string& s) {
        s.erase(std::ranges::find_if(s | std::views::reverse,
                                     [](unsigned char c) {
                                         return !std::isspace(c);
                                     })
                    .base(),
                s.end());
    }

    inline void trim(std::string& s) {
        ltrim(s);
        rtrim(s);
    }

    template <typename T>
        requires std::is_enum_v<T>
    constexpr std::string enumToString(T v) {
        std::string res{"<unamed>"};
        template for (constexpr auto e :
                      define_static_array(std::meta::enumerators_of(^^T))) {
            if (v == [:e:]) {
                res = std::meta::identifier_of(e);
            }
        }
        return res;
    }

    template <class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };
    template <class... Ts>
    overload(Ts...) -> overload<Ts...>;
}
