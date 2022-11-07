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
	cout << uri << "\n";
	std::vector<string> req = split(uri, '/');
	std::vector<string> root;
	for (std::map<string, Route *>::iterator rit = _routes.begin();
		 rit != _routes.end(); rit++) {
		root = split((*rit).first, '/');
		std::vector<string>::iterator root_it = root.begin();

		for (std::vector<string>::iterator it = req.begin();
			 it < req.end(); it++) {
			if (*it == "")
				continue ;
			cout << *it << " - " << *root_it << "\n";
			if (*it != *(root_it++))
				break;
			if (root_it == root.end())
				return ((*rit).second);
		}
	}
	return this;
}
