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
 * Find either an autoindex or an index into the directory required by request
 *
 * Input: The uri client asked, the real local path to the directory
 * Output: The file or the autoindex page to display
 */

string Route::getIndex(string uri, string path) {
	std::stringstream	 content;
	std::stringstream	 ret;
	DIR					*dir;
	struct dirent		*entry;
	struct stat			 info;
	vec_string::iterator it;

	if ((dir = opendir(path.c_str())) == NULL) {
		return "";
	} else {
		cout << "get index(): path=" << path << "\n";
		content << "<h3 style=\"text-align: center;\">" << path
				<< " files :</h3>\n<ul>\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			for (it = _indexs.begin(); it < _indexs.end(); it++) {
				if (entry->d_name == *it)
					return (read_file(path + "/" + *it));
			}
			content << "<li><a href=\"" << uri + "/" + entry->d_name << "\">"
					<< entry->d_name << "</a></li>\n";
			if (stat(path.c_str(), &info) != 0)
				std::cerr << "stat() error on " << path << ": "
						  << strerror(errno) << "\n";
		}
		content << "</ul>";
		closedir(dir);
	}
	if (!_autoindex)
		return "";
	cout << "Getting autoindex\n";
	ret << "Content-type: text/html \r\n";
	ret << "Content-length: " << content.str().length() << "\r\n";
	ret << "\r\n" << content.str();
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
	vec_string::iterator it;
	vec_string::iterator it2;

	ret << "./" << _root;
	vec_string loc_split = split(_location, "/");
	vec_string uri_split = split(uri, "/");
	it2 = uri_split.begin();
	for (it = loc_split.begin(); it < loc_split.end(); it++) {
		while (it2 < uri_split.end() && *it2 == "")
			it2++;
		while (it < loc_split.end() && *it == "")
			it++;
		if (it != loc_split.end())
			it2++;
	}

	while (it2 < uri_split.end()) {
		ret << "/" << *(it2++);
	}
	return ret.str();
}
