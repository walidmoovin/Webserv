#include "webserv.hpp"

Server::Server(JSONNode *server) : Route(server) {
	JSONObject datas = server->obj();
	if (datas["server_name"])
		_name = datas["server_name"]->str();
	if (datas["listens"]) {
		JSONList listens = datas["listens"]->lst();
		for (JSONList::iterator it = listens.begin(); it < listens.end();
			 it++) {
			Socket *sock = new Socket(this, (*it)->str());
			if (sock->launch() == EXIT_SUCCESS)
				_sockets.push_back(sock);
			else
				delete sock;
		}
	}
	if (datas["locations"]) {
		JSONObject locations = datas["locations"]->obj();
		for (JSONObject::iterator it = locations.begin(); it != locations.end();
			 it++) {
			Route *route = new Route((*it).second);
			_routes[(*it).first] = route;
		}
	}
}

Server::~Server(void) { cout << "Server destroyed!\n"; }

void Server::set_fds(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++) {
		(*it)->set_fds();
	}
}

void Server::refresh(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++) {
		(*it)->refresh();
	}
}

Route *Server::get_route(string uri) {
	for (std::map<string, Route *>::iterator it = _routes.begin();
		 it != _routes.end(); it++) {
		if (uri == (*it).first)
			return (*it).second;
	}
	return this;
}
