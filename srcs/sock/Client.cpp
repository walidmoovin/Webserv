/**
 * @file Client.cpp
 * @brief The client sockets class which keep keep clients information and handle answer to them.
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-12
 */
#include "webserv.hpp"

inline string get_extension(string str) {
	size_t pos = str.rfind('.');
	if (pos != string::npos) return str.substr(pos);
	else return "";
}

Client::Client(int fd, ip_port_t ip_port, Master *parent) : _fd(fd), _ip_port(ip_port), _parent(parent) {
	init();
	cout << "New connection, socket fd is " << fd << ", ip is : " << _ip_port.ip << ", port : " << _ip_port.port << "\n";
}

Client::~Client(void) {
	close(_fd);
	_headers.clear();
	cout << "Host disconnected, ip " << _ip_port.ip << ", port " << _ip_port.port << "\n";
}

void Client::init(void) {
	_finish = false;
	_server = NULL;
	_route = NULL;
	_method = _uri = _host = _header = _body = "";
	_len = 0;
	_last_chunk = false;
	_headers.clear();
}

bool Client::getRequest(Env *env, string paquet) {
	if (paquet.length() < 1) send_error(403);
	if (DEBUG) debug_block("Paquet: ", paquet);
	if (header_pick("Method:", 0) != "") return getBody(paquet);
	vec_string lines = split(paquet, "\r\n");
	for (vec_string::iterator it = lines.begin(); it < lines.end(); it++) {
		size_t pos = paquet.find("\r\n");
		if (pos != string::npos) paquet.erase(0, pos + 2);
		else paquet.clear();
		_header += *it + (it + 1 != lines.end() ? "\r\n" : "");
		if (_header.find("\r\n\r\n") != string::npos)
			return !parseHeader(env) ? false : (_len != 0 ? getBody(paquet) : true);
	}
	return false;
}

