#include "webserv.hpp"

void *ft_memset(void *b, int c, size_t len) {
	size_t i;
	unsigned char *b_cpy;

	b_cpy = (unsigned char *)b;
	i = 0;
	while (i < len)
		*(unsigned char *)(b_cpy + i++) = (unsigned char)c;
	return ((void *)b);
}

bool isInt(string str) {
	for (string::iterator it = str.begin(); it < str.end(); it++)
		if (*it < '0' || *it > '9')
			return false;
	return true;
}

std::vector<string> split(string str, char delim) {
	std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);
    while (getline(ss, token, delim)){
        tokens.push_back(token);
    }
	return tokens;
}
