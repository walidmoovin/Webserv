#pragma once
#include "webserv.hpp"

class Route {
  protected:
	string _location;
	string _root;
	string _ret;
	std::vector<string> _indexs;
	bool _autoindex;

  public:
	Route(string location, JSONNode *datas);
	~Route(void);
	string getLocation(void);
	string getRoot(void);
	string getReturn(void);
	std::vector<string> getIndexs(void);
	string getAutoindex(string uri);
	string correctUri(string uri);
};
