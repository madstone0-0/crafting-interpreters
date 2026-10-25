#include "chunk.hpp"

namespace lox::chunk {
    void Chunk::writeChunk(Byte byte, size_t line) {
        codes.emplace_back(byte);
        if (!lineRuns.empty() && lineRuns.back().line == line) {
            ++lineRuns.back().count;
        } else {
            lineRuns.emplace_back(line, 1);
        }
    }

    void Chunk::writeConstant(const value::Value& value, size_t line) {
        const auto idx = addConstant(value);
        if (idx <= 0xFF) {
            writeChunk(OP_CONSTANT, line);
            writeChunk(idx, line);
        } else if (idx <= 0xFFFFFF) {
            writeChunk(OP_CONSTANT_LONG, line);
            writeChunk(static_cast<Byte>(idx & 0xFF), line);
            writeChunk(static_cast<Byte>((idx >> 8) & 0xFF), line);
            writeChunk(static_cast<Byte>((idx >> 16) & 0xFF), line);
        } else {
            throw std::length_error{"too many constants"};
        }
    }

    size_t Chunk::addConstant(const value::Value& value) {
        constants.emplace_back(value);
        return static_cast<int>(constants.size()) - 1;
    }

    size_t Chunk::getLine(size_t offset) const {
        size_t seen{};
        for (const auto& run : lineRuns) {
            seen += run.count;
            if (offset < seen) return run.line;
        }
        throw std::out_of_range{"offset past end of chunk"};
    }

}
