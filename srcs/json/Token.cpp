#include "webserv.hpp"

Tokenizer::Tokenizer(string fileName) {
	file.open(fileName.c_str(), std::ios::in);
	if (!file.good())
		cout << "File open error"
			 << "\n";
}
bool Tokenizer::hasMoreTokens() { return !file.eof(); }

char Tokenizer::getWithoutWhiteSpace() {
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
}
void Tokenizer::rollBackToken() {
	if (file.eof())
		file.clear();
	file.seekg(prevPos);
}
Token Tokenizer::getToken() {
	char c;
	if (file.eof()) {
		cout << "Exhaused tokens"
			 << "\n";
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
}
