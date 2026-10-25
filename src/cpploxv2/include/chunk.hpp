#pragma once
#include <cstdint>
#include <vector>
#define DEBUG_TRACE_EXECUTION

#include "value.hpp"

namespace lox::chunk {
    using Byte = uint8_t;
    using Codes = std::vector<Byte>;

    enum OpCode : Byte {
        OP_CONSTANT,
        OP_CONSTANT_LONG,
        OP_RETURN,
    };

    struct LineRun {
        size_t line{};
        size_t count{};
    };

    struct Chunk {
        Codes codes;
        value::Values constants;
        std::vector<LineRun> lineRuns;

        void writeChunk(Byte, size_t);
        void writeConstant(const value::Value&, size_t);
        size_t addConstant(const value::Value&);
        size_t getLine(size_t) const;
    };

    using Chunks = std::vector<Chunk>;

    using ChunkPtr = std::unique_ptr<Chunk>;

    inline ChunkPtr box(Chunk c) {
        return std::make_unique<Chunk>(std::move(c));
    }
}
