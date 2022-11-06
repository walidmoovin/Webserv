#include "../webserv.hpp"
#include "Route.hpp"
#include "Socket.hpp"


class Server {
	string				_name;
	std::vector<Socket>	_sockets;
	std::map<string, Route *> _routes;

  public:
	Server(JSONNode *server) {
        JSONObject datas = server->obj();
		if (datas["server_name"])
			_name = datas["server_name"]->str();
		if (datas["listens"]) {
			JSONList listens = datas["listens"]->lst();
			for (JSONList::iterator i = listens.begin(); i < listens.end(); i++) {
				//_listens.push_back((*i)->str());
				Socket sock((*i)->str());
				_sockets.push_back(sock);
			}
			//_port = std::atoi(_listens.front().c_str());
		}
		//_routes["default"] = new Route(datas["root"], datas["return"], datas["index"], datas["autoindex"]);
	}

	void check() {
		for (std::vector<Socket>::iterator it = _sockets.begin(); it < _sockets.end(); it++){
			(*it).check();
		}
	}
};