bool Client::getBody(string paquet) {
	vec_string					 lines = split(paquet, "\r\n");
	vec_string::iterator it;

	for (it = lines.begin(); it < lines.end(); it++) {
		if (DEBUG) cout << "Remaining length: " << _len << "\n";
		if ((*it).length() && _len <= 0 && header_pick("Transfer-Encoding:", 0) == "chunked") {
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

	if (DEBUG) cout << "Parsing header...\n";
	lines = split(_header, "\r\n");
	method = split(lines.at(0), " ");
	_headers["Method:"] = method;
	if (lines.size() > 0) {
		for (vec_string::iterator it = lines.begin() + 1; it < lines.end(); it++) {
			line = split(*it, " ");
			_headers[line.at(0)] = vec_string(line.begin() + 1, line.end());
		}
	}
	_method = header_pick("Method:", 0);
	if ((_method == "POST" || _method == "PUT") && header_pick("Content-Length:", 0) == "" &&
			header_pick("Transfer-Encoding:", 0) != "chunked")
		return (send_error(400), false);
	vec_string uri_split = split(header_pick("Method:", 1), "?");
	_uri = uri_split.at(0);
	if (uri_split.size() > 1) _query = uri_split.at(1);
	_host = header_pick("Host:", 0);
	_env = env;
	_server = _parent->choose_server(env, _host);
	_route = _server->choose_route(_uri);
	if (DEBUG) debug_header();
	string len = header_pick("Content-Length:", 0).c_str();
	if (len != "") {
		_len = std::atoi(len.c_str());
		_last_chunk = true;
		int max_len = _route->_client_max_body_size > 0		 ? _route->_client_max_body_size
									: _server->_client_max_body_size > 0 ? _server->_client_max_body_size
																											 : INT_MAX;
		if (_len > max_len) return (send_error(413), false);
	} else _len = 0;
	return true;
}

string Client::header_pick(string key, size_t id) { return _headers[key].size() <= id ? "" : _headers[key].at(id); }

bool Client::check_method(void) {
	vec_string allowed;
	if ((_route && (allowed = _route->_allowed_methods).size() > 0) ||
			(_server && (allowed = _server->_allowed_methods).size() > 0) || ((allowed = _env->_allowed_methods).size() > 0))
		return std::find(allowed.begin(), allowed.end(), _method) < allowed.end() ? true : false;
	else if (_method == "GET" || _method == "POST" || _method == "DELETE" || _method == "PUT") return (true);
	return (false);
}

void Client::handleRequest(void) {
	if (DEBUG) {
		debug_block("Header: ", _header);
		debug_block("Body: ", _body);
	}
	string ret;
	string req_path = _route->getRoot() + _uri;
	std::cout << "||-> Request for " << req_path << " received <-||\n";
	string cgi_path = _route->_cgi.size()		 ? _route->_cgi[get_extension(req_path)]
										: _server->_cgi.size() ? _server->_cgi[get_extension(req_path)]
																					 : "";
	if (DEBUG) cout << "Path: " << req_path << "\n";
	if (!check_method()) send_error(405);
	else {
		if ((ret = _route->getIndex(_uri, req_path)) == "") ret = read_file(req_path);
		if (ret == "404") {
			if (_method == "POST" || _method == "PUT") create_file(req_path);
			else send_error(404);
		} else if (ret == "403") send_error(403);
		else if (_method == "DELETE") std::remove(req_path.c_str());
		else if (cgi_path != "") cgi(cgi_path, req_path);
		else send_answer("HTTP/1.1 200 OK\r\n" + ret);
	}
}

void Client::create_file(string path) {
	std::ofstream file(path.c_str());
	if (!file.good()) send_error(403);
	else {
		file << _body;
		file.close();
		send_answer("HTTP/1.1 201 Accepted\r\n\r\n");
	}
}

/**
 * @brief Launch cgi binary to parse the file requested by the client.
 *
 * @param cgi_path The cgi binary location specified in configuration file according to the file requested.
 * @param path The path to the file requested.
 */
void Client::cgi(string cgi_path, string path) {
	int pipe_in[2];

	send(_fd, "HTTP/1.1 200 OK\r\n", 17, MSG_NOSIGNAL);
	if (!std::ifstream(cgi_path.c_str()).good()) return send_error(404);
    if (DEBUG) std::cout << "Send cgi\n";
	if (fork() == 0) {
		const char **args = new const char *[cgi_path.length() + 1];
		args[0] = cgi_path.c_str();
		args[1] = NULL;
		string			 path_info = "PATH_INFO=" + _route->getRoot();
		string			 query = "QUERY_STRING=" + _query;
		const char **env = new const char *[path_info.length() + query.length() + 2];
		env[0] = path_info.c_str();
		env[1] = query.c_str();
		env[2] = NULL;
		pipe(pipe_in);
		std::stringstream tmp;
		tmp << std::ifstream(path.c_str()).rdbuf();
		string file = tmp.str();
		write(pipe_in[1], file.c_str(), file.size());
		close(pipe_in[1]);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);
		dup2(_fd, STDOUT_FILENO);
		close(_fd);
		execve(cgi_path.c_str(), (char **)args, (char **)env);
		exit(1);
	}
	_finish = true;
}

/**
 * @brief Send an error answer to the client.
 *
 * @param error_code The HTTP response code to send.
 */
void Client::send_error(int error_code, string opt) {
	switch (error_code) {
	case 301:
		return send_answer("HTTP/1.1 301 Moved Permanently\r\nLocation: " + opt + "\r\n\r\n");
	case 400:
		return send_answer("HTTP/1.1 400 Bad Request\r\n\r\n");
	case 403:
		return send_answer("HTTP/1.1 403 Forbidden\r\n\r\n");
	case 404:
		return send_answer("HTTP/1.1 404 Not Found\r\n\r\n");
	case 405:
		return send_answer("HTTP/1.1 405 Method Not Allowed\r\nConnection: "
											 "close\r\n\r\n");
	case 413:
		return send_answer("HTTP/1.1 413 Payload Too "
											 "Large\r\nConnection: close\r\n\r\n");
	}
}

/**
 * @brief Send an answer to the client.
 *
 * @param msg The HTTP message to send.
 */
void Client::send_answer(string msg) {
	if (DEBUG) debug_block("ANSWER: ", msg);
#ifdef __linux__
	send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	write(_fd, msg.c_str(), msg.length());
#endif
	init();
}
