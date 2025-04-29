#include "json.h"

#include <string_view>

using namespace std;

namespace json {

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("Failed to read array from stream"s);
    }

    return Node(move(result));
}

Node LoadNumber(istream& input) {
    using namespace literals;

    string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(istream& input) {
    using namespace literals;
    
    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(move(s));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        if (c != '"') {
            throw ParsingError("Expected key in double quotes");
        }

        string key = LoadString(input).AsString();
        input >> c;
        if (c != ':') {
            throw ParsingError("Expected ':' after key");
        }

        result.insert({move(key), LoadNode(input)});
    }

    if (!input) {
        throw ParsingError("Failed to read dictionary from stream"s);
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (isdigit(c) || c == '-') {
        input.putback(c);
        return LoadNumber(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (isalpha(c)) {
        string word;
        word += c;

        while (isalpha(input.peek())) {
            word += static_cast<char>(input.get());
        }
        
        if (word == "null") {
            return Node(nullptr);
        } else if (word == "true") {
            return Node(true);
        } else if (word == "false") {
            return Node(false);
        } else {
            throw ParsingError("Invalid literal: " + word);
        }
    } else {
        throw ParsingError("Failed to read JSON from stream"s);
    }
}

bool Node::IsNull() const {
    return holds_alternative<nullptr_t>(value_);
}

bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}

bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}

bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return holds_alternative<double>(value_) || holds_alternative<int>(value_);
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}

bool Node::IsString() const {
    return holds_alternative<string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("Not an array");
    }
    return get<Array>(value_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("Not a map");
    }
    return get<Dict>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("Not a bool");
    }
    return get<bool>(value_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("Not an int");
    }
    return get<int>(value_);
}

double Node::AsDouble() const {
    if (IsInt()) {
        return static_cast<double>(get<int>(value_));
    } else if (IsPureDouble()) {
        return get<double>(value_);
    }
    throw logic_error("Not a double");
}

const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("Not a string");
    }
    return get<string>(value_);
}

const Node::Value& Node::GetValue() const {
    return value_;
}

bool Node::operator==(const Node& other) const {
    return value_ == other.value_;
}

bool Node::operator!=(const Node& other) const {
    return value_ != other.value_;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document& other) const {
    return root_ == other.root_;
}

bool Document::operator!=(const Document& other) const {
    return root_ != other.root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintValue(nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(string value, const PrintContext& ctx) {
    ctx.out << '"';
    for (const char c : value) {
        switch (c) {
            case '\\':
                ctx.out << "\\\\"sv;
                break;
            case '"':
                ctx.out << "\\\""sv;
                break;
            case '\n':
                ctx.out << "\\n"sv;
                break;
            case '\t':
                ctx.out << "\\t"sv;
                break;
            case '\r':
                ctx.out << "\\r"sv;
                break;
            default:
                ctx.out << c;
        }
    }
    ctx.out << '"';
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << boolalpha << value;
}

void PrintValue(Array array, const PrintContext& ctx) {
    ctx.out << "[\n"sv;
    auto inner_ctx = ctx.Indented();
    bool first = true;
    for (const auto& elem : array) {
        if (!first) {
            ctx.out << ", \n"sv;
        }

        first = false;
        inner_ctx.PrintIndent();
        PrintNode(elem, inner_ctx);
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << "]"sv;
}

void PrintValue(Dict dict, const PrintContext& ctx) {
    ctx.out << "{\n"sv;
    auto inner_ctx = ctx.Indented();
    bool first = true;
    for (const auto& [key, node] : dict) {
        if (!first) {
            ctx.out << ", \n"sv;
        }

        first = false;
        inner_ctx.PrintIndent();
        PrintValue(key, ctx);
        ctx.out << ": ";
        PrintNode(node, inner_ctx);
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << "}"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    visit([&ctx](const auto& value) {
        PrintValue(value, ctx);
    }, node.GetValue());
}

void Print(const Document& doc, ostream& output) {
    PrintContext ctx{output};
    PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json