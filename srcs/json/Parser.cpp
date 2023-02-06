#include "webserv.hpp"

JSONParser::JSONParser(const string filename) : tokenizer(filename) {}

JSONNode *JSONParser::parse() {
	string		key = "";
	JSONNode *parsed = NULL;
	Token			token;
	token = tokenizer.getToken();
	if (token.type == CURLY_OPEN) parsed = parseObject();
	else throw std::logic_error("Invalid json syntax: json file must be an unique object block");
	return parsed;
}

JSONNode *JSONParser::parseObject() {
	JSONNode	 *node = new JSONNode;
	JSONObject *keyObjectMap = new JSONObject;
	node->setObj(keyObjectMap);
	try {
		while (1) {
			if (!tokenizer.hasMoreTokens()) {
				delete node;
				throw std::logic_error("No more tokens");
			}
			Token token = tokenizer.getToken();
			if (token.type != STRING) throw std::logic_error("Invalid json syntax: Unclosed string");
			string key = token.value;
			token = tokenizer.getToken();
			if (token.type != COLON) throw std::logic_error("Invalid json syntax: missing colon");
			token = tokenizer.getToken();
			switch (token.type) {
			case CURLY_OPEN: {
				(*keyObjectMap)[key] = parseObject();
				break;
			}
			case ARRAY_OPEN: {
				(*keyObjectMap)[key] = parseList();
				break;
			}
			case STRING: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseString();
				break;
			}
			case NUMBER: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseNumber();
				break;
			}
			case BOOLEAN: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseBoolean();
				break;
			}
			case NULL_TYPE: {
				(*keyObjectMap)[key] = parseNull();
				break;
			}
			default:
				throw std::logic_error("Invalid json syntax: Invalid object member type");
				break;
			}
			token = tokenizer.getToken();
			if (token.type == CURLY_CLOSE) break;
		}
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
JSONNode *JSONParser::parseList() {
	JSONNode *node = new JSONNode();
	JSONList *list = new JSONList();
	node->setLst(list);

	try {
		bool hasCompleted = false;
		while (!hasCompleted) {
			if (!tokenizer.hasMoreTokens()) {
				delete node;
				throw std::logic_error("No more tokens");
			}
			Token			token = tokenizer.getToken();
			JSONNode *subNode;
			switch (token.type) {
			case CURLY_OPEN: {
				subNode = parseObject();
				break;
			}
			case ARRAY_OPEN: {
				subNode = parseList();
				break;
			}
			case STRING: {
				tokenizer.rollBackToken();
				subNode = parseString();
				break;
			}
			case NUMBER: {
				tokenizer.rollBackToken();
				subNode = parseNumber();
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
				throw std::logic_error("Invalid json syntax: Invalid list member type");
				break;
			}
			list->push_back(subNode);
			token = tokenizer.getToken();
			if (token.type == ARRAY_CLOSE) { hasCompleted = true; }
		}
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
JSONNode *JSONParser::parseString() {
	JSONNode *node = new JSONNode();
	try {
		Token		token = tokenizer.getToken();
		string *sValue = new string(token.value);
		node->setStr(sValue);
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
JSONNode *JSONParser::parseNumber() {
	JSONNode *node = new JSONNode();
	try {
		Token	 token = tokenizer.getToken();
		string value = token.value;
		int		 nbr = std::atoi(value.c_str());
		node->setNbr(nbr);
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
JSONNode *JSONParser::parseBoolean() {
	JSONNode *node = new JSONNode();
	try {

		Token token = tokenizer.getToken();
		node->setBoo(token.value == "True" ? true : false);
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
JSONNode *JSONParser::parseNull() {
	JSONNode *node = new JSONNode();
	try {
		node->setNull();
		return node;
	} catch (std::exception &e) {
		delete node;
		throw std::logic_error(e.what());
	}
}
