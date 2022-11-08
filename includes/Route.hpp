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
	std::vector<string> getIndexsLst(void);
	string getIndex(string path);
	string read_file(string path);
	string correctUri(string uri);
	string getMime(string path);
};
