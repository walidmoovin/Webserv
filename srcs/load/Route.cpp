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
std::vector<string> Route::getIndexsLst(void) { return _indexs; }

string Route::getIndex(string path) {
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
		content << path << " files :\n";
		while ((entry = readdir(dir)) != NULL) {
			if (entry->d_name[0] == '.')
				continue;
			for (it = indexs.begin(); it < indexs.end(); it++) {
				if (entry->d_name == *it)
					return (read_file(path + "/" + *it));
			}
			content << "- " << entry->d_name << "\n";
			if (stat(path.c_str(), &info) != 0)
				std::cerr << "stat() error on " << path << ": "
						  << strerror(errno) << "\n";
		}
		closedir(dir);
	}
	ret << "Content-type: text/html \n";
	ret << "Content-length: "<< content.str().length();
	ret << "\n\n" << content;
	return ret.str();
}

string Route::read_file(string path) {
	string str;
	string content;
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
		content += str + "\n";
	}
	ret << "Content-type: " << getMime(path) << "\n";
	ret << "Content-length: " << content.length();
	ret << "\n\n" << content;
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
