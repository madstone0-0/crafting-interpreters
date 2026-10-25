#include "vm.hpp"

namespace lox::vm {

    InterpretResult VM::run() {
        using enum chunk::OpCode;
        using enum InterpretResult;

        const auto readByte = [this]() { return *ip++; };

        const auto readConstant = [this, &readByte](bool isLong) {
            auto idx = readByte();
            if (isLong) {
                idx |= readByte() << 8;
                idx |= readByte() << 16;
            }
            return chunk->constants.at(idx);
        };

        while (true) {
#ifdef DEBUG_TRACE_EXECUTION
            lox::debug::disassembleInstruction(
                chunk, static_cast<int>(ip - chunk->codes.data()));
#endif  // DEBUG_TRACE_EXECUTION

            chunk::Byte ins{};
            switch (ins = readByte()) {
                case OP_CONSTANT:
                case OP_CONSTANT_LONG: {
                    auto constant = readConstant(ins == OP_CONSTANT_LONG);
                    std::println("{}", constant);
                    break;
                }

                case OP_RETURN:
                    return OK;
            }
        }
    }

    InterpretResult VM::interpret(chunk::ChunkPtr chunk) {
        this->chunk = std::move(chunk);
        this->ip = this->chunk->codes.data();
        return run();
    }
}
