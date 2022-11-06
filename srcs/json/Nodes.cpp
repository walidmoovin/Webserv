#include "webserv.hpp"

#define DEBUG 0

JSONObject JSONNode::obj() {
	if (type == OBJECT)
		return *values.object;
	throw std::logic_error("Improper return");
}
JSONList JSONNode::lst() {
	if (type == LIST)
		return *values.list;
	throw std::logic_error("Improper return");
}
string JSONNode::str() {
	if (type == STRING)
		return *values.str;
	throw std::logic_error("Improper return");
}
int JSONNode::nbr() {
	if (type == NUMBER)
		return values.nbr;
	throw std::logic_error("Improper return");
}
bool JSONNode::boo() {
	if (type == BOOLEAN)
		return values.bValue;
	throw std::logic_error("Improper return");
}

void JSONNode::setObject(JSONObject *object) {
	this->values.object = object;
	type = OBJECT;
}
void JSONNode::setList(JSONList *list) {
	this->values.list = list;
	type = LIST;
}
void JSONNode::setString(string *str) {
	this->values.str = str;
	type = STRING;
}
void JSONNode::setNumber(int nbr) {
	this->values.nbr = nbr;
	type = NUMBER;
}
void JSONNode::setBoolean(bool v) {
	this->values.bValue = v;
	type = BOOLEAN;
}
void JSONNode::setNull() { type = NULL_TYPE; }

string JSONNode::stringify(int indentationLevel) {
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
