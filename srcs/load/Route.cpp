#include "webserv.hpp"

Route::Route(JSONNode *datas) {
	JSONObject object = datas->obj();
	if (object["root"])
		_root = object["root"]->str();
	if (object["return"])
		_ret = object["return"]->str();
	if (object["autoindex"])
		_autoindex = object["autoindex"]->boo();
	if (object["indexs"]) {
		JSONList indexs = object["indexs"]->lst();
		for (JSONList::iterator it = indexs.begin(); it < indexs.end(); it++) {
			_indexs.push_back((*it)->str());
		}
	}
}

Route::~Route(void) {}

string Route::getRoot(void) { return _root; }
string Route::getReturn(void) { return _ret; }
std::vector<string> Route::getIndexs(void) { return _indexs; }

string Route::getAutoindex(string uri) {
	if (!_autoindex)
		return "4\n\n404!";
	std::stringstream page;
	std::stringstream ret;
	string path = correctUri(uri);
	DIR *dir;
	struct dirent *entry;
	struct stat info;

	if ((dir = opendir(path.c_str())) == NULL)
		ret << " 19\n\nFolder unaccesible.";
	else {
		page << path << " files :\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			page << "- " << entry->d_name << "\n";
			if (stat(path.c_str(), &info) != 0)
				std::cerr << "stat() error on " << path << ": "
						  << strerror(errno) << "\n";
		}
		closedir(dir);
	}
	ret << page.str().length() << "\n\n" << page.str();
	return ret.str();
}

string Route::correctUri(string uri) {
	std::stringstream ret;
	int slash_pos;
	string root = _root;

	cout << "Correcting request: " << uri
		 << " with root: " << root << "\n";
	int i = 0;
	while (uri.at(i) == '/')
		i++;
	uri.erase(0, i);
	while ((slash_pos = root.find('/')) > 0) {
		ret << root.substr(0, slash_pos);
		root.erase(0, slash_pos);
		uri = uri.substr(uri.find('/'), uri.length());
	}
	ret << uri;
	cout << "resutlt: " << ret.str() << "\n";
	return ret.str();
}
