#include "json.h"

using namespace std;

namespace json {

Node LoadNode(std::istream& input);

std::string ParseString(std::istream& input) {
    std::string s;
    char c;
    while (input.get(c) && c != ' ' && c != '\n' && c != '\t' && c != '\r') {
        if (c == ',' || c == ']' || c == '}') {
            input.putback(c);
            break;
        }

        s += static_cast<char>(c);
    }
    return s;
}

nullptr_t LoadNull(std::istream& input) {
    if (ParseString(input) == "null"s) {
        return nullptr;
    } else {
        throw ParsingError("Failed to read null from stream"s);
    }
}

bool LoadBool(std::istream& input) {
    std::string s = ParseString(input);
    if (s == "true"s) {
        return true;
    } else if (s == "false"s) {
        return false;
    } else {
        throw ParsingError("Failed to read bool from stream"s);
    }
}

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
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
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string LoadString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
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

    return s;
}

Array LoadArray(istream& input) {
    Array result;
    bool brace_found = false;
    for (char c; input >> c;) {
        if (c == ']') {
            brace_found = true;
            break;
        }
        if (c != ',') {
            input.putback(c);
        }

        result.push_back(LoadNode(input));
    }
    if (brace_found) {
        return result;
    } else {
        throw ParsingError("Failed to read array from stream"s);
    }
}

Dict LoadDict(istream& input) {
    Dict result;
    bool brace_found = false;
    for (char c; input >> c;) {
        if (c == '}') {
            brace_found = true;
            break;
        }
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
    if (brace_found) {
        return result;
    } else {
        throw ParsingError("Failed to read map from stream"s);
    }
}

void SkipSpacesAndEscapes(std::istream& input) {
    char c;
    while (input.get(c)) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
            continue;
        } else {
            input.putback(c);
            break;
        }
    }
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return Node(LoadArray(input));
    } else if (c == '{') {
        return Node(LoadDict(input));
    } else if (c == '"') {
        return Node(LoadString(input));
    } else {
        input.putback(c);
        SkipSpacesAndEscapes(input);
        auto p = input.peek();
        if (p == 'n') {
            return Node(LoadNull(input));
        } else if (p == 't' || p == 'f') {
            return Node(LoadBool(input));
        } else {
            Number number = LoadNumber(input);
            if (holds_alternative<double>(number)) {
                return Node(get<double>(number));
            } else if (holds_alternative<int>(number)) {
                return Node(get<int>(number));
            }
        }
    }
    return Node{};
}

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& value, const PrintContext& ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"s;
}

template <>
void PrintValue<bool>(const bool& value, const PrintContext& ctx) {
    ctx.out << std::boolalpha << value << std::noboolalpha;
}

void PrintString(const std::string& str, std::ostream& out) {
    out << '"';
    for (char ch : str) {
        switch (ch) {
            case '\n':
                out << "\\n";
                break;
            case '\t':
                out << "\\t";
                break;
            case '\r':
                out << "\\r";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            default:
                out << ch;
                break;
        }
    }
    out << '"';
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<Array>(const Array& arr, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "[\n"s;
    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& elem : arr) {
        if (is_first) {
            is_first = false;
        } else {
            out << ",\n"s;
        }
        inner_ctx.PrintIndent();
        PrintNode(elem, inner_ctx);
    }
    out << '\n';
    ctx.PrintIndent();
    out << ']';
}

template <>
void PrintValue<Dict>(const Dict& dict, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"s;
    bool is_first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, value] : dict) {
        if (is_first) {
            is_first = false;
        } else {
            out << ",\n"s;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "s;
        PrintNode(value, inner_ctx);
    }
    out << '\n';
    ctx.PrintIndent();
    out << '}';
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ 
            PrintValue(value, ctx); 
        },
        node.GetValue());
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(std::istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json