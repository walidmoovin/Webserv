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

string Route::getMime(string path)
{
	string extension;
	size_t position = path.find('.');
	if (position != string::npos)
		extension = path.substr(position + 1);
	else
		extension = "html";
	if (path.length() == 1)
	{
		path = "/index.html";
		return("text/html\r\n\r\n");
	}
	else if (!(extension.compare("html")) || !(extension.compare("htm")) || !(extension.compare("shtml")))
		return("text/html\r\n\r\n");
	else if (!(extension.compare("css")))
		return("text/css\r\n\r\n");
	else if (!(extension.compare("xml")))
		return("text/xml\r\n\r\n");
	else if (!(extension.compare("gif")))
		return("image/gif\r\n\r\n");
	else if (!(extension.compare("jpeg")) || !(extension.compare("jpg")))
		return("image/jpeg\r\n\r\n");
	else if (!(extension.compare("js")))
		return("application/javascript\r\n\r\n");
	else if (!(extension.compare("atom")))
		return("application/atom+xml\r\n\r\n");
	else if (!(extension.compare("rss")))
		return("application/rss+xml\r\n\r\n");
	else if (!(extension.compare("mml")))
		return("text/mathml\r\n\r\n");
	else if (!(extension.compare("txt")))
		return("text/plain\r\n\r\n");
	else if (!(extension.compare("jad")))
		return("text/vnd.sun.j2me.app-descriptor\r\n\r\n");
	else if (!(extension.compare("wml")))
		return("text/vnd.wap.wml\r\n\r\n");
	else if (!(extension.compare("htc")))
		return("text/x-component\r\n\r\n");
	else if (!(extension.compare("avif")))
		return("image/avif\r\n\r\n");
	else if (!(extension.compare("png")))
		return("image/png\r\n\r\n");
	else if (!(extension.compare("svg")) || !(extension.compare("svgz")))
		return("image/svg+xml\r\n\r\n");
	else if (!(extension.compare("tif")) || !(extension.compare("tiff")))
		return("image/tiff\r\n\r\n");
	else if (!(extension.compare("wbmp")))
		return("image/vnd.wap.wbmp\r\n\r\n");
	else if (!(extension.compare("webp")))
		return("image/webp\r\n\r\n");
	else if (!(extension.compare("ico")))
		return("image/x-icon\r\n\r\n");
	else if (!(extension.compare("jng")))
		return("image/x-jng\r\n\r\n");
	else if (!(extension.compare("bmp")))
		return("image/x-ms-bmp\r\n\r\n");
	else if (!(extension.compare("woff")))
		return("font/woff\r\n\r\n");
	else if (!(extension.compare("woff2")))
		return("font/woff2\r\n\r\n");
	else if (!(extension.compare("jar")) || !(extension.compare("war")) || !(extension.compare("ear")))
		return("application/java-archive\r\n\r\n");
	else if (!(extension.compare("json")))
		return("application/json\r\n\r\n");
	else if (!(extension.compare("hqx")))
		return("application/mac-binhex40\r\n\r\n");
	else if (!(extension.compare("doc")))
		return("application/msword\r\n\r\n");
	else if (!(extension.compare("pdf")))
		return("application/pdf\r\n\r\n");
	else if (!(extension.compare("ps")) || !(extension.compare("eps")) || !(extension.compare("ai")))
		return("application/postscript\r\n\r\n");
	else if (!(extension.compare("rtf")))
		return("application/rtf\r\n\r\n");
	else if (!(extension.compare("m3u8")))
		return("application/vnd.apple.mpegurl\r\n\r\n");
	else if (!(extension.compare("xls")) || !(extension.compare("xlt")) || !(extension.compare("xlm")) || !(extension.compare("xld")) || !(extension.compare("xla")) || !(extension.compare("xlc")) || !(extension.compare("xlw")) || !(extension.compare("xll")))
		return("application/vnd.ms-excel\r\n\r\n");
	else if (!(extension.compare("ppt")) || !(extension.compare("pps")))
		return("application/vnd.ms-powerpoint\r\n\r\n");
	else if (!(extension.compare("wmlc")))
		return("application/vnd.wap.wmlc\r\n\r\n");
	else if (!(extension.compare("kml")))
		return("application/vnd.google-earth.kml+xml\r\n\r\n");
	else if (!(extension.compare("kmz")))
		return("application/vnd.google-earth.kmz\r\n\r\n");
	else if (!(extension.compare("7z")))
		return("application/x-7z-compressed\r\n\r\n");
	else if (!(extension.compare("cco")))
		return("application/x-cocoa\r\n\r\n");
	else if (!(extension.compare("jardiff")))
		return("application/x-java-archive-diff\r\n\r\n");
	else if (!(extension.compare("jnlp")))
		return("application/x-java-jnlp-file\r\n\r\n");
	else if (!(extension.compare("run")))
		return("application/x-makeself\r\n\r\n");
	else if (!(extension.compare("pl")) || !(extension.compare("pm")))
		return("application/x-perl\r\n\r\n");
	else if (!(extension.compare("pdb")) || !(extension.compare("pqr")) || !(extension.compare("prc")) || !(extension.compare("pde")))
		return("application/x-pilot\r\n\r\n");
	else if (!(extension.compare("rar")))
		return("application/x-rar-compressed\r\n\r\n");
	else if (!(extension.compare("rpm")))
		return("application/x-redhat-package-manager\r\n\r\n");
	else if (!(extension.compare("sea")))
		return("application/x-sea\r\n\r\n");
	else if (!(extension.compare("swf")))
		return("application/x-shockwave-flash\r\n\r\n");
	else if (!(extension.compare("sit")))
		return("application/x-stuffit\r\n\r\n");
	else if (!(extension.compare("tcl")) || !(extension.compare("tk")))
		return("application/x-tcl\r\n\r\n");
	else if (!(extension.compare("der")) || !(extension.compare("pem")) || !(extension.compare("crt")))
		return("application/x-x509-ca-cert\r\n\r\n");
	else if (!(extension.compare("xpi")))
		return("application/x-xpinstall\r\n\r\n");
	else if (!(extension.compare("xhtml")) || !(extension.compare("xht")))
		return("application/xhtml+xml\r\n\r\n");
	else if (!(extension.compare("zip")))
		return("application/zip\r\n\r\n");
	else if (!(extension.compare("bin")) || !(extension.compare("exe")) || !(extension.compare("dll")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("deb")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("dmg")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("eot")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("img")) || !(extension.compare("iso")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("msi")) || !(extension.compare("msp")) || !(extension.compare("msm")))
		return("application/octet-stream\r\n\r\n");
	else if (!(extension.compare("mid")) || !(extension.compare("midi")) || !(extension.compare("kar")))
		return("audio/midi\r\n\r\n");
	else if (!(extension.compare("mp3")))
		return("audio/mpeg\r\n\r\n");
	else if (!(extension.compare("ogg")))
		return("audio/ogg\r\n\r\n");
	else if (!(extension.compare("m4a")))
		return("audio/x-m4a\r\n\r\n");
	else if (!(extension.compare("ra")))
		return("audio/x-realaudio\r\n\r\n");
	else if (!(extension.compare("3gpp")) || !(extension.compare("3gp")))
		return("video/3gpp\r\n\r\n");
	else if (!(extension.compare("ts")))
		return("video/mp2t\r\n\r\n");
	else if (!(extension.compare("mp4")))
		return("video/mp4\r\n\r\n");
	else if (!(extension.compare("mpeg")) || !(extension.compare("mpg")))
		return("video/mpeg\r\n\r\n");
	else if (!(extension.compare("mov")))
		return("video/quicktime\r\n\r\n");
	else if (!(extension.compare("webm")))
		return("video/webm\r\n\r\n");
	else if (!(extension.compare("flv")))
		return("video/x-flv\r\n\r\n");
	else if (!(extension.compare("m4v")))
		return("video/x-m4v\r\n\r\n");
	else if (!(extension.compare("mng")))
		return("video/x-mng\r\n\r\n");
	else if (!(extension.compare("asx")) || !(extension.compare("asf")))
		return("video/x-ms-asf\r\n\r\n");
	else if (!(extension.compare("wmv")))
		return("video/x-ms-wmv\r\n\r\n");
	else if (!(extension.compare("avi")))
		return("video/x-msvideo\r\n\r\n");
	else
		return("text/plain\r\n\r\n");
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
	ret << "Content-type: " << this->getMime(path) << "\n";
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
