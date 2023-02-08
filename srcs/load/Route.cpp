#include "webserv.hpp"

/**
 * @brief Constructor
 *
 * - Routes define how requests will be handled
 * - Server inherits from Route class
 * - Each route is associated to a location
 *
 * @param server The Server parent of the route. NULL if the object is the
 * server.
 * @param location The location of the route.
 * @param datas The JSONNode config of the route.
 */
Route::Route(Server *server, string location, JSONNode *datas) : _server(server), _location(location) {
	JSONObject	object = datas->obj();
	JSONNode	*tmp;
	_autoindex = false;
	_client_max_body_size = -1;
	if ((tmp = object["root"])) _root = tmp->str();
	if ((tmp = object["return"])) {
		_ret_code = tmp->lst()[0]->nbr();
		_ret_uri = tmp->lst()[1]->str();
	}
	if ((tmp = object["autoindex"])) _autoindex = tmp->boo();
	if ((tmp = object["upload_folder"])) _upload_folder = tmp->str();
	if ((tmp = object["keepalive_time"])) _timeout = tmp->nbr();
	else _timeout = 0;
	if ((tmp = object["keepalive_requests"])) _max_requests = tmp->nbr();
	else _max_requests = 0;
	if ((tmp = object["error_pages"])) {
		JSONObject pages = tmp->obj();
		for (JSONObject::iterator it = pages.begin(); it != pages.end(); it++) {
			JSONList err = (*it).second->lst();
			for (JSONList::iterator it2 = err.begin(); it2 != err.end(); it2++) _err_page[(*it2)->nbr()] = (*it).first;
		}
	}
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

Route::~Route(void) {}
string Route::getLocation(void) { return _location; }
string Route::getRoot(void) { return _root; }

/**
 * @brief Search for an index while generating autoindex
 *
 * @param uri The uri requested by client.
 * @param path The correct path associated with uri.
 *
 * @return The index content to give to client or an empty string if there is
 * nothing for him.
 */
string Route::getIndex(string uri, string path) {
	std::stringstream		 body, ret;
	DIR									*dir;
	struct dirent				*entry;
	struct stat					 info;
	vec_string::iterator it;

	if ((dir = opendir(path.c_str()))) {
		body << "<h3 style=\"text-align: center;\">" << path << " files :</h3>\n<ul>\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.') continue;
			for (it = _indexs.begin(); it < _indexs.end(); it++) {
				if (entry->d_name == *it){
          closedir(dir);
          return (file_answer(path + "/" + *it));
        } 
			}
			body << "<li><a href=\"" << uri + "/" + entry->d_name << "\">" << entry->d_name << "</a></li>\n";
			if (stat(path.c_str(), &info) != 0) std::cerr << "stat() error on " << path << ": " << strerror(errno) << "\n";
		}
		body << "</ul>";
		closedir(dir);
	}
	if (!dir || !_autoindex) return "";
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
