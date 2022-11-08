#include "webserv.hpp"

Route::Route(Server *server, string location, JSONNode *datas)
	: _server(server), _location(location) {
	JSONObject object = datas->obj();
	JSONNode *tmp;
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
	if ((tmp = object["add_header"])) {
		JSONList headers = tmp->lst();
		for (JSONList::iterator it = headers.begin(); it < headers.end();
			 it++) {
			_headers.push_back((*it)->str());
		}
	}
}

Route::~Route(void) {}

string Route::getLocation(void) { return _location; }
string Route::getRoot(void) { return _root; }
string Route::getReturn(void) { return _ret; }
std::vector<string> Route::getIndexsLst(void) { return _indexs; }
std::vector<string> Route::getHeadersLst(void) { return _headers; }
    
string Route::getIndex(string uri, string path) {
	std::stringstream content;
	std::stringstream ret;
	DIR *dir;
	struct dirent *entry;
	struct stat info;
	std::vector<string> indexs = getIndexsLst();
	std::vector<string>::iterator it;

	if ((dir = opendir(path.c_str())) == NULL)
		return "";
	else {
		content << "<h3 style=\"text-align: center;\">" << path
				<< " files :</h3>\n<ul>\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			for (it = indexs.begin(); it < indexs.end(); it++) {
				if (entry->d_name == *it)
					return (read_file(path + "/" + *it));
			}
			content << "<li><a href=\"" << uri + "/" + entry->d_name << "\">"
					<< entry->d_name << "</a></li>\n";
			if (stat(path.c_str(), &info) != 0)
				std::cerr << "stat() error on " << path << ": "
						  << strerror(errno) << "\n";
		}
		content << "<ul>";
		closedir(dir);
	}
	if (!_autoindex)
		return "";
	ret << "Content-type: text/html \r\n";
	ret << "Content-length: " << content.str().length() << "\r\n";
	ret << "\r\n" << content.str();
	return ret.str();
}

string Route::correctUri(string uri) {
	std::stringstream ret;
	std::vector<string>::iterator it;
	std::vector<string>::iterator it2;

	// cout << "Correcting request: " << uri << " with root: " << _root << "\n";
	ret << _root;
	std::vector<string> loc_split = split(_location, '/');
	std::vector<string> uri_split = split(uri, '/');
	it2 = uri_split.begin();
	for (it = loc_split.begin(); it < loc_split.end(); it++) {
		while (*it2 == "")
			it2++;
		while (*it == "")
			it++;
		if (it != loc_split.end())
			it2++;
	}

	while (it2 < uri_split.end()) {
		ret << "/" << *(it2++);
	}
	// cout << "resutlt: " << ret.str() << "\n";
	return ret.str();
}
