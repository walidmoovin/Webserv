#include "webserv.hpp"


JSONParser::JSONParser(const string filename) : tokenizer(filename) {}

JSONNode *JSONParser::parse() {
    string key = "";
    JSONNode *parsed;
    while (tokenizer.hasMoreTokens()) {
        Token token;
        try {
            token = tokenizer.getToken();
            switch (token.type) {
            case CURLY_OPEN: {
                parsed = parseObject();
                break;
            }
            case ARRAY_OPEN: {
                parsed = parseList();
                break;
            }
            case STRING: {
                tokenizer.rollBackToken();
                parsed = parseString();
                break;
            }
            case NUMBER: {
                tokenizer.rollBackToken();
                parsed = parseNumber();
                break;
            }
            case BOOLEAN: {
                tokenizer.rollBackToken();
                parsed = parseBoolean();
                break;
            }
            default:
                break;
            }
        } catch (std::logic_error &e) {
            break;
        }
    }
    return parsed;
}

JSONNode *JSONParser::parseObject() {
    JSONNode *node = new JSONNode;
    JSONObject *keyObjectMap = new JSONObject;
    while (1) {
        if (tokenizer.hasMoreTokens()) {
            Token token = tokenizer.getToken();
            string key = token.value;
            tokenizer.getToken();
            token = tokenizer.getToken();
            switch (token.type) {
            case CURLY_OPEN: {
                if (DEBUG)
                    cout << "=object=|" << key << "|===>>\n";
                (*keyObjectMap)[key] = parseObject();
                if (DEBUG)
                    cout << "<<===end object\n";
                break;
            }
            case ARRAY_OPEN: {
                if (DEBUG)
                    cout << "-list-|" << key << "|--->>\n";
                (*keyObjectMap)[key] = parseList();
                if (DEBUG)
                    cout << "<<---end list\n";
                break;
            }
            case STRING: {
                tokenizer.rollBackToken();
                (*keyObjectMap)[key] = parseString();
                if (DEBUG)
                    cout << key << "='" << (*keyObjectMap)[key]->str() << "'\n";
                break;
            }
            case NUMBER: {
                tokenizer.rollBackToken();
                (*keyObjectMap)[key] = parseNumber();
                if (DEBUG)
                    cout << key << "=" << (*keyObjectMap)[key]->nbr() << "\n";
                break;
            }
            case BOOLEAN: {
                tokenizer.rollBackToken();
                (*keyObjectMap)[key] = parseBoolean();
                if (DEBUG)
                    cout << key << "(BOOL)\n";
                break;
            }
            case NULL_TYPE: {
                (*keyObjectMap)[key] = parseNull();
                break;
            }
            default:
                break;
            }
            token = tokenizer.getToken();
            if (token.type == CURLY_CLOSE)
                break;

        } else {
            throw std::logic_error("No more tokens");
        }
    }
    node->setObject(keyObjectMap);
    return node;
}
JSONNode *JSONParser::parseList() {
    JSONNode *node = new JSONNode();
    JSONList *list = new JSONList();
    bool hasCompleted = false;
    while (!hasCompleted) {
        if (!tokenizer.hasMoreTokens()) {
            throw std::logic_error("No more tokens");
        } else {
            Token token = tokenizer.getToken();
            JSONNode *subNode;
            switch (token.type) {
            case CURLY_OPEN: {
                if (DEBUG)
                    cout << "=object===>>\n";
                subNode = parseObject();
                if (DEBUG)
                    cout << "<<===end object\n";
                break;
            }
            case ARRAY_OPEN: {
                if (DEBUG)
                    cout << "-list--->>\n";
                subNode = parseList();
                if (DEBUG)
                    cout << "<<---end list\n";
                break;
            }
            case STRING: {
                tokenizer.rollBackToken();
                subNode = parseString();
                if (DEBUG)
                    cout << "|'" << subNode->str() << "'";
                break;
            }
            case NUMBER: {
                tokenizer.rollBackToken();
                subNode = parseNumber();
                if (DEBUG)
                    cout << "|" << subNode->nbr();
                break;
            }
            case BOOLEAN: {
                tokenizer.rollBackToken();
                subNode = parseBoolean();
                break;
            }
            case NULL_TYPE: {
                subNode = parseNull();
                break;
            }
            default:
                break;
            }
            list->push_back(subNode);
            token = tokenizer.getToken();
            if (token.type == ARRAY_CLOSE) {
                hasCompleted = true;
            }
        }
    }
    node->setList(list);
    return node;
}
JSONNode *JSONParser::parseString() {
    JSONNode *node = new JSONNode();
    Token token = tokenizer.getToken();
    string *sValue = new string(token.value);
    node->setString(sValue);
    return node;
}
JSONNode *JSONParser::parseNumber() {
    JSONNode *node = new JSONNode();
    Token token = tokenizer.getToken();
    string value = token.value;
    int nbr = std::atoi(value.c_str());
    node->setNumber(nbr);
    return node;
}
JSONNode *JSONParser::parseBoolean() {
    JSONNode *node = new JSONNode();
    Token token = tokenizer.getToken();
    node->setBoolean(token.value == "True" ? true : false);
    return node;
}
JSONNode *JSONParser::parseNull() {
    JSONNode *node = new JSONNode();
    node->setNull();
    return node;
}
