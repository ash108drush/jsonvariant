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
    using Value = std::variant<std::nullptr_t, int, double, std::string,bool,Array,Dict>;

    const Value& GetValue() const { return value_; }


    Node(Value value):value_(value){};

    Node() = default;
    Node(std::nullptr_t) {};
    Node(Dict value) : value_(value) {};
    Node(Array value) : value_(value) {};
    Node(bool value) : value_(value) {};
    Node(int value) : value_(value) {};
    Node(double value) : value_(value) {};
    Node(std::string value) : value_(value) {};

   bool operator == (const Node& n) const {
        return (n.GetValue() == value_);
    }

    bool operator != (const Node& n) const {
        return !(*this == n);
    }

    bool IsInt() const {
        return  std::holds_alternative<int>(value_);
    };
    bool IsDouble() const{
        return  (std::holds_alternative<double>(value_) || std::holds_alternative<int>(value_)) ;
    };


    bool IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }

/*
    bool IsPureDouble() const{
        if (!std::holds_alternative<double>(value_)) {
            return false;  // Не double — сразу false
        }
        const double d = std::get<double>(value_);
        return d != static_cast<int>(d);  // Если при приведении к int значение меняется — это «чистый» double
    };
*/
    bool IsBool() const{
        return  std::holds_alternative<bool>(value_);
    };
    bool IsString() const{
        return  std::holds_alternative<std::string>(value_);
    };
    bool IsNull() const{
        return  std::holds_alternative<std::nullptr_t>(value_);
    };
    bool IsArray() const{
         return  std::holds_alternative<Array>(value_);
    };
    bool IsMap() const{
         return  std::holds_alternative<Dict>(value_);
    };

    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;

private:
    Value value_;

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


inline bool operator==(const Document &d1,const Document &d2){
    return d1.GetRoot() == d2.GetRoot();
}
inline bool operator!=(const Document &d1,const Document d2){
    return !(d1 == d2);
}

}  // namespace json
