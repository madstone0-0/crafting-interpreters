#pragma once
#include <print>

#include "chunk.hpp"
#include "debug.hpp"

namespace lox::vm {
    enum class InterpretResult : uint8_t {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR,
    };

    class VM {
       private:
        chunk::ChunkPtr chunk;
        chunk::Byte* ip;

        InterpretResult run();

       public:
        InterpretResult interpret(chunk::ChunkPtr);
    };
}
