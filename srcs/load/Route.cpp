/**
 * @file Route.cpp
 * @brief A location class which handle
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-12
 */
#include "webserv.hpp"

/**
 * @brief Constructor
 *
 * A route is an object which define how to handle a request. Each Server is Route inherited and each location block
 * lead to a new Route object.
 *
 * @param server The Server parent of the route. NULL if the object is the server.
 * @param location The uri associatied to the route.
 * @param datas The JSONNode giving configuration.
 */
Route::Route(Server *server, string location, JSONNode *datas) : _server(server), _location(location) {
	JSONObject object = datas->obj();
	JSONNode	*tmp;
	_autoindex = false;
	_client_max_body_size = -1;
	if ((tmp = object["root"])) _root = tmp->str();
	if ((tmp = object["return"])) _ret = tmp->str();
	if ((tmp = object["autoindex"])) _autoindex = tmp->boo();
	if ((tmp = object["indexs"])) {
		JSONList indexs = tmp->lst();
		for (JSONList::iterator it = indexs.begin(); it < indexs.end(); it++) _indexs.push_back((*it)->str());
	}
	if ((tmp = object["allowed_methods"])) {
		JSONList headers = tmp->lst();
		for (JSONList::iterator it = headers.begin(); it < headers.end(); it++) _allowed_methods.push_back((*it)->str());
	}
	if ((tmp = object["cgi"])) {
		JSONObject cgis = tmp->obj();
		for (JSONObject::iterator it = cgis.begin(); it != cgis.end(); it++) _cgi[(*it).first] = (*it).second->str();
	}
	if ((tmp = object["client_max_body_size"])) _client_max_body_size = tmp->nbr();
}

/// @brief Destructor
Route::~Route(void) {}

// Getters
string Route::getLocation(void) { return _location; }
string Route::getRoot(void) { return _root; }
string Route::getReturn(void) { return _ret; }

/**
 * @brief Search for an index while generating autoindex
 *
 * @param uri The uri requested by client.
 * @param path The correct path associated with uri.
 *
 * @return The index content to give to client or an empty string if there is nothing for him.
 */
string Route::getIndex(string uri, string path) {
	std::stringstream		 body, ret;
	DIR									*dir;
	struct dirent				*entry;
	struct stat					 info;
	vec_string::iterator it;

	if ((dir = opendir(path.c_str()))) {
		if (DEBUG) cout << "get index(): path=" << path << "\n";
		body << "<h3 style=\"text-align: center;\">" << path << " files :</h3>\n<ul>\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.') continue;
			for (it = _indexs.begin(); it < _indexs.end(); it++) {
				if (entry->d_name == *it) return (read_file(path + "/" + *it));
			}
			body << "<li><a href=\"" << uri + "/" + entry->d_name << "\">" << entry->d_name << "</a></li>\n";
			if (stat(path.c_str(), &info) != 0) std::cerr << "stat() error on " << path << ": " << strerror(errno) << "\n";
		}
		body << "</ul>";
		closedir(dir);
	}
	if (!dir || !_autoindex) return "";
	if (DEBUG) cout << "Getting autoindex\n";
	ret << "Content-type: text/html \r\n";
	ret << "Content-length: " << body.str().length() << "\r\n";
	ret << "\r\n" << body.str();
	return ret.str();
}

/**
 * @brief Find the local path corresponding to the uri asked by te client.
 *
 * @param uri The uri asked by the client.
 *
 * @return The local path.
 * @deprecated Not used by nginx until config use rewrite keyword.
 */
string Route::correctUri(string uri) {
	std::stringstream		 ret;
	vec_string::iterator loc_word, uri_word;

	vec_string loc_words = split(_location, "/");
	vec_string uri_words = split(uri, "/");
	uri_word = uri_words.begin();
	for (loc_word = loc_words.begin(); loc_word < loc_words.end(); loc_word++) {
		while (uri_word < uri_words.end() && *uri_word == "") uri_word++;
		while (loc_word < loc_words.end() && *loc_word == "") loc_word++;
		if (loc_word != loc_words.end()) uri_word++;
	}
	ret << "./" << _root;
	while (uri_word < uri_words.end()) ret << "/" << *(uri_word++);
	return ret.str();
}
