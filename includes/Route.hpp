#pragma once
#include "webserv.hpp"

class Route {
protected:
	Server *_server;
	string	_location, _root, _ret;
	bool		_autoindex;

public:
	vec_string							 _indexs, _allowed_methods;
	std::map<string, string> _cgi;
	int											 _client_max_body_size;

	Route(Server *server, string location, JSONNode *datas);
	~Route(void);
	string	getLocation(void);
	string	getRoot(void);
	string	getReturn(void);
	Server *getServer(void);
	string	getIndex(string uri, string path);
	string	correctUri(string uri);
};
