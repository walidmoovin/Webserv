#pragma once
#include "webserv.hpp"

class Env {
	std::vector<Server *> _servers;

  public:
	Env(JSONNode *conf);
	void set_fds();
	void refresh();
};
