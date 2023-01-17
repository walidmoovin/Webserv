/**
 * @file Server.cpp
 * @brief The servers object. One is created for each config server.
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-12
 */

#include "webserv.hpp"

/**
 * @brief Constructor.
 *
 * The Route constructor scrap the routing informations (index, root,
 * autoindex ...) and the Server one the others ones (server_name, sub-routes)
 *
 * @param server A server block JSONNode get by parser.
 *
 */
Server::Server(JSONNode *server) : Route(NULL, "/", server) {
	JSONObject datas = server->obj();
	if (datas["server_name"]) _name = datas["server_name"]->str();
	if (datas["locations"]) {
		JSONObject locations = datas["locations"]->obj();
		for (JSONObject::iterator it = locations.begin(); it != locations.end(); it++) {
			Route *route = new Route(this, (*it).first, (*it).second);
			_routes[(*it).first] = route;
		}
	}
}

/**
 * @brief Destructor.
 *
 * Delete alocated routes.
 */
Server::~Server(void) {
	for (std::map<string, Route *>::iterator it = _routes.begin(); it != _routes.end(); it++) delete (*it).second;
	cout << "Server destroyed!\n";
}

/**
 * @return The server name (server_name)
 */
string Server::getName(void) { return _name; }

/**
 * @brief Master socket safe creation.
 *
 * @param str a "ip:port" string
 *
 * @return a Master socket object or NULL if the creation failed.
 */
Master *Server::create_master(string str) {
	ip_port_t listen = get_ip_port_t(str);
	if (listen.ip.at(0) != '[') {
		try {
			_listens.push_back(listen);
			Master *sock = new Master(listen);
			return (sock);
		} catch (std::exception &e) { std::cerr << e.what() << '\n'; }
	} else cout << "Listen: IPv6 isn't supported\n";
	return NULL;
}

/**
 * @brief Create server's defined sockets:
 *
 * @param server A server block node from JSONParser.
 *
 * @retrn A vector containing all the succesfull created sockets using
 * listens from the server block.
 */
std::vector<Master *> Server::get_sockets(JSONNode *server) {
	JSONObject						datas = server->obj();
	std::vector<Master *> ret;
	Master							 *tmp;
	ip_port_t							listen;
	if (datas["listens"]) {
		JSONList listens = datas["listens"]->lst();
		for (JSONList::iterator it = listens.begin(); it != listens.end(); it++) {
			if ((tmp = create_master((*it)->str()))) ret.push_back(tmp);
		}
	} else if ((tmp = create_master("0.0.0.0"))) ret.push_back(tmp);
	return ret;
}

/**
 * @brief Choose the route an uri asked to the server.
 *
 * @param uri The uri asked by the client to the server.
 *
 * @return The route object choosen or the server itself if no location is found
 */
Route *Server::choose_route(string uri) {
	vec_string uri_words, loc_words;
	uri_words = split(uri, "/");
	for (std::map<string, Route *>::iterator loc_it = _routes.begin(); loc_it != _routes.end(); loc_it++) {
		loc_words = split((*loc_it).first, "/");
		vec_string::iterator loc_word = loc_words.begin();
		for (vec_string::iterator uri_word = uri_words.begin(); uri_word < uri_words.end(); uri_word++) {
			while (uri_word != uri_words.end() && *uri_word == "") uri_word++;
			if (*uri_word != *(loc_word++)) break;
			while (loc_word != loc_words.end() && *loc_word == "") loc_word++;
			if (loc_word == loc_words.end()) return ((*loc_it).second);
		}
	}
	return this;
}
