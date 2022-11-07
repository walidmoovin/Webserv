#include "webserv.hpp"

Server::Server(JSONNode *server) : Route("/", server) {
	JSONObject datas = server->obj();
	if (datas["server_name"])
		_name = datas["server_name"]->str();
	if (datas["locations"]) {
		JSONObject locations = datas["locations"]->obj();
		for (JSONObject::iterator it = locations.begin(); it != locations.end();
			 it++) {
			Route *route = new Route((*it).first, (*it).second);
			_routes[(*it).first] = route;
		}
	}
}

std::vector<Socket *> Server::get_sockets(JSONNode *server) {
	JSONObject datas = server->obj();
	std::vector<Socket *> ret;
	listen_t listen;
	if (datas["listens"]) {
		JSONList listens = datas["listens"]->lst();
		for (JSONList::iterator it = listens.begin(); it != listens.end();
			 it++) {
			listen = get_listen_t((*it)->str());
			cout << listen.ip << ":" << listen.port << " socket creation...\n";
			if (listen.ip.at(0) == '[') {
				cout << "Listen: IPv6 isn't supported\n";
				continue;
			}
			_listens.push_back(listen);
			Socket *sock = new Socket(listen);
			if (sock->launch() == EXIT_SUCCESS)
				ret.push_back(sock);
			else
				delete sock;
		}
	} else {
		listen = get_listen_t("localhost:80");
		Socket *sock = new Socket(listen);
		_listens.push_back(listen);
		if (sock->launch() == EXIT_SUCCESS) {
			ret.push_back(sock);
		}
	}
	return ret;
}

Server::~Server(void) { cout << "Server destroyed!\n"; }

Route *Server::get_route(string uri) {
	cout << uri << "\n";
	std::vector<string> req = split(uri, '/');
	std::vector<string> root;
	for (std::map<string, Route *>::iterator rit = _routes.begin();
		 rit != _routes.end(); rit++) {
		root = split((*rit).first, '/');
		std::vector<string>::iterator root_it = root.begin();

		for (std::vector<string>::iterator it = req.begin(); it < req.end();
			 it++) {
			if (*it == "")
				continue;
			cout << *it << " - " << *root_it << "\n";
			if (*it != *(root_it++))
				break;
			if (root_it == root.end())
				return ((*rit).second);
		}
	}
	return this;
}
