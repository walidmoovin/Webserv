#pragma once
#include "webserv.hpp"

class Route {
	string _root;
	string _ret;
	std::vector<string> _indexs;
	bool _autoindex;
  public:
	Route();
	~Route();
};
