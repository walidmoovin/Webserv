#include "webserv.hpp"

void print_block(string name, string content) {
	string separator = "|==================================================="
					   "===========================|\n";
	cout << name << separator.substr(name.length(), string::npos) << content
		 << "\n"
		 << separator;
}
