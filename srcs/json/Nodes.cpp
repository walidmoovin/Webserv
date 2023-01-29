#include "webserv.hpp"

/*
- In the parser, a node is created and added to the object map.
- The node is then passed to the parse function for the value.
*/

JSONObject JSONNode::obj() {
	if (type == OBJECT) return *values.object;
	throw std::logic_error("Improper return");
}
JSONList JSONNode::lst() {
	if (type == LIST) return *values.list;
	throw std::logic_error("Improper return");
}
string JSONNode::str() {
	if (type == STRING) return *values.str;
	throw std::logic_error("Improper return");
}
int JSONNode::nbr() {
	if (type == NUMBER) return values.nbr;
	throw std::logic_error("Improper return");
}
bool JSONNode::boo() {
	if (type == BOOLEAN) return values.bValue;
	throw std::logic_error("Improper return");
}

void JSONNode::setObj(JSONObject *object) {
	this->values.object = object;
	type = OBJECT;
}
void JSONNode::setLst(JSONList *list) {
	this->values.list = list;
	type = LIST;
}
void JSONNode::setStr(string *str) {
	this->values.str = str;
	type = STRING;
}
void JSONNode::setNbr(int nbr) {
	this->values.nbr = nbr;
	type = NUMBER;
}
void JSONNode::setBoo(bool v) {
	this->values.bValue = v;
	type = BOOLEAN;
}
void JSONNode::setNull() { type = NULL_TYPE; }

/**
 * @brief Destructor
 * Destroy Node and all sub Nodes he contain.
 */
JSONNode::~JSONNode(void) {
	switch (type) {
	case OBJECT:
		for (JSONObject::iterator it = values.object->begin(); it != values.object->end(); it++) { delete it->second; }
		delete values.object;
		break;
	case LIST:
		for (JSONList::iterator it = values.list->begin(); it != values.list->end(); it++) { delete *it; }
		delete values.list;
		break;
	case STRING:
		delete values.str;
		break;
	default:
		break;
	}
}