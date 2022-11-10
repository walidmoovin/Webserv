#pragma once
#include "webserv.hpp"

class Server : public Route {
	string _name;
	std::map<string, Route *> _routes;
  public:
  std::vector<listen_t> _listens;
	Server(JSONNode *server);
	~Server(void);
  std::vector<Socket *> get_sockets(JSONNode *server);
	Route *choose_route(string uri);
	string getName(void);
};
