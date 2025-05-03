#include "json_builder.h"

using namespace std;

namespace json {

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}

DictKeyContext Builder::Key(string key) {
    auto* last_node = nodes_stack_.back();

    if (!key_.has_value() && last_node->IsMap()) {
        key_ = move(key);
    } else {
        throw logic_error("Key error"s);
    }
    return *this;
}

Builder& Builder::Value(Node::Value value) {
    auto* last_node = nodes_stack_.back();
    
    if (nodes_stack_.empty()) {
        throw logic_error("Value error"s);
    }
    if (key_.has_value()) {
        if (last_node->IsMap()) {
            auto& dict = get<Dict>(last_node->GetValue());
            auto [pos, _] = dict.emplace(move(key_.value()), Node{});
            key_.reset();
            last_node = &pos->second;
            *last_node = GetNode(move(value));
        } else {
            throw logic_error("Value`s dict key error"s);
        }
    } else if (last_node->IsArray()) {
        auto& array = get<Array>(last_node->GetValue());
        array.emplace_back(GetNode(value));
        last_node = &array.back();
    } else if (root_.IsNull()) {
        root_ = GetNode(move(value));
    } else {
        throw logic_error("Value`s container error"s);
    }
    return *this;
}

DictItemContext Builder::StartDict() {
    auto* last_node = nodes_stack_.back();
    
    if (key_.has_value()) {
        if (last_node->IsMap()) {
            auto& dict = get<Dict>(last_node->GetValue());
            auto [pos, _] = dict.emplace(move(key_.value()), Dict());
            key_.reset();
            last_node = &pos->second;
            nodes_stack_.emplace_back(last_node);
        } else {
            throw logic_error("Dict`s dict key error");
        }
    } else if (last_node->IsArray()) {
        auto& array = get<Array>(last_node->GetValue());
        array.emplace_back(Dict());
        nodes_stack_.emplace_back(&array.back());
    } else if (last_node->IsNull()) {
        *last_node = GetNode(Dict());
        root_ = GetNode(Dict());
    } else {
        throw logic_error("Dict`s parent error");
    }
    return *this;
}

ArrayItemContext Builder::StartArray() {
    auto* last_node = nodes_stack_.back();
    
    if (key_.has_value()) {
        if (last_node->IsMap()) {
            auto& dict = get<Dict>(last_node->GetValue());
            auto [pos, _] = dict.emplace(move(key_.value()), Array());
            key_.reset();
            last_node = &pos->second;
            nodes_stack_.emplace_back(last_node);
        } else {
            throw logic_error("Array`s dict key error");
        }
    } else if (last_node->IsArray()) {
        auto& array = get<Array>(last_node->GetValue());
        array.emplace_back(Array());
        nodes_stack_.emplace_back(&array.back());
    } else if (last_node->IsNull()) {
        *last_node = GetNode(Array());
        root_ = GetNode(Array());
    } else {
        throw logic_error("Array`s parent error");
    }
    return *this;
}

Builder& Builder::EndDict() {
    auto* last_node = nodes_stack_.back();
    
    if (!last_node->IsMap()) {
        throw logic_error("Not a Dict");
    } else {
        nodes_stack_.pop_back();
    }
    return *this;
}

Builder& Builder::EndArray() {
    auto* last_node = nodes_stack_.back();
    
    if (!last_node->IsArray()) {
        throw logic_error("Not an Array");
    } else {
        nodes_stack_.pop_back();
    }
    return *this;
}

Node Builder::Build() {
    if (root_.IsNull() || nodes_stack_.size() > 1) {
        throw logic_error("Not all nodes closed");
    }
    return root_;
}

Node Builder::GetNode(Node::Value value) {
    if (holds_alternative<int>(value)) return Node(get<int>(value));
    if (holds_alternative<double>(value)) return Node(get<double>(value));
    if (holds_alternative<string>(value)) return Node(get<string>(value));
    if (holds_alternative<nullptr_t>(value)) return Node(get<nullptr_t>(value));
    if (holds_alternative<bool>(value)) return Node(get<bool>(value));
    if (holds_alternative<Dict>(value)) return Node(get<Dict>(value));
    if (holds_alternative<Array>(value)) return Node(get<Array>(value));
    return {};
}

DictItemContext DictKeyContext::Value(Node::Value value) {
    return DictItemContext(builder_.Value(value));
}

ArrayItemContext DictKeyContext::StartArray() {
    return builder_.StartArray();
}

DictItemContext DictKeyContext::StartDict() {
    return builder_.StartDict();
}

DictKeyContext DictItemContext::Key(string key) {
    return builder_.Key(move(key));
}

Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return ArrayItemContext(builder_.Value(value));
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder& ArrayItemContext::EndArray() {
    return builder_.EndArray();
}

} // namespace json