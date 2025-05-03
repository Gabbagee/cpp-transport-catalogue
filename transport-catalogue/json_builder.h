#pragma once

#include "json.h"

#include <optional>

namespace json {

class DictKeyContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder();
    DictKeyContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndDict();
    Builder& EndArray();
    Node Build();
    Node GetNode(Node::Value value);

private:
    Node root_{nullptr};
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_{std::nullopt};
};

class DictKeyContext {
public:
    DictKeyContext(Builder& builder) 
    : builder_(builder) {}
    
    DictItemContext Value(Node::Value value);
    ArrayItemContext StartArray();
    DictItemContext StartDict();
    
private:
    Builder& builder_;
};

class DictItemContext {
public:
    DictItemContext(Builder& builder) 
    : builder_(builder) {}

    DictKeyContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};
    
class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) 
    : builder_(builder) {}

    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

} // namespace json
