#pragma once
#include "webserv.hpp"

class Route {
	string _root;
	string _ret;
	std::vector<string> _indexs;
	bool _autoindex;
  public:
	Route(JSONNode *datas);
	~Route(void);
	string getRoot(void);
	string getReturn(void);
	std::vector<string> getIndexs(void);
	bool getAutoindex(void);
};
