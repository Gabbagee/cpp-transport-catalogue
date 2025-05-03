#pragma once

#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>
#include <variant>

namespace json {

struct PrintContext {
    PrintContext(std::ostream& out)
        : out(out) {
    }

    PrintContext(std::ostream& out, int indent_step, int indent)
    : out(out)
    , indent_step(indent_step)
        , indent(indent) {
    }

    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
    
    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
};

class Node;

Node LoadArray(std::istream& input);

Node LoadNumber(std::istream& input);

Node LoadString(std::istream& input);

Node LoadDict(std::istream& input);

Node LoadNode(std::istream& input);

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    Node()
    : value_(nullptr) {}

    Node(std::nullptr_t) 
    : value_(nullptr) {}

    Node(Array array) 
    : value_(std::move(array)) {}

    Node(Dict map) 
    : value_(std::move(map)) {}

    Node(bool value) 
    : value_(value) {}

    Node(int value) 
    : value_(value) {}

    Node(double value) 
    : value_(value) {}

    Node(std::string value) 
    : value_(std::move(value)) {}

    bool IsNull() const;

    bool IsArray() const;

    bool IsMap() const;

    bool IsBool() const;

    bool IsInt() const;

    bool IsDouble() const;

    bool IsPureDouble() const;

    bool IsString() const;

    const Array& AsArray() const;

    const Dict& AsMap() const;

    bool AsBool() const;

    int AsInt() const;

    double AsDouble() const;

    const std::string& AsString() const;

    const Value& GetValue() const;

    Value& GetValue();

    bool operator==(const Node& other) const;

    bool operator!=(const Node& other) const;

private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& other) const;

    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void PrintValue(std::nullptr_t, const PrintContext& ctx);

void PrintValue(std::string value, const PrintContext& ctx);

void PrintValue(bool value, const PrintContext& ctx);

void PrintValue(Array array, const PrintContext& ctx);

void PrintValue(Dict dict, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx);

void PrintNode(const Node& node, const PrintContext& ctx);

void Print(const Document& doc, std::ostream& output);

}  // namespace json