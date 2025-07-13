#include "json_builder.h"

namespace json {

Builder::DictItemContext Builder::StartDict() {
    AddContainerToStack<Dict>();
    has_data_ = true;
    return DictItemContext{*this};
}

Builder::ValueItemContext Builder::EndDict() {
    if (!nodes_stack_.empty() && !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Failed to end dict");
    }
    nodes_stack_.pop_back();
    return ValueItemContext{*this};
}

Builder::KeyItemContext Builder::Key(std::string key) {
    cur_key_ = std::move(key);
    return KeyItemContext{*this};
}

Builder::ValueItemContext Builder::Value(Node::Value value) {
    if (!cur_key_) {
        AddValue(Node{std::move(value)});
    } else {
        AddDictValue(std::move(value));
    }
    has_data_ = true;
    return ValueItemContext{*this};
}

Builder::ArrayItemContext Builder::StartArray() {
    AddContainerToStack<Array>();
    has_data_ = true;
    return ArrayItemContext{*this};
}

Builder::ValueItemContext Builder::EndArray() {
    if (!nodes_stack_.empty() && !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Failed to end array");
    }
    nodes_stack_.pop_back();
    return ValueItemContext{*this};
}

Node Builder::Build() {
    if (!nodes_stack_.empty() || !has_data_) {
        throw std::logic_error("Failed to build");
    }
    return std::move(root_);
}

void Builder::CheckIfFinalized() const {
    if (has_data_ && !root_.IsMap() && !root_.IsArray()) {
        throw std::logic_error("Failed to change finalized object");
    }
}

template <typename T>
void Builder::AddContainerToStack() {
    CheckIfFinalized();
    if (root_.IsNull()) {
        root_ = T{};
        nodes_stack_.push_back(&root_);
    } else {
        Node* last_node = nodes_stack_.back();
        if (last_node->IsMap() && cur_key_) {
            auto key = cur_key_.value();
            Value(T{});
            nodes_stack_.push_back(&last_node->AsMap().at(key));
        } else if (last_node->IsArray()) {
            last_node->AsArray().emplace_back(T{});
            nodes_stack_.push_back(&last_node->AsArray().back());
        }
    }
}

void Builder::AddValue(Node value) {
    CheckIfFinalized();
    if (root_.IsNull()) {
        root_ = std::move(value);
    } else if (Node* last_node = nodes_stack_.back(); 
            last_node->IsArray()) {
        last_node->AsArray().emplace_back(std::move(value));
    }
}

void Builder::AddDictValue(Node::Value value) {
    Node* last_node = nodes_stack_.back();
    last_node->AsMap().emplace(cur_key_.value(), std::move(value));
    cur_key_ = std::nullopt;
}

} // namespace json