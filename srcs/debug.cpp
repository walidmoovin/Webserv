#include "webserv.hpp"

void print_block(string name, string content) {
	if (!DEBUG)
		return ;
	string separator = "|==================================================="
					   "===========================|\n";
	cout << name << separator.substr(name.length(), string::npos) << content
		 << "\n"
		 << separator;
}

void Client::print_header(void) {
	cout << "Method: " << _method << "\n";
	cout << "Uri: " << _uri << "\n";
	cout << "Query: " << _query << "\n";
	cout << "Host: " << _host << "\n";
	cout << "Location: " << _route->getRoot() << "\n";
}
