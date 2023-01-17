#pragma once
#include "webserv.hpp"

class JSONParser {
	std::fstream file;
	Tokenizer		 tokenizer;

public:
	JSONParser(const string filename);
	JSONNode *parse();
	JSONNode *parseObject();
	JSONNode *parseList();
	JSONNode *parseString();
	JSONNode *parseNumber();
	JSONNode *parseBoolean();
	JSONNode *parseNull();
};
