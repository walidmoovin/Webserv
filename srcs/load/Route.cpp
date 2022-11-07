#include "webserv.hpp"

Route::Route(string location, JSONNode *datas) : _location(location) {
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
}

Route::~Route(void) {}

string Route::getLocation(void) { return _location; }
string Route::getRoot(void) { return _root; }
string Route::getReturn(void) { return _ret; }
std::vector<string> Route::getIndexs(void) { return _indexs; }

string Route::getAutoindex(string path) {
	if (!_autoindex)
		return "";
	std::stringstream page;
	DIR *dir;
	struct dirent *entry;
	struct stat info;

	if ((dir = opendir(path.c_str())) == NULL)
		return "";
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
	return page.str();
}

string Route::read_file(string path) {
	string str;
	std::stringstream ret;
	struct stat info;
	if (stat(path.c_str(), &info) != 0) {
		std::cerr << "stat() error on " << path << ": "
			<< strerror(errno) << "\n";
		return "";
	}
	std::ifstream file(path.c_str());
	while (file) {
		std::getline(file, str);
		ret << str << "\n";
	}
	return (ret.str());
}

string Route::correctUri(string uri) {
	std::stringstream ret;
	std::vector<string>::iterator it;
	std::vector<string>::iterator it2;

	cout << "Correcting request: " << uri << " with root: " << _root << "\n";
	ret << _root;
	std::vector<string> loc_split = split(_location, '/');
	std::vector<string> uri_split = split(uri, '/');
	it2 = uri_split.begin();
	for (it = loc_split.begin(); it < loc_split.end(); it++) {
		while (*it2 == "")
			it2++;
		while (*it == "")
			it++;
		it2++;
	}

	while (it2 < uri_split.end()) {
		ret << "/" << *(it2++);
	}
	cout << "resutlt: " << ret.str() << "\n";
	return ret.str();
}
