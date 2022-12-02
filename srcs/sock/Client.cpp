#include "webserv.hpp"

inline string get_extension(string str) { return str.substr(str.rfind('.')); }

Client::Client(int fd, ip_port_t ip_port, Master *parent) : _fd(fd), _ip_port(ip_port), _parent(parent) {
	clean();
	cout << "New connection, socket fd is " << fd << ", ip is : " << _ip_port.ip << ", port : " << _ip_port.port << "\n";
}

Client::~Client(void) {
	close(_fd);
	cout << "Host disconnected, ip " << this->_ip_port.ip << ", port " << this->_ip_port.port << "\n";
}

void Client::clean(void) {
	this->_server = NULL;
	this->_route = NULL;
	this->_method = this->_uri = this->_host = this->_header = this->_body = "";
	this->_len = 0;
	this->_last_chunk = false;
	this->_request.clear();
}

bool Client::getHeader(Env *env, string paquet) {
	if (paquet.length() < 1)
		send_error(403);
	print_block("Paquet: ", paquet);
	if (header_pick("Method:", 0) != "")
		return getBody(paquet);
	vec_string lines = split(paquet, "\r\n");
	for (vec_string::iterator it = lines.begin(); it < lines.end(); it++) {
		size_t pos = paquet.find("\r\n");
		if (pos != string::npos)
			paquet.erase(0, pos + 2);
		else
			paquet.clear();
		_header += *it + (it + 1 != lines.end() ? "\r\n" : "");
		if (_header.find("\r\n\r\n") != string::npos)
			return !this->parseHeader(env)
					   ? false
					   : (_len != 0 ? getBody(paquet) : true);
	}
	return false;
}

bool Client::getBody(string paquet) {
	vec_string			 lines = split(paquet, "\r\n");
	vec_string::iterator it;

	for (it = lines.begin(); it < lines.end(); it++) {
		if (DEBUG)
			cout << "Remaining length: " << _len << "\n";
		if ((*it).length() && _len <= 0 &&
			header_pick("Transfer-Encoding:", 0) == "chunked") {
			_len = std::strtol((*it).c_str(), 0, 16) + 2;
			_last_chunk = _len == 2 ? true : false;
		} else if (_len > 0 || it != lines.begin()) {
			_body += *it + "\r\n";
			_len -= ((*it).length() + 2);
		}
	}
	// if (_body.size())
	_body.resize(_body.length() - 2);
	_len += 2;
	return (_last_chunk && _len == 0) ? true : false;
}

bool Client::parseHeader(Env *env) {
	vec_string lines, method, line;

	if (DEBUG)
		cout << "Parsing header...\n";
	lines = split(_header, "\r\n");
	method = split(lines.at(0), " ");
	_request["Method:"] = method;
	if (lines.size() > 0) {
		for (vec_string::iterator it = lines.begin() + 1; it < lines.end();
			 it++) {
			line = split(*it, " ");
			_request[line.at(0)] = vec_string(line.begin() + 1, line.end());
		}
	}
	_method = header_pick("Method:", 0);
	if ((_method == "POST" || _method == "PUT") &&
		header_pick("Content-Length:", 0) == "" &&
		header_pick("Transfer-Encoding:", 0) != "chunked")
		return (send_error(400), false);
	vec_string uri_split = split(header_pick("Method:", 1), "?");
	_uri = uri_split.at(0);
	if (uri_split.size() > 1)
		_query = uri_split.at(1);
	_host = header_pick("Host:", 0);
	_env	= env;
	_server = _parent->choose_server(env, _host);
	_route = _server->choose_route(_uri);
	if (DEBUG)
		print_header();
	string len = header_pick("Content-Length:", 0).c_str();
	if (len != "") {
		_len = std::atoi(len.c_str());
		_last_chunk = true;
		int max_len = _route->_client_max_body_size >= 0 ? _route->_client_max_body_size : _server->_client_max_body_size >= 0 ? _server->_client_max_body_size : INT_MAX;
		if (_len > max_len)
			return (send_error(413), false);
	} else
		_len = 0;
	return true;
}

bool Client::check_method(void) {
	vec_string allowed;
	if ((_route && (allowed = _route->_allowed_methods).size() > 0) ||
		(_server && (allowed = _server->_allowed_methods).size() > 0) ||
		((allowed = _env->_allowed_methods).size() > 0))
		return std::find(allowed.begin(), allowed.end(), _method) <
					   allowed.end()
				   ? true
				   : false;
	else if (_method == "GET" || _method == "POST" || _method == "DELETE" ||
			 _method == "PUT")
		return (true);
	return (false);
}

string Client::header_pick(string key, size_t id) {
	return _request[key].size() <= id ? "" : _request[key].at(id);
}

void Client::answer(void) {
	print_block("Header: ", _header);
	print_block("Body: ", _body);
	string ret;
	string path = _route->correctUri(_uri);
	string cgi = _route->_cgi.size()	? _route->_cgi[get_extension(path)]
				 : _server->_cgi.size() ? _server->_cgi[get_extension(path)]
										: "";
	if (DEBUG)
		cout << "Path: " << path << "\n";
	if (!check_method())
		send_error(405);
	else {
		if ((ret = _route->getIndex(_uri, path)) == "")
			ret = read_file(path);
		if (ret == "404") {
			if (_method == "POST" || _method == "PUT")
				create_file(path);
			else
				send_error(404);
		}
		else if (ret == "403")
			send_error(403);
		else if (_method == "DELETE")
			std::remove(path.c_str());
		else if (cgi != "")
			send_cgi(cgi, path);
		else
			send_answer("HTTP/1.1 200 OK\r\n" + ret);
	}
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
	if (fork() == 0) {
		const char **args = new const char *[cgi.length() + path.length() + 2];
		args[0] = cgi.c_str();
		args[1] = path.c_str();
		args[2] = NULL;
		string		path_info = "PATH_INFO=" + _route->getRoot();
		string		query = "QUERY_STRING=" + _query;
		const char **env = new const char *[path_info.length() + query.length() + 2];
		env[0] = path_info.c_str();
		env[1] = query.c_str();
		env[2] = NULL;
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		execve(cgi.c_str(), (char **)args, (char **)env);
	}
	close(fd[1]);
	waitpid(-1, &status, 0);
	char buffer[10000];
	buffer[read(fd[0], buffer, 10000)] = 0;
	ret = string(buffer);
	ss << "HTTP/1.1 200 OK\r\nContent-Length: "
	   << ret.length() - ret.find("\r\n\r\n") - 4 << "\r\n"
	   << ret;
	send_answer(ss.str());
}

void Client::send_redir(int redir_code, string opt) {
	switch (redir_code) {
	case 301:
		return send_answer(
			"HTTTP/1.1 301 Moved Permanently\r\nLocation: " + opt + "\r\n\r\n");
	}
}

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
#ifdef DEBUG
	print_block("ANSWER: ", msg);
#endif
#ifdef __linux__
	send(this->_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(this->_fd, msg.c_str(), msg.length(), 0);
#endif
	clean();
}
