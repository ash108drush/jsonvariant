#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {
using namespace std::literals;

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};





class Node {
public:
    /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    const Value& GetValue() const { return value_; }

//   explicit Node(Array array);
//    explicit Node(Dict map);
    explicit Node(Value && value):value_(std::move(value)){};
//    explicit Node(int value);
//    explicit Node(std::string value);

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    const std::string& AsString() const;

private:
    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }

    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }
    // Другие перегрузки функции PrintValue пишутся аналогично




    Value value_;
    Array as_array_;
    Dict as_map_;
    int as_int_ = 0;
    std::string as_string_;
};




class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
