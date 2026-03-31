#include "json.h"
#include <cstddef>
#include <iostream>
#include<cctype>
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
    if (!input) {
        throw ParsingError("Array parsing error!");
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
        result.insert({std::move(key), LoadNode(input)});
    }
    if (!input) {
        throw ParsingError("Dict parsing error!");
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
        if(std::isdigit(c)==0 && c !='-'){
            std::string line;
            std::getline(input, line);
            if(c =='n' &&line.find("ull") != line.npos){
                return Node(nullptr);
            }
            if(c =='t' && line.find("rue") != line.npos){
                return Node(true);
            }
            if(c =='f' && line.find("alse") != line.npos){
                return Node(false);
            }
        }else{
            input.putback(c);
           // std::string line;
            //std::getline(input, line);
           // cout << line;

            Number num = LoadNumber(input);
            if(std::holds_alternative<int>(num)){
                return Node(get<int>(num));
            }
            if(std::holds_alternative<double>(num)){
                return Node(get<double>(num));
            }
        }


        }

        throw ParsingError("Unknown character parsing error!");

    }



}  // namespace




const Array& Node::AsArray() const {
    if (IsArray()) {
        return get<Array>(value_);
    }
    throw std::logic_error("Not <Map> value!");
}

const Dict& Node::AsMap() const {
    if (IsMap()) {
        return get<Dict>(value_);
    }
    throw std::logic_error("Not <Map> value!");

}


int Node::AsInt() const {
    if (IsInt()) {
        return std::get<int>(value_);
    }
    throw std::logic_error("Not  <int> value!");
}

bool Node::AsBool() const {
    if(IsBool()){
        return get<bool>(value_);
    }
    throw std::logic_error("Not <bool> value!");
}

double Node::AsDouble() const
{
    if (IsInt()) {
        return std::get<int>(value_);
    } else if (IsDouble()) {
        return std::get<double>(value_);
    }

    throw std::logic_error("Not <double> or <int> value!");
}

const string& Node::AsString() const {
   if (IsString()) {
        return get<std::string>(value_);
    }
    throw std::logic_error("Not <double> or <int> value!");
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
void PrintValue(const std::string value, std::ostream& out) {
    using namespace std::literals;
    out << "\""sv;

    for (char c : value) {
        switch (c)
        {
        case '\r':
            out << "\\r"sv;
            break;
        case  '\t':
            out << "\\t"sv;
            break;
        case  '\n':
            out << "\\n"sv;
            break;
        case '"':
            out << "\\\""sv;
            break;
        case '\\':
            out << "\\\\"sv;
            break;
        default:
            out << c;
            break;
        }
    }
    out << "\""sv;
};

void PrintValue(const bool value, std::ostream& out) {
    if (value) {
        out << "true";
        return;
    }
    out << "false";
};

void PrintValue(const Array value, std::ostream& out) {
    out << "[";
    bool first = true;
    for (auto & item : value) {
        if (!first) {
            out << ", ";
        } else {
            first = false;
        }
        PrintNode(item, out);
    }
    out << "]";
    return;
};

//std::map<std::string, Node>;
void PrintValue(const Dict value, std::ostream& out) {
    out << "{";
    bool first = true;
    for (auto iter{value.begin()}; iter != value.end(); ++iter) {
        if (!first) {
            out << ",";
        } else {
            first = false;
        }

        PrintValue(iter -> first, out);
        out << ":";
        PrintNode(iter -> second, out);
    }
    out << "}";
    return;
};

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
