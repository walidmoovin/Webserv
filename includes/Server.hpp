#pragma once
#include "webserv.hpp"

class Server {
	string				_name;
	std::vector<Socket *>	_sockets;
	std::map<string, Route *> _routes;

  public:
	Server(JSONNode *server);
	~Server();
	void set_fds();
	void refresh();
};
