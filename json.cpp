#include "json.h"
#include <cstddef>
#include <iostream>
using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    return Node(std::move(result));
}

Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }

    return Node(result);
}

Node LoadNumber(std::istream& input) {
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
                int num = std::stoi(parsed_num);
                return Node(num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        double dbl = std::stod(parsed_num);
        return Node(dbl);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
    return Node();
}

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

Node LoadNodeString(istream& input) {
    string line = LoadString(input);
    return Node(std::move(line));
}




Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadNodeString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }

    return Node(std::move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadNodeString(input);
    } else {
        if(c =='n'){
            std::string ull="";
            input >> c;
            ull+='u';
            input >> c;
            ull+='l';
            input >> c;
            ull+='l';
            if(ull == "ull"){
                return Node(nullptr);
            }

            }else{
            input.putback(c);
            return LoadInt(input);
            }

    }
    return Node(nullptr);
}

}  // namespace




const Array& Node::AsArray() const {
    try {
        return get<Array>(value_);;
    } catch (const bad_variant_access& e) {
        cout << e.what() << endl;
        return {};
    }

}

const Dict& Node::AsMap() const {
    try {
    return get<Dict>(value_);
    } catch (const bad_variant_access& e) {
        cout << e.what() << endl;
        return {};
    }

}


int Node::AsInt() const {
    try {
        return get<int>(value_);
    } catch (const bad_variant_access& e) {
        cout << e.what() << endl;
        return 0;
    }
}

bool Node::AsBool() const {
    try {
        return get<bool>(value_);
    } catch (const bad_variant_access& e) {
        cout << e.what() << endl;
        return false;
    }
}

const string& Node::AsString() const {
    try {
        return get<std::string>(value_);
    } catch (const bad_variant_access& e) {
        cout << e.what() << endl;
        return "";
    }
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintNode(const Node& node, std::ostream& out);

void PrintValue(std::nullptr_t, std::ostream& out) {
    out << "null"sv;
}

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, std::ostream& out) {
    out << value;
}

//std::string,bool,Array,Dict
void PrintValue(const std::string str, std::ostream& out) {
    out << str;
}

void PrintValue(const Array& arr, std::ostream& out) {
    for(const Node & nd:arr){
        PrintNode( nd,out);
    }

}

//std::map<std::string, Node>;
void PrintValue(const Dict& dict, std::ostream& out) {
    for (const auto& [key, value] : dict) {
        out << key;
        PrintNode(value,out);
    }

}

// Шаблон, подходящий для вывода double и int
//template <typename Value>
//void PrintValue(const Value& value, std::ostream& out) {
//    out << static_cast<Value>(value);
//}

// Другие перегрузки функции PrintValue пишутся аналогично

void PrintNode(const Node& node, std::ostream& out) {
    std::visit(
        [&out](const auto& value){ PrintValue(value, out); },
        node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    Node root_node = doc.GetRoot();
    PrintNode(root_node,output);
    //(void) &output;

    // Реализуйте функцию самостоятельно
}

}  // namespace json
