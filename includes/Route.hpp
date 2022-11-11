#pragma once
#include "webserv.hpp"

class Route {
  protected:
	Server *_server;
	string	_location;
	string	_root;
	string	_ret;
	bool	_autoindex;

  public:
	std::vector< string >	   _indexs;
	std::vector< string >	   _headers;
	std::map< string, string > _cgi;

	Route(Server *server, string location, JSONNode *datas);
	~Route(void);
	string	getLocation(void);
	string	getRoot(void);
	string	getReturn(void);
	Server *getServer(void);
	string	getIndex(string uri, string path);
	string	correctUri(string uri);
};
