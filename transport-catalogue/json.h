#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, 
    double, std::string>;

    const Value& GetValue() const { 
        return value_; 
    }

    Node() = default;

    Node(std::nullptr_t value) 
        : value_(value) {
    }

    Node(int value)
        : value_(value) {
    }

    Node(double value) 
        : value_(value) {
    }

    Node(bool value) 
        : value_(value) {
    }

    Node(std::string value)
        : value_(move(value)) {
    }

    Node(Array array)
        : value_(move(array)) {
    }

    Node(Dict map)
        : value_(move(map)) {
    }

    bool IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }

    bool IsInt() const {
        return std::holds_alternative<int>(value_);
    }

    bool IsDouble() const {
        return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_);
    }

    bool IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

    bool IsBool() const {
        return std::holds_alternative<bool>(value_);
    }

    bool IsString() const {
        return std::holds_alternative<std::string>(value_);
    }

    bool IsArray() const {
        return std::holds_alternative<Array>(value_);
    }

    bool IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }

    int AsInt() const {
        using namespace std::literals;
        if (!IsInt()) {
            throw std::logic_error("Unexpected type access"s);
        }
        return std::get<int>(value_);
    }

    double AsDouble() const {
        using namespace std::literals;
        if (!IsDouble()) {
            throw std::logic_error("Unexpected type access"s);
        }
        if (IsPureDouble()) {
            return std::get<double>(value_);
        }
        return std::get<int>(value_);
    }

    bool AsBool() const {
        using namespace std::literals;
        if (!IsBool()) {
            throw std::logic_error("Unexpected type access"s);
        }
        return std::get<bool>(value_);
    }

    const std::string& AsString() const {
        using namespace std::literals;
        if (!IsString()) {
            throw std::logic_error("Unexpected type access"s);
        }
        return std::get<std::string>(value_);
    }

    const Array& AsArray() const {
        using namespace std::literals;
        if (!IsArray()) {
            throw std::logic_error("Unexpected type access"s);
        }
        return std::get<Array>(value_);
    }

    const Dict& AsMap() const {
        using namespace std::literals;
        if (!IsMap()) {
            throw std::logic_error("Unexpected type access"s);
        }
        return std::get<Dict>(value_);
    }


    bool operator==(const Node& other) const {
        return value_ == other.value_;
    }

    bool operator!=(const Node& other) const {
        return !(*this == other);
    }

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json