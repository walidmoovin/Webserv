#pragma once
#include "webserv.hpp"

class Env {
  public:
	std::vector<Server *> _servers;
	std::vector<Socket *> _sockets;
	Env(JSONNode *conf);
	void set_fds(void);
	void refresh(void);
	Server *choose_server(Socket *sock, string host);
	~Env();
};
