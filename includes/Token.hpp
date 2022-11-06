#pragma once
#include "webserv.hpp"

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
} Token;

class Tokenizer {
	std::fstream file;
	size_t prevPos;

  public:
	Tokenizer(string fileName);
	bool hasMoreTokens();
	char getWithoutWhiteSpace();
	void rollBackToken();
	Token getToken();
};
