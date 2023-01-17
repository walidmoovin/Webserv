#pragma once
#include "webserv.hpp"

class Server : public Route {
	string										_name;	 ///< The server name
	std::map<string, Route *> _routes; ///< The server's routings with a route object as object and his location as key.

public:
	std::vector<ip_port_t> _listens; ///< The list of listens the server which are linked to the server.
	Server(JSONNode *server);
	~Server(void);
	Master							 *create_master(string str);
	std::vector<Master *> get_sockets(JSONNode *server);
	Route								 *choose_route(string uri);
	string								getName(void);
};
