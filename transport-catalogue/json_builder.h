#pragma once

#include "json.h"

#include <optional>

namespace json {

class Builder {
    class ItemContext;
    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;
public:
    Builder() = default;

    DictItemContext StartDict();

    ValueItemContext EndDict();

    KeyItemContext Key(std::string key);

    ValueItemContext Value(Node::Value value);

    ArrayItemContext StartArray();

    ValueItemContext EndArray();

    Node Build();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> cur_key_;
    bool has_data_ = false;

    class ItemContext {
    public:
        Builder& builder;

        ItemContext(Builder& bldr) 
            : builder(bldr) {
        }

        ItemContext& Key(std::string key) {
            builder.Key(std::move(key));
            return *this;
        }

        ItemContext& Value(Node::Value value) {
            builder.Value(std::move(value));
            return *this;
        }

        ItemContext& StartDict() {
            builder.StartDict();
            return *this;
        }

        ItemContext& StartArray() {
            builder.StartArray();
            return *this;
        }

        ItemContext& EndDict() {
            builder.EndDict();
            return *this;
        }

        ItemContext& EndArray() {
            builder.EndArray();
            return *this;
        }

        Node Build() {
            return builder.Build();
        }
    };

    class KeyItemContext : public ItemContext {
    public:
        ItemContext& Key(std::string key) = delete;

        DictItemContext Value(Node::Value value) {
            builder.Value(std::move(value));
            return DictItemContext{*this};
        }

        DictItemContext StartDict() {
            builder.StartDict();
            return DictItemContext{*this};
        }

        ArrayItemContext StartArray() {
            builder.StartArray();
            return ArrayItemContext{*this};
        }

        ItemContext& EndDict() = delete;
        ItemContext& EndArray() = delete;
        Node Build() = delete;
    };

    class ValueItemContext : public ItemContext {
    public:
        ItemContext& Key(std::string key) = delete;
        ItemContext& Value(Node::Value value) = delete;
        ItemContext& StartDict() = delete;
        ItemContext& StartArray() = delete;
        ItemContext& EndDict() = delete;
        ItemContext& EndArray() = delete;

        Node Build() {
            return builder.Build();
        }
    };

    class DictItemContext : public ItemContext {
    public:
        KeyItemContext Key(std::string key) {
            builder.Key(std::move(key));
            return KeyItemContext{*this};
        }

        ItemContext& Value(Node::Value value) = delete;
        ItemContext& StartDict() = delete;
        ItemContext& StartArray() = delete;

        ItemContext& EndDict() {
            builder.EndDict();
            return *this;
        }

        ItemContext& EndArray() = delete;
        Node Build() = delete;
    };

    class ArrayItemContext : public ItemContext {
    public:
        ItemContext& Key(std::string key) = delete;

        ArrayItemContext Value(Node::Value value) {
            builder.Value(std::move(value));
            return ArrayItemContext{*this};
        }

        DictItemContext StartDict() {
            builder.StartDict();
            return DictItemContext{*this};
        }

        ArrayItemContext StartArray() {
            builder.StartArray();
            return *this;
        }

        ItemContext& EndDict() = delete;

        ItemContext& EndArray() {
            builder.EndArray();
            return *this;
        }

        Node Build() = delete;
    };

    void CheckIfFinalized() const;

    template <typename T>
    void AddContainerToStack();

    void AddValue(Node value);

    void AddDictValue(Node::Value value);
};

} // namespace json