#include "Nodes.hpp"
#include <fstream>

enum TOKEN {
    CURLY_OPEN,
    CURLY_CLOSE,
    COLON,
    STRING,
    NUMBER,
    ARRAY_OPEN,
    ARRAY_CLOSE,
    COMMA,
    BOOLEAN,
    NULL_TYPE
};

typedef struct Token_s {
    string value;
    TOKEN type;
    string toString() {
        switch (type) {
        case CURLY_OPEN: {
            return "Curly open";
        }
        case CURLY_CLOSE: {
            return "Curly close";
        }
        case COLON: {
            return "COLON";
        }
        case NUMBER: {
            return "Number: " + value;
        }
        case STRING: {
            return "String: " + value;
        }

        case ARRAY_OPEN: {
            return "Array open";
        }
        case ARRAY_CLOSE: {
            return "Array close";
        }
        case COMMA: {
            return "Comma";
        }
        case BOOLEAN: {
            return "Boolean: " + value;
        }

        case NULL_TYPE: {
            return "Null";
        }
        default:
            return "Other...";
        }
    }
} Token;

class Tokenizer {
    std::fstream file;
    size_t prevPos;

  public:
    Tokenizer(string fileName) {
        file.open(fileName.c_str(), std::ios::in);
        if (!file.good())
            cout << "File open error"
                 << "\n";
    };
    bool hasMoreTokens() { return !file.eof(); }
    char getWithoutWhiteSpace() {
        char c = ' ';
        while ((c == ' ' || c == '\n') || c == '\t') {
            file.get(c); // check

            if ((c == ' ' || c == '\n') && !file.good()) {
                // cout << file.eof() << " " << file.fail() << "\n";
                throw std::logic_error("Ran out of tokens");
            } else if (!file.good()) {
                return c;
            }
        }

        return c;
    };
    void rollBackToken() {
        if (file.eof())
            file.clear();
        file.seekg(prevPos);
    };
    Token getToken() {
        char c;
        if (file.eof()) {
            cout << "Exhaused tokens" << "\n";
            // throw std::exception("Exhausted tokens");
        }
        prevPos = file.tellg();
        c = getWithoutWhiteSpace();

        Token token;
        token.type = NULL_TYPE;
        if (c == '"') {
            token.type = STRING;
            token.value = "";
            file.get(c);
            while (c != '"') {
                token.value += c;
                file.get(c);
            }
        } else if (c == '{') {
            token.type = CURLY_OPEN;
        } else if (c == '}') {
            token.type = CURLY_CLOSE;
        } else if (c == '-' || (c >= '0' && c <= '9')) {
            // Check if string is numeric
            token.type = NUMBER;
            token.value = "";
            token.value += c;
            std::streampos prevCharPos = file.tellg();
            while ((c == '-') || (c >= '0' && c <= '9') || c == '.') {
                prevCharPos = file.tellg();
                file.get(c);

                if (file.eof()) {
                    break;
                } else {
                    if ((c == '-') || (c >= '0' && c <= '9') || (c == '.')) {
                        token.value += c;
                    } else {
                        file.seekg(prevCharPos);
                    }
                }
            }
        } else if (c == 'f') {
            token.type = BOOLEAN;
            token.value = "False";
            file.seekg(4, std::ios_base::cur);
        } else if (c == 't') {
            token.type = BOOLEAN;
            token.value = "True";
            file.seekg(3, std::ios_base::cur);
        } else if (c == 'n') {
            token.type = NULL_TYPE;
            file.seekg(3, std::ios_base::cur);
        } else if (c == '[') {
            token.type = ARRAY_OPEN;
        } else if (c == ']') {
            token.type = ARRAY_CLOSE;
        } else if (c == ':') {
            token.type = COLON;
        } else if (c == ',') {
            token.type = COMMA;
        }
        // cout << token.type << " : " << token.value << "\n";
        return token;
    };
};
