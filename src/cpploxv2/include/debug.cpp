#include "debug.hpp"

namespace lox::debug {
    void disassembleChunk(const chunk::ChunkPtr& chunk,
                          std::string_view name) {
        std::println("== {} ==", name);
        for (size_t offset{}; offset < chunk->codes.size();) {
            offset = disassembleInstruction(chunk, offset);
        }
    }

    size_t disassembleInstruction(const chunk::ChunkPtr& chunk,
                                  size_t offset) {
        using enum chunk::OpCode;
        std::print("{:04d} ", offset);

        if (offset > 0 &&
            chunk->getLine(offset) == chunk->getLine(offset - 1)) {
            std::print(" | ");
        } else {
            std::print("{:4d} ", chunk->getLine(offset));
        }

        auto ins = chunk->codes.at(offset);
        switch (ins) {
            case OP_RETURN:
                return simpleInstruction(chunk::OpCode{ins}, offset);
            case OP_CONSTANT:
            case OP_CONSTANT_LONG:
                return constantInstruction(chunk::OpCode{ins}, chunk, offset);
            default:
                std::println("Unknown opcode {}", ins);
                return offset + 1;
        };
    }

    size_t simpleInstruction(chunk::OpCode opcode, size_t offset) {
        std::println("{}", utils::enumToString(opcode));
        return offset + 1;
    }

    size_t constantInstruction(chunk::OpCode opcode,
                               const chunk::ChunkPtr& chunk, size_t offset) {
        using enum chunk::OpCode;
        size_t constant{};
        size_t length{};

        if (opcode == OP_CONSTANT) {
            constant = chunk->codes.at(offset + 1);
            length = 2;
        } else {
            constant = chunk->codes.at(offset + 1) |
                       (chunk->codes.at(offset + 2) << 8) |
                       (chunk->codes.at(offset + 3) << 16);
            length = 4;
        }

        std::println("{:16s} {:04d} {}", utils::enumToString(opcode), constant,
                     chunk->constants.at(constant));
        return offset + length;
    }
}
