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
listen_t get_listen_t(string listen) {
  listen_t ret;
	size_t sep_pos = listen.rfind(':');

	string tmp = listen.substr(0, sep_pos);
	ret.ip = isInt(tmp) ? "0.0.0.0" : (tmp == "localhost" ? "127.0.0.1" : tmp);
	tmp = listen.substr(sep_pos + 1, listen.length() - sep_pos - 1).c_str();
	ret.port = !isInt(tmp) ? 80 : std::atoi(tmp.c_str());
  return ret;
}
