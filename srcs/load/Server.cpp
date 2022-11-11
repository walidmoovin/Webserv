#include "webserv.hpp"
/*|===========|
 * Server destructor:
 *
 * delete all routes owned by the server;
 */

Server::~Server(void) {
	for (std::map< string, Route * >::iterator it = _routes.begin();
		 it != _routes.end(); it++)
		delete (*it).second;
	cout << "Server destroyed!\n";
}
/*|===========|
 * Server constructor:
 *
 * Input: A server block node given by JSONParser.
 * Output: A Server class object and also a Route one as Server herite from
 * Route. The Route constructor scrap the routing informations (index, root,
 * autoindex ...) and the Server one the others ones (server_name, sub-routes)
 */

Server::Server(JSONNode *server) : Route(NULL, "/", server) {
	JSONObject datas = server->obj();
	if (datas["server_name"])
		_name = datas["server_name"]->str();
	if (datas["locations"]) {
		JSONObject locations = datas["locations"]->obj();
		for (JSONObject::iterator it = locations.begin(); it != locations.end();
			 it++) {
			Route *route = new Route(this, (*it).first, (*it).second);
			_routes[(*it).first] = route;
		}
	}
}

/* Get the server name (_server_name)*/

string Server::getName(void) { return _name; }
/* |==========|
 * Safely create a master socket:
 *
 * Input: a "ip:port" string
 * Output: a Master socket or NULL if creation failed
 */

Master *Server::create_master(string str) {
	listen_t listen = get_listen_t(str);
	if (listen.ip.at(0) == '[') {
		cout << "Listen: IPv6 isn't supported\n";
	}
	try {
		_listens.push_back(listen);
		Master *sock = new Master(listen);
		return (sock);
	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		return NULL;
	}
}
/*|===========|
 * Create server's defined sockets:
 *
 * Input: A server block node from JSONParser.
 * Output: A vector containing all the succesfull created sockets using listens
 * from the server block.
 */

std::vector< Master * > Server::get_sockets(JSONNode *server) {
	JSONObject				datas = server->obj();
	std::vector< Master * > ret;
	listen_t				listen;
	Master				   *tmp;
	if (datas["listens"]) {
		JSONList listens = datas["listens"]->lst();
		for (JSONList::iterator it = listens.begin(); it != listens.end();
			 it++) {
			if ((tmp = create_master((*it)->str())))
				ret.push_back(tmp);
		}
	} else if ((tmp = create_master("0.0.0.0")))
		ret.push_back(tmp);
	return ret;
}
/*|===========|
 * Choose the route an uri asked to the server must lead to.
 *
 * Intput: The uri asked by the client to the server.
 * Output: The route object choosen or the server itself if no location block is
 * adapted.
 */

Route *Server::choose_route(string uri) {
	std::vector< string > req = split(uri, '/');
	std::vector< string > root;
	for (std::map< string, Route * >::iterator rit = _routes.begin();
		 rit != _routes.end(); rit++) {
		root = split((*rit).first, '/');
		std::vector< string >::iterator root_it = root.begin();
		for (std::vector< string >::iterator it = req.begin(); it < req.end();
			 it++) {
			if (*it == "")
				continue;
			if (*it != *(root_it++))
				break;
			if (root_it == root.end())
				return ((*rit).second);
		}
	}
	return this;
}
