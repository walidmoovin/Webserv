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
	ip_port_t listen = get_ip_port_t(str);
	if (listen.ip.at(0) != '[') {
		try {
			_listens.push_back(listen);
			Master *sock = new Master(listen);
			return (sock);
		} catch (std::exception &e) {
			std::cerr << e.what() << '\n';
		}
	} else
		cout << "Listen: IPv6 isn't supported\n";
	return NULL;
}
/*|===========|
 * Create server's defined sockets:
 *
 * Input: A server block node from JSONParser.
 * Output: A vector containing all the succesfull created sockets using
 * listens from the server block.
 */

std::vector< Master * > Server::get_sockets(JSONNode *server) {
	JSONObject				datas = server->obj();
	std::vector< Master * > ret;
	Master				   *tmp;
	ip_port_t				listen;
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
 * Output: The route object choosen or the server itself if no location
 * block is adapted.
 */

Route *Server::choose_route(string uri) {
	vec_string uri_words, loc_words;
	uri_words = split(uri, "/");
	for (std::map< string, Route * >::iterator loc_it = _routes.begin();
		 loc_it != _routes.end(); loc_it++) {
		loc_words = split((*loc_it).first, "/");
		vec_string::iterator loc_word = loc_words.begin();
		for (vec_string::iterator uri_word = uri_words.begin();
			 uri_word < uri_words.end(); uri_word++) {
			while (uri_word != uri_words.end() && *uri_word == "")
				uri_word++;
			if (*uri_word != *(loc_word++))
				break;
			while (loc_word != loc_words.end() && *loc_word == "")
				loc_word++;
			if (loc_word == loc_words.end())
				return ((*loc_it).second);
		}
	}
	return this;
}
