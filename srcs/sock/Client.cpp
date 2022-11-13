#include "webserv.hpp"

Client::Client(int fd, ip_port_t ip_port, Master *parent)
	: _fd(fd), _ip_port(ip_port), _parent(parent) {
	clean();
	cout << "New connection, socket fd is " << fd << ", ip is : " << _ip_port.ip
		 << ", port : " << _ip_port.port << "\n";
}

Client::~Client(void) {
	close(_fd);
	cout << "Host disconnected, ip " << _ip_port.ip << ", port "
		 << _ip_port.port << "\n";
}

void Client::clean(void) {
	_server = NULL;
	_route = NULL;

	_method = "";
	_uri = "";
	_host = "";
	_len = 0;
	_last_chunk = false;

	_header = "";
	_body = "";
	_request.clear();
}

bool Client::getHeader(Env *env, string paquet) {
	if (paquet.length() < 1)
		send_error(403);
	if (header_pick("Method:", 0) != "")
		return getBody(paquet);
	std::vector< string > lines = split(paquet, "\r\n");
	for (std::vector< string >::iterator it = lines.begin(); it < lines.end();
		 it++) {
		size_t pos = paquet.find("\r\n");
		if (pos != string::npos)
			paquet.erase(0, pos + 2);
		else
			paquet.clear();
		_header += *it + (it + 1 != lines.end() ? "\r\n" : "");
		if (_header.find("\r\n\r\n") != string::npos) {
			print_block("Header: ", _header);
			if (!this->parseHeader(env))
				return false;
			if (header_pick("Method:", 0) == "GET" ||
				header_pick("Method:", 0) == "HEAD")
				return true;
			else if (paquet.length() > 0) {
				// cout << "Remaining paquet: " << paquet.length() << "\n";
				return getBody(paquet);
			}
			cout << "next: " << *it << "\n";
			cout << "paquet length remain: " << paquet.length() << "\n";
		}
	}
	return false;
}

bool Client::getBody(string paquet) {
	std::vector< string >			lines = split(paquet, "\r\n");
	std::vector< string >::iterator it;
	cout << paquet << "\n";

	for (it = lines.begin(); it < lines.end(); it++) {
		cout << "line: " << *it << "\n";
		if ((*it).length() && _len <= 0 &&
			header_pick("Transfer-Encoding:", 0) == "chunked") {
			_len = std::strtol((*it).c_str(), 0, 16) + 2;
			_last_chunk = _len == 2 ? true : false;
			// +2 for the final \r\n closing chunk
		} else if (_len > 0 || it != lines.begin()) {
			_body += *it + "\r\n";
			_len -= ((*it).length() + 2);
		}
		cout << "Remaining chunk length: " << _len << "\n";
		cout << "Is it last chunk ? " << _last_chunk << "\n";
	}
	if (_body.size())
		_body.resize(_body.length() - 2);
	_len += 2;
	cout << "Remaining chunk characters: " << _len << "\n";
	if (_last_chunk && _len == 0) {
		print_block("Body: ", _body);
		return true;
	}
	string content_length = header_pick("Content_Length:", 0);
	if (content_length != "" &&
		std::strtoul(content_length.c_str(), 0, 10) <= _body.length()) {
		print_block("Body: ", _body);
		return true;
	}
	return false;
}

bool Client::parseHeader(Env *env) {
	std::vector< string > lines = split(_header, "\r\n");
	std::vector< string > method = split(lines.at(0), " ");
	_request["Method:"] = method;

	std::vector< string > line;
	if (lines.size() > 0) {
		for (std::vector< string >::iterator it = lines.begin() + 1;
			 it < lines.end(); it++) {
			line = split(*it, " ");
			_request[line.at(0)] =
				std::vector< string >(line.begin() + 1, line.end());
		}
	}
	if ((method.at(0) == "POST" || method.at(0) == "PUT") &&
		header_pick("Content-Length:", 0) == "" &&
		header_pick("Transfer-Encoding:", 0) != "chunked") {
		send_error(400);
		return false;
	}

	_method = header_pick("Method:", 0);
	_uri = header_pick("Method:", 1);
	_host = header_pick("Host:", 0);
	_server = _parent->choose_server(env, _host);
	_route = _server->choose_route(_uri);
	string len = header_pick("Content-Length:", 0).c_str();
	if (len != "") {
		_len = std::atoi(len.c_str());
		_last_chunk = true;
		if (_len > _route->_client_max_body_size) {
			send_error(413);
			return false;
		}
	}
	return true;
}

bool Client::check_method() {
	std::vector< string > allowed;
	if (_method != "GET" && _method != "POST" && _method != "DELETE" &&
		_method != "PUT")
		send_error(405);
	else if ((allowed = _route->_allowed_methods).size() > 0) {
		if (std::find(allowed.begin(), allowed.end(), _method) == allowed.end())
			send_error(405);
		else
			return true;
	} else if ((allowed = _server->_allowed_methods).size() > 0) {
		if (std::find(allowed.begin(), allowed.end(), _method) == allowed.end())
			send_error(405);
		else
			return true;
	} else
		return (true);
	return (false);
}

string Client::header_pick(string key, size_t id) {
	if (_request[key].size() <= id)
		return "";
	return _request[key].at(id);
}

inline string get_extension(string str) { return str.substr(str.rfind('.')); }

void Client::answer() {
	cout << "Method: " << _method << "\n";
	cout << "URI: " << _uri << "\n";
	cout << "Host: " << _host << "\n";
	string ret;
	string path = _route->correctUri(_uri);
	string cgi =
		_route->_cgi.size()
			? _route->_cgi[get_extension(path)]
			: (_server->_cgi.size() ? _server->_cgi[get_extension(path)] : "");
	cout << "Path: " << path << "\n";
	if (_method == "PUT")
		create_file(path);
	if (cgi == "") {
		if (check_method()) {
			if ((ret = _route->getIndex(_uri, path)) == "")
				ret = read_file(path);
			if (ret == "404")
				send_error(404);
			else if (ret == "403")
				send_error(403);
			else
				send_answer("HTTP/1.1 200 OK\r\n" + ret);
		}
	} else
		send_cgi(cgi, path);
}

void Client::create_file(string path) {
	std::ofstream file(path.c_str());
	if (!file.good())
		send_error(403);
	else {
		file << _body;
		file.close();
		send_answer("HTTP/1.1 201 Accepted\r\nContent-Length: 0\r\n\r\n");
	}
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
		string		 path_info = "PATH_INFO=" + _route->getRoot();
		const char **env = new const char *[path_info.length() + 1];
		env[0] = path_info.c_str();
		env[1] = NULL;
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		execve(cgi.c_str(), (char **)args, (char **)env);
	}
	close(fd[1]);
	waitpid(pid, &status, 0);
	char buffer[10000];
	buffer[read(fd[0], buffer, 10000)] = 0;
	ret = string(buffer);
	ss << "HTTP/1.1 200 OK\r\nContent-Length: "
	   << ret.length() - ret.find("\r\n\r\n") - 4 << "\r\n"
	   << ret;
	send_answer(ss.str());
}
/*
void Client::send_redir(int redir_code, string opt) {
	switch (redir_code) {
	case 301:
		return send_answer(
			"HTTTP/1.1 301 Moved Permanently\r\nLocation: " + opt +
"\r\n\r\n");
	}
}
*/
void Client::send_error(int error_code) {
	switch (error_code) {
	case 400:
		return send_answer(
			"HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n");
	case 403:
		return send_answer(
			"HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n");
	case 404:
		return send_answer(
			"HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
	case 405:
		return send_answer("HTTP/1.1 405 Method Not Allowed\r\nConnection: "
						   "close\r\nContent-Length: 0\r\n\r\n");
	case 413:
		return send_answer(
			"HTTP/1.1 413 Payload Too "
			"Large\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
	}
}

void Client::send_answer(string msg) {
#ifdef __linux__
	print_block("Answer: ", msg);
	send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(_fd, msg.c_str(), msg.length(), 0);
#endif
	clean();
}
