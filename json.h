#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        explicit Node() = default;
        Node(std::nullptr_t n);
        Node(bool b);
        Node(double b);

        Node(NodeValue value);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(std::string value);

        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool IsNull() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsPureDouble() const;

        bool operator==(const Node& other) const {
            return node_value_ == other.node_value_;
        }
        
        bool operator!=(const Node& other) const {
            return node_value_ != other.node_value_;
        }

    private:
        NodeValue node_value_ = nullptr;
    };

class Document {
public:
explicit Document(Node root);

const Node& GetRoot() const;
        
bool operator== (const Document& other) const {
    return root_ == other.GetRoot();
}
        
bool operator!= (const Document& other) const {
    return root_ != other.GetRoot();
}

private:
    Node root_;
};

   
    
Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}
