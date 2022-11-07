#pragma once
#include "webserv.hpp"

class Env {
	std::vector<Server *> _servers;
  std::vector<Socket *> _sockets;

  public:
	Env(JSONNode *conf);
	void set_fds();
	void refresh();
  Server *choose_server(Socket *sock, string host);
};
