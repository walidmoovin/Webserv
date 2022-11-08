#pragma once
#include "webserv.hpp"

class Route {
  protected:
	Server *_server;
	string _location;
	string _root;
	string _ret;
	std::vector<string> _indexs;
	std::vector<string> _headers;
	bool _autoindex;

  public:
	Route(Server *server, string location, JSONNode *datas);
	~Route(void);
	string getLocation(void);
	string getRoot(void);
	string getReturn(void);
	std::vector<string> getIndexsLst(void);
	std::vector<string> getHeadersLst(void);
	Server *getServer(void);
	string getIndex(string uri, string path);
	string correctUri(string uri);
};
