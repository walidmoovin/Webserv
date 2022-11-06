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

string getRoot(void);
string getReturn(void);
std::vector<string> Route::getIndexs(void) { return _indexs; }
bool getAutoindex(void);
