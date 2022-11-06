#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define DEBUG 0

using std::cout;
using std::string;

class JSONNode;
typedef std::map<string, JSONNode *> JSONObject;
typedef std::vector<JSONNode *> JSONList;

class JSONNode {
  public:
    JSONObject obj() {
        if (type == OBJECT)
            return *values.object;
        throw std::logic_error("Improper return");
    }
    JSONList lst() {
        if (type == LIST)
            return *values.list;
        throw std::logic_error("Improper return");
    }
    string str() {
        if (type == STRING)
            return *values.str;
        throw std::logic_error("Improper return");
    }
    int nbr() {
        if (type == NUMBER)
            return values.nbr;
        throw std::logic_error("Improper return");
    }

  private:
    enum Type { OBJECT, LIST, STRING, NUMBER, BOOLEAN, NULL_TYPE };
    union Values {
        JSONObject *object;
        JSONList *list;
        string *str;
        int nbr;
        bool bValue;
    } values;
    Type type;

    void setObject(JSONObject *object) {
        this->values.object = object;
        type = OBJECT;
    }
    void setList(JSONList *list) {
        this->values.list = list;
        type = LIST;
    }
    void setString(string *str) {
        this->values.str = str;
        type = STRING;
    }
    void setNumber(int nbr) {
        this->values.nbr = nbr;
        type = NUMBER;
    }
    void setBoolean(bool v) {
        this->values.bValue = v;
        type = BOOLEAN;
    }
    void setNull() { type = NULL_TYPE; }

    string stringify(int indentationLevel) {
        string spaceString = string(indentationLevel, ' ');
        // sstreams
        std::stringstream output;
        // cout < type << "\n";
        switch (type) {
        case STRING: {
            output << spaceString << *values.str;
            break;
        }
        case NUMBER: {
            output << spaceString << values.nbr;
            break;
        }
        case BOOLEAN: {
            output << spaceString << (values.bValue ? "true" : "false");
            break;
        }
        case NULL_TYPE: {
            output << spaceString << "null";
            break;
        }
        case LIST: {
            // cout << "[";
            output << spaceString << "[\n";
            unsigned int index = 0;
            for (JSONList::iterator i = (*values.list).begin();
                 i != (*values.list).end(); i++) {
                output << (*i)->stringify(indentationLevel + 1);
                if (index < (*values.list).size() - 1) {
                    output << ",\n";
                }
                index++;
            };
            output << "\n" << spaceString << "]\n";
            break;
        }
        case OBJECT: {
            output << spaceString << "{\n";
            for (JSONObject::iterator i = (*values.object).begin();
                 i != (*values.object).end(); i++) {
                output << spaceString << " "
                       << "\"" << i->first << "\""
                       << ": ";
                output << i->second->stringify(indentationLevel + 1);
                JSONObject::iterator next = i;
                next++;
                if ((next) != (*values.object).end()) {
                    output << ",\n";
                }
                output << spaceString << "\n";
            }
            output << spaceString << "}";
            return output.str();
        }
        }
        return output.str();
    }
    friend class JSONParser;
};
