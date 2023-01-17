#include "webserv.hpp"

JSONParser::JSONParser(const string filename) : tokenizer(filename) {}

JSONNode *JSONParser::parse() {
	string		key = "";
	JSONNode *parsed = NULL;
	Token			token;
	token = tokenizer.getToken();
	if (token.type == CURLY_OPEN) parsed = parseObject();
	if (!parsed) std::cout << "Configuration file isn't valid\n";
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
			if (token.type != STRING) throw std::logic_error("Invalid json syntax: object key isn't a string");
			string key = token.value;
			token = tokenizer.getToken();
			if (token.type != COLON) throw std::logic_error("Invalid json syntax: missing colon");
			token = tokenizer.getToken();
			switch (token.type) {
			case CURLY_OPEN: {
				if (DEBUG) cout << "=object=|" << key << "|===>>\n";
				(*keyObjectMap)[key] = parseObject();
				if (DEBUG) cout << "<<===end object\n";
				break;
			}
			case ARRAY_OPEN: {
				if (DEBUG) cout << "-list-|" << key << "|--->>\n";
				(*keyObjectMap)[key] = parseList();
				if (DEBUG) cout << "<<---end list\n";
				break;
			}
			case STRING: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseString();
				if (DEBUG) cout << key << "='" << (*keyObjectMap)[key]->str() << "'\n";
				break;
			}
			case NUMBER: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseNumber();
				if (DEBUG) cout << key << "=" << (*keyObjectMap)[key]->nbr() << "\n";
				break;
			}
			case BOOLEAN: {
				tokenizer.rollBackToken();
				(*keyObjectMap)[key] = parseBoolean();
				if (DEBUG) cout << key << "(BOOL)\n";
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
				if (DEBUG) cout << "=object===>>\n";
				subNode = parseObject();
				if (DEBUG) cout << "<<===end object\n";
				break;
			}
			case ARRAY_OPEN: {
				if (DEBUG) cout << "-list--->>\n";
				subNode = parseList();
				if (DEBUG) cout << "<<---end list\n";
				break;
			}
			case STRING: {
				tokenizer.rollBackToken();
				subNode = parseString();
				if (DEBUG) cout << "|'" << subNode->str() << "'";
				break;
			}
			case NUMBER: {
				tokenizer.rollBackToken();
				subNode = parseNumber();
				if (DEBUG) cout << "|" << subNode->nbr();
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
