#pragma once

#include <print>
#include <string_view>

#include "chunk.hpp"
#include "utils.hpp"

namespace lox::debug {
    void disassembleChunk(const chunk::ChunkPtr&, std::string_view);
    size_t disassembleInstruction(const chunk::ChunkPtr&, size_t);
    size_t simpleInstruction(chunk::OpCode, size_t);
    size_t constantInstruction(chunk::OpCode, const chunk::ChunkPtr&, size_t);
}
