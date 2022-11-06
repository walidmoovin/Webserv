#include "webserv.hpp"

Server::Server(JSONNode *server) {
    JSONObject datas = server->obj();
    if (datas["server_name"])
        _name = datas["server_name"]->str();
    if (datas["listens"]) {
        JSONList listens = datas["listens"]->lst();
        for (JSONList::iterator i = listens.begin(); i < listens.end(); i++) {
            Socket *sock = new Socket((*i)->str());
			if (sock->launch() == EXIT_SUCCESS) 
            	_sockets.push_back(sock);
			else
				delete sock;
        }
    }
    //_routes["default"] = new Route(datas["root"], datas["return"],
    // datas["index"], datas["autoindex"]);
}

Server::~Server() {
	cout << "Server destroyed!\n";
}

void Server::set_fds() {
    for (std::vector<Socket *>::iterator it = _sockets.begin();
         it < _sockets.end(); it++) {
        (*it)->set_fds();
    }
}

void Server::refresh() {
    for (std::vector<Socket *>::iterator it = _sockets.begin();
         it < _sockets.end(); it++) {
        (*it)->refresh();
    }
}
