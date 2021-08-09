#include "json.hpp"

#include <cctype> // для функции isalpha
#include <utility>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            Number result;

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
                        result = std::stoi(parsed_num);

                        return Node{get<int>(result)};
                    } catch (...) {
                        // В случае неудачи, например, при переполнении
                        // код ниже попробует преобразовать строку в double
                    }
                }
                result = std::stod(parsed_num);

                return Node{get<double>(result)};
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            string parsed_string;

            // Считывает в parsed_string очередной символ из input
            auto read_char = [&parsed_string, &input] {
                parsed_string += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            size_t cnt_quota = 0;
            while (input.peek() != EOF) {
                char ch = input.peek();
                if (ch == '"') {
                    ++cnt_quota;
                    input.ignore();
                    break;
                } else if (ch == '\\') {
                    input.ignore();
                    ch = input.peek();
                    switch (ch) {
                        case 'n':
                            input.ignore();
                            parsed_string += '\n';
                            break;
                        case 't':
                            input.ignore();
                            parsed_string += '\t';
                            break;
                        case 'r':
                            input.ignore();
                            parsed_string += '\r';
                            break;
                        case '"':
                            read_char();
                            break;
                        case '\\':
                            read_char();
                            break;
                        case ':':
                            read_char();
                            break;
                        case '/':
                            read_char();
                            break;
                        case '}':
                            read_char();
                            break;
                        case ']':
                            read_char();
                            break;
                        default:
                            throw ParsingError("Unrecognized escape sequence \\"s + ch);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    read_char();
                }
            }

            if (cnt_quota != 1) {
                throw ParsingError("String parsing error");
            }

            return Node(move(parsed_string));
        }

        Node LoadBool(istream& input) {
            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            while (input) {
                if (input.peek() == ',') {
                    break;
                } else if (isalpha(input.peek())) {
                    read_char();
                } else if (input.peek() == '}' || input.peek() == ']') {
                    break;
                } else {
                    input.ignore();
                }
            }

            if (parsed_num != "true"s && parsed_num != "false"s) {
                throw ParsingError("type bool failed"s);
            }

            return Node(parsed_num == "true"s);
        }

        Node LoadArray(istream& input) {
            Array result;
            size_t cnt_escape = 0;

            for (char c; input >> c;) {
                if (c == ']') {
                    ++cnt_escape;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (cnt_escape < 1) {
                throw ParsingError("not even ["s);
            }

            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            size_t cnt_escape = 0;

            for (char c; input >> c;) {
                if (c == '}') {
                    ++cnt_escape;
                    break;
                }
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }

            if (cnt_escape < 1) {
                throw ParsingError("not even {"s);
            }

            return Node(move(result));
        }

        Node IsNull(istream& input) {
            string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            while (input) {
                if (input.peek() == ',') {
                    break;
                } else if (isalpha(input.peek())) {
                    read_char();
                } else if (input.peek() == '}' || input.peek() == ']') {
                    break;
                } else {
                    input.ignore();
                }
            }

            if (parsed_num != "null"s) {
                throw ParsingError("is null failed");
            }

            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            } else if (c == '{') {
                return LoadDict(input);
            } else if (c == '"') {
                return LoadString(input);
            } else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            } else if (c == 'n') {
                input.putback(c);
                return IsNull(input);;
            } else {
                input.putback(c);
                return LoadNumber(input);
            }
        }
        
    }  // namespace

    Node::Node(nullptr_t n)
            : node_value_(n) {
    }

    Node::Node(bool b)
            : node_value_(b) {
    }

    Node::Node(double d)
            : node_value_(d) {
    }

    Node::Node(NodeValue value)
            : node_value_(std::move(value)) {
    }

    Node::Node(Array array)
            : node_value_(move(array)) {
    }

    Node::Node(Dict map)
            : node_value_(move(map)) {
    }

    Node::Node(int value)
            : node_value_(value) {
    }

    Node::Node(string value)
            : node_value_(move(value)) {
    }

    // ************************* /
    // ********* AS ************ /
    // ************************* /
    int Node::AsInt() const {
        if (!IsInt()) {
            throw logic_error("is not int type"s);
        }
        return get<int>(node_value_);
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(AsInt());
        }
        if (!IsDouble()) {
            throw logic_error("is not double type");
        }
        return get<double>(node_value_);
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("is not string type");
        }
        return get<string>(node_value_);
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("is not bool type");
        }
        return get<bool>(node_value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("is not array type");
        }
        return get<Array>(node_value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw logic_error("is not map type");
        }
        return get<Dict>(node_value_);
    }

    // ************************* /
    // ********* IS ************/
    // ************************* /
    bool Node::IsNull() const {
        if (holds_alternative<nullptr_t>(node_value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsInt() const {
        if (holds_alternative<int>(node_value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsDouble() const {
        if (holds_alternative<double>(node_value_)) {
            return true;
        } else if (holds_alternative<int>(node_value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsString() const {
        if (holds_alternative<string>(node_value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsBool() const {
        if (holds_alternative<bool>(node_value_)) {
            return true;
        }
        return false;
    }

    bool Node::IsArray() const {
        if (holds_alternative<Array>(node_value_)) {
            return true;
        }
        return false;
    }

bool Node::IsMap() const {
    if (holds_alternative<Dict>(node_value_)) {
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const {
    if (holds_alternative<double>(node_value_)) {
        return true;
    } else if (holds_alternative<int>(node_value_)) {
        return false;
    }
    return false;
}

Document::Document(Node root)
        : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

ostream& operator<<(ostream& output, const Node& node) {
        if (node.IsNull()) {
            output << "null"s;
        }

        else if (node.IsInt()) {
            output << node.AsInt();
        }

        else if (node.IsDouble()) {
            output << std::hex << node.AsDouble();
        }

        else if (node.IsString()) {
            output << "\"";

            for (const char& ch : node.AsString()) {
                if (ch == '"') {
                    output << "\\";
                }
                output << ch;
            }

            output << "\"";
        }

        else if (node.IsBool()) {
            if (node.AsBool() == true) {
                output << "true"s;
            } else {
                output << "false"s;
            }
        }

        else if (node.IsArray()) {
            output << "["s;
            bool flag = false;

            for (const auto& arr : node.AsArray()) {
                if (flag) {
                    output << ", "s;
                }
                output << arr;
                flag = true;
            }

            output << "]"s;
        }

        else if (node.IsMap()) {
            output << std::endl<< "{ "s << std::endl;

            bool flag = false;
            for (const auto& dict : node.AsMap()) {
                if (flag) {
                    output << ", "s << std::endl;
                }
                output << "\"" << dict.first << "\"";
                output << ": "s;
                output << dict.second;
                flag = true;
            }

            output << std::endl << " }"s;
        }

        return output;
    }

    void Print(const Document& doc, std::ostream& output) {
        output << doc.GetRoot();
    }

}
