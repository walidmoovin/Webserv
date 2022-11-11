#include "webserv.hpp"

Client::Client(int fd, listen_t listen, Master *parent)
	: _fd(fd), _listen(listen), _parent(parent) {
	cout << "New connection, socket fd is " << fd << ", ip is : " << _listen.ip
		 << ", port : " << _listen.port << "\n";
}
Client::~Client(void) {
	close(_fd);
	cout << "Host disconnected, ip " << _listen.ip << ", port " << _listen.port
		 << "\n";
}

bool Client::getRequest(string paquet) {
	cout << "|===|paquet|===>" << paquet << "|===||\n";
	if (paquet.length() < 1) // HTTPS?
		return false;
	std::vector< string > lines = split(paquet, '\n');
	long				  chunk_len =
		 (_content.length() > 0 && _request["Transfer-Encoding:"].size() &&
		  _request["Transfer-Encoding:"].at(0) == "chunked")
							 ? std::strtol(lines.at(0).substr(1).c_str(), 0, 16)
							 : -1;
	cout << "Chunk length: " << chunk_len << "\n";
	for (std::vector< string >::iterator it = lines.begin(); it < lines.end();
		 it++) {
		if (*it == "\r" && _content.length() == 0)
			this->parseHeader();
		if (*it != "\r" && _content.length() == 0)
			_header += *it + "\n";
		else if (chunk_len == -1 || it != lines.begin())
			_content += *it + "\n";
	}
	cout << "Header: \n-|" << _header << "|-\n";
	cout << "Content: \n-|" << _content << "|-\n";
	if (_content.length() > 0) {
		_content.resize(_content.length() - 1);
		if (_request["Method:"].at(0) == "GET" ||
			(chunk_len == 0 ||
			 std::strtoul(_request["Content-Length:"].at(0).c_str(), 0, 10) <=
				 _content.length())) {
			cout << "Request received\n";
			return true;
		}
	} else
		_header.resize(_header.length() - 1);
	return false;
}

bool Client::parseHeader() {
	std::vector< string > lines = split(_header, '\n');
	std::vector< string > line;
	if (lines.size() > 0) {
		for (std::vector< string >::iterator it = lines.begin() + 1;
			 it < lines.end(); it++) {
			line = split(*it, ' ');
			_request[line.at(0)] =
				std::vector< string >(line.begin() + 1, line.end());
		}
	}
	std::vector< string > method = split(lines.at(0), ' ');
	if (method.at(0) == "POST" &&
		_request.find("Content-Length:") == _request.end() &&
		_request.find("Transfer-Encoding:") == _request.end())
		return false;
	_request["Method:"] = method;
	return true;
}

bool Client::check_method(Server *server, Route *route, string method) {
	std::vector< string > allowed;
	if (method != "GET" && method != "POST" && method != "DELETE")
		send_error(405);
	else if ((allowed = route->_headers).size() > 0 &&
			 std::find(allowed.begin(), allowed.end(), method) ==
				 allowed.end()) {
		send_error(405);
	} else if ((allowed = server->_headers).size() > 0 &&
			   std::find(allowed.begin(), allowed.end(), method) ==
				   allowed.end()) {
		send_error(405);
	} else
		return (true);
	return (false);
}

string Client::header_pick(string key, int id) {
	if (_request[key].size() == 0)
		throw std::runtime_error("The key is not in request's header.");
	string ret = _request[key].at(id);
	return ret;
}

inline string get_extension(string str) { return str.substr(str.rfind('.')); }

void Client::answer(Env *env) {
	string method = header_pick("Method:", 0);
	string uri = header_pick("Method:", 1);
	string host = header_pick("Host:", 0);
	cout << "Method: " << method << "\n";
	cout << "URI: " << uri << "\n";
	cout << "Host: " << host << "\n";
	string ret;

	Server *server = _parent->choose_server(env, host);
	Route  *route = server->choose_route(uri);
	if (check_method(server, route, method)) {
		string path = route->correctUri(uri);
		string cgi = route->_cgi.size() ? route->_cgi[get_extension(path)] : "";
		if (cgi == "") {
			if ((ret = route->getIndex(uri, path)) == "" &&
				(ret = read_file(path)) == "")
				send_error(404);
			else
				send_answer("HTTP/1.1 200 OK\r\n" + ret);
		} else
			send_cgi(cgi, path);
	}
	_content = "";
	_header = "";
}

void Client::send_cgi(string cgi, string path) {
	int				  status;
	int				  fd[2];
	std::stringstream ss;
	string			  ret;

	if (!std::ifstream(cgi.c_str()).good())
		return send_error(404);
	pipe(fd);
	int pid = fork();
	if (pid == 0) {
		const char **args = new const char *[cgi.length() + path.length() + 2];
		args[0] = cgi.c_str();
		args[1] = path.c_str();
		args[2] = NULL;
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		execve(cgi.c_str(), (char **)args, NULL);
	}
	close(fd[1]);
	waitpid(pid, &status, 0);
	char buffer[10000];
	buffer[read(fd[0], buffer, 10000)] = 0;
	ret = string(buffer);
	ss << "HTTP/1.1 200 OK\r\nContent-length: "
	   << ret.length() - ret.find("\r\n\r\n") - 4 << "\r\n"
	   << ret;
	send_answer(ss.str());
}

void Client::send_error(int error_code) {
	switch (error_code) {
	case 404:
		return send_answer(
			"HTTP/1.1 404 Not Found\r\nContent-length: 0\r\n\r\n");
	case 405:
		return send_answer(
			"HTTP/1.1 405 Method Not Allowed\r\nContent-length: 0\r\n\r\n");
	}
}

void Client::send_answer(string msg) {
#ifdef __linux__
	send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(_fd, msg.c_str(), msg.length(), 0);
#endif
}
