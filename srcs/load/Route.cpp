#include "webserv.hpp"
/* |==========|
 * Route constructor:
 * A route is a class which is used with each request to know how to handle uri
 * requested They are defined with conf file from locations block for raw routes
 * and from servers blocks for Server class which herite from Route
 *
 * Input: The Server parent for locations blocks, the uri which lead to the
 * route and the JSON node giving conf datas
 * Output: A Route object
 */

Route::Route(Server *server, string location, JSONNode *datas)
	: _server(server), _location(location) {
	JSONObject object = datas->obj();
	JSONNode  *tmp;
	if ((tmp = object["root"]))
		_root = tmp->str();
	if ((tmp = object["return"]))
		_ret = tmp->str();
	if ((tmp = object["autoindex"]))
		_autoindex = tmp->boo();
	if ((tmp = object["indexs"])) {
		JSONList indexs = tmp->lst();
		for (JSONList::iterator it = indexs.begin(); it < indexs.end(); it++) {
			_indexs.push_back((*it)->str());
		}
	}
	if ((tmp = object["allowed_methods"])) {
		JSONList headers = tmp->lst();
		for (JSONList::iterator it = headers.begin(); it < headers.end();
			 it++) {
			_allowed_methods.push_back((*it)->str());
		}
	}
	if ((tmp = object["cgi"])) {
		JSONObject cgis = tmp->obj();
		for (JSONObject::iterator it = cgis.begin(); it != cgis.end(); it++) {
			_cgi[(*it).first] = (*it).second->str();
		}
	}
	if ((tmp = object["client_max_body_size"]))
		_client_max_body_size = tmp->nbr();
}
/* Route destructor */

Route::~Route(void) {}
/* Getters ... */

string Route::getLocation(void) { return _location; }
string Route::getRoot(void) { return _root; }
string Route::getReturn(void) { return _ret; }
/* |==========|
 * Generate the autoindex of folder.
 * If while generating it an index file is found, it return the opened file.
 * Else it finish generating autoindex and return it finally if autoindex is
 * enable on this route.
 *
 * Input: The uri client asked and the real local path to the directory
 * Output: The index or the autoindex page to display
 */

string Route::getIndex(string uri, string path) {
	std::stringstream	 body, ret;
	DIR					*dir;
	struct dirent		*entry;
	struct stat			 info;
	vec_string::iterator it;

	if ((dir = opendir(path.c_str())) == NULL) {
		return "";
	} else {
		// cout << "get index(): path=" << path << "\n";
		body << "<h3 style=\"text-align: center;\">" << path
			 << " files :</h3>\n<ul>\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			for (it = _indexs.begin(); it < _indexs.end(); it++) {
				if (entry->d_name == *it)
					return (read_file(path + "/" + *it));
			}
			body << "<li><a href=\"" << uri + "/" + entry->d_name << "\">"
				 << entry->d_name << "</a></li>\n";
			if (stat(path.c_str(), &info) != 0)
				std::cerr << "stat() error on " << path << ": "
						  << strerror(errno) << "\n";
		}
		body << "</ul>";
		closedir(dir);
	}
	if (!_autoindex)
		return "";
	// cout << "Getting autoindex\n";
	ret << "Content-type: text/html \r\n";
	ret << "Content-length: " << body.str().length() << "\r\n";
	ret << "\r\n" << body.str();
	return ret.str();
}
/* |==========|
 * Correct the uri the client asked
 *
 * Input: The uri
 * Output: The local path corresponding to that uri in the route
 */

string Route::correctUri(string uri) {
	std::stringstream	 ret;
	vec_string::iterator loc_word, uri_word;

	vec_string loc_words = split(_location, "/");
	vec_string uri_words = split(uri, "/");
	uri_word = uri_words.begin();
	for (loc_word = loc_words.begin(); loc_word < loc_words.end(); loc_word++) {
		while (uri_word < uri_words.end() && *uri_word == "")
			uri_word++;
		while (loc_word < loc_words.end() && *loc_word == "")
			loc_word++;
		if (loc_word != loc_words.end())
			uri_word++;
	}
	ret << "./" << _root;
	while (uri_word < uri_words.end())
		ret << "/" << *(uri_word++);
	return ret.str();
}
