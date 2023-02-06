#pragma once
#include "webserv.hpp"

class Route {
protected:
	Server	*_server;
	string	_location, _root;
	bool	_autoindex;

public:
	vec_string					_indexs, _allowed_methods;
	std::map<string, string>	_cgi;
	int							_client_max_body_size, _timeout, _max_requests, _ret_code;
	string 						_upload_folder, _ret_uri;
	std::map<int, string>		_err_page;

	Route(Server *server, string location, JSONNode *datas);
	~Route(void);
	string	getLocation(void);
	string	get_custom_err(int error_code);
	string	getRoot(void);
	string	getReturn(void);
	Server	*getServer(void);
	string	getIndex(string uri, string path);
	string	correctUri(string uri);
};
