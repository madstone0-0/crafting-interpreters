#pragma once
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "environment_fwd.hpp"
#include "token_fwd.hpp"
#include "utils.hpp"
namespace lox::lexer {

    struct Callable;
    struct Class;
    struct Instance;

    using StringLiteral = std::string;
    using NumberLiteral = double;
    using BoolLiteral = bool;
    using CallableLiteral = Callable;
    using ClassType = std::shared_ptr<Class>;
    using InstanceType = std::shared_ptr<Instance>;
    using ClassLiteral = ClassType;
    using InstanceLiteral = InstanceType;

    using LiteralVariant =
        std::variant<std::monostate, StringLiteral, NumberLiteral, BoolLiteral,
                     CallableLiteral, ClassLiteral, InstanceLiteral>;
    using Literal = LiteralVariant;

    using Literals = std::vector<Literal>;

    using CallableFunc = std::function<lexer::Literal(
        const environment::EnvPtr&, const Literals&)>;

    struct Callable {
        std::string name;
        size_t arity_{};
        environment::EnvPtr closure;
        CallableFunc f;
        bool isInit{};

        bool operator==(const Callable&) const { return false; }

        lexer::Literal call(const Literals&) const;

        auto arity() const { return arity_; }

        Callable bind(InstanceLiteral);
    };

    using Methods = std::unordered_map<std::string, lexer::CallableLiteral>;
    using Getters = std::unordered_map<std::string, lexer::CallableLiteral>;

    struct Class : public std::enable_shared_from_this<Class> {
        std::string name_;
        Methods methods;
        Getters getters;

        Class(std::string_view name, Methods methods, Getters getters)
        : name_{name}
        , methods{std::move(methods)}
        , getters{std::move(getters)} {}

        // TODO: come back to this
        bool operator==(const Class& other) const {
            return other.name_ == name_;
        }

        lexer::Literal call(const Literals&) const;

        std::optional<lexer::CallableLiteral> findMethod(
            const std::string&) const;

        std::optional<lexer::CallableLiteral> findGetter(
            const std::string&) const;

        size_t arity() const;

        auto name() const { return name_; }
    };

    using Fields = std::unordered_map<std::string, lexer::Literal>;
    struct Instance : public std::enable_shared_from_this<Instance> {
        std::shared_ptr<const Class> c;
        Fields fields;

        Instance(std::shared_ptr<Class const> c)
        : c{std::move(c)} {}

        bool operator==(const Instance& other) const {
            // Two instances can only be the same if:
            // - they are instances of the same class
            // - they are the same instance i.e. exist at the same address in
            // memory
            return (other.c.get() == c.get()) && (this == &other);
        }

        lexer::Literal access(const lexer::Token&);

        void set(const lexer::Token&, const lexer::Literal&);

        std::string name() const {
            return std::format("{} instance", c->name());
        }
    };

    inline constexpr auto LiteralVisitor = utils::overload{
        [](const StringLiteral& e) { return e; },
        [](const NumberLiteral& e) { return std::to_string(e); },
        [](const BoolLiteral& e) { return std::string{e ? "true" : "false"}; },
        [](const CallableLiteral& e) { return e.name; },
        [](const ClassLiteral& e) { return e->name(); },
        [](const InstanceLiteral& e) { return e->name(); },
        [](std::monostate) { return std::string{""}; },
    };
}
