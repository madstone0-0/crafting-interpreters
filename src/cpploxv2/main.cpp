#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <exception>
#include <iostream>
#include <print>

#include "include/chunk.hpp"
#include "include/debug.hpp"
#include "include/vm.hpp"

namespace op = boost::program_options;

namespace {};

int main(int argc, const char* argv[]) {
    using enum lox::chunk::OpCode;
    lox::vm::VM vm;

    auto chunk = lox::chunk::box(lox::chunk::Chunk{});
    chunk->writeConstant(1.2, 123);
    chunk->writeChunk(OP_RETURN, 123);

    vm.interpret(std::move(chunk));
}

// int main(int argc, char* argv[]) {
//     using namespace op;
//     bool isHelp{};
//
//     options_description desc{"cpplox [option] script"};
//     desc.add_options()("help,h", bool_switch(&isHelp), "display help")(
//         "script", value<std::string>(), "script to run");
//
//     positional_options_description positional;
//     positional.add("script", 1);
//
//     command_line_parser parser{argc, argv};
//     parser.options(desc);
//     parser.positional(positional);
//
//     variables_map vm;
//     try {
//         auto parsed = parser.run();
//         store(parsed, vm);
//         notify(vm);
//     } catch (const std::exception& e) {
//         std::println(std::cerr, "Error -> {}", e.what());
//         return 64;
//     }
//
//     if (isHelp) {
//         std::cout << desc;
//         return 0;
//     }
//
//     try {
//         if (vm["script"].empty()) {
//             std::println("REPL");
//             return 0;
//         } else {
//             auto file = vm["script"].as<std::string>();
//             std::println("FILE");
//             return 0;
//         }
//     } catch (const std::exception& e) {
//         std::println(std::cerr, "Error running -> {}", e.what());
//         return -1;
//     }
//
//     return 0;
// }
