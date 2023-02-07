#include "webserv.hpp"
#include <iomanip>
#include <iostream>

inline string get_extension(string str) {
	size_t pos = str.rfind('.');
	if (pos != string::npos) return str.substr(pos);
	else return "";
}

Client::Client(int fd, ip_port_t ip_port, Master *parent) : _fd(fd), _ip_port(ip_port), _parent(parent) {
	_requests_done = 0;
	_death_time = 0;
	_finish = false;
	_route = NULL;
	_server = NULL;
	_pid = 0;
	init();
	if (!SILENT)
		cout << "New connection, socket fd is " << fd << ", ip is : " << _ip_port.ip << ", port : " << _ip_port.port
				 << "\n";
}

Client::~Client(void) {
	close(_fd);
	Master::_pollfds[_poll_id].fd = 0;
	Master::_pollfds[_poll_id].events = 0;
	Master::_pollfds[_poll_id].revents = 0;
	Master::_poll_id_amount--;
	_headers.clear();
	if (!SILENT) cout << "Host disconnected, ip " << _ip_port.ip << ", port " << _ip_port.port << "\n";
}

/**
 * @brief Initialize the client.
 *  If the client has a route, check if the max requests is reached.
 *  If the client has a server, check if the max requests is reached.
 *  Reset all the variables.
 *
 * @Usage Called when the client is created and when the request is done.
 */
void Client::init(void) {
	_requests_done++;
	if (_route && _route->_max_requests > 0) {
		if (_requests_done > _route->_max_requests) _finish = true;
	} else if (_server && _server->_max_requests > 0) {
		if (_requests_done > _server->_max_requests) _finish = true;
	}
	_method = _uri = _host = _header = _body = "";
	_len = 0;
	_headers.clear();
}

/**
 * @brief Get the request from the client.
 *  If the request is not complete, return false (we detect the end of the request with the double \r\n).
 *  If the request is complete, parse the header and get the body if there is one.
 *
 */
bool Client::getRequest(Env *env, string paquet) {
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

template <typename T> void tab(T t, const int &width) {
	std::cout << std::left << std::setw(width) << std::setfill(' ') << t;
}
void Client::debug(bool head) {

	if (head) {
		std::cout << "Client " << _poll_id << " debug ===================================\n";
		tab("Fd", 4);
		tab("Ip", 12);
		tab("Port", 6);
		tab("Servername", 12);
		tab("Route", 12);
		tab("Method", 6);
		tab("URI", 20);
		tab("Query", 12);
		tab("Host", 12);
		tab("Len", 6);
		tab("Keep", 4);
		tab("Death", 6);
		tab("Request", 6);
		tab("Finish", 6);
		std::cout << "\n";
	}
	tab(_fd, 4);
	tab(_ip_port.ip, 12);
	tab(_ip_port.port, 6);
	tab(_server->getName(), 12);
	tab(_route->getLocation(), 12);
	tab(_method, 6);
	tab(_uri, 20);
	tab(_query, 12);
	tab(_host, 12);
	tab(_len, 6);
	tab(_keepalive, 4);
	tab(_death_time, 6);
	tab(_requests_done, 6);
	tab(_finish, 6);
	std::cout << "\n";
}

bool Client::getBody(string paquet) {
	vec_string					 lines = split(paquet, "\r\n");
	vec_string::iterator it;

	for (it = lines.begin(); it < lines.end(); it++) {
		if (_len > 0 || it != lines.begin()) {
			_body += *it + "\r\n";
			_len -= ((*it).length() + 2);
		}
	}
	_body.resize(_body.length() - 2);
	_len += 2;
	return (_len == 0) ? true : false;
}

bool Client::parseHeader(Env *env) {
	vec_string lines, method, line;
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
	if ((_method == "POST" || _method == "PUT") && header_pick("Content-Length:", 0) == "")
		return (send_error(400), false);
	vec_string uri_split = split(header_pick("Method:", 1), "?");
	_uri = uri_split.at(0);
	if (uri_split.size() > 1) _query = uri_split.at(1);
	_host = header_pick("Host:", 0);
	_keepalive = header_pick("Connection:", 0) == "keep-alive";
	_env = env;
	struct timeval t;
	gettimeofday(&t, NULL);
	_server = _parent->choose_server(env, _host);
	if (_server->_timeout) _death_time = _server->_timeout + t.tv_sec;
	_route = _server->choose_route(_uri);
	if (_route->_timeout) _death_time = _route->_timeout + t.tv_sec;
	string len = header_pick("Content-Length:", 0).c_str();
	if (len != "") {
		_len = std::atoi(len.c_str());
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
	if (_route->_ret_uri != "") send_error(_route->_ret_code, _route->_ret_uri);
	else if (_server->_ret_uri != "") send_error(_server->_ret_code, _server->_ret_uri);
	else {
		string				 ret;
		string req_path = _route->getRoot() + _uri;
		if (!SILENT) std::cout << "||-> Request for " << req_path << " received <-||\n";
		string cgi_path = _route->_cgi.size()		 ? _route->_cgi[get_extension(req_path)]
											: _server->_cgi.size() ? _server->_cgi[get_extension(req_path)]
																						 : "";
		if (!check_method()) send_error(405);
		else {
			if ((ret = _route->getIndex(_uri, req_path)) == "") ret = file_answer(req_path);
			if (ret == "404") {
				if (_method == "POST" || _method == "PUT") create_file(req_path);
				else send_error(404);
			} else if (ret == "403") send_error(403);
			else if (_method == "DELETE"){
				std::remove(req_path.c_str());
				send_answer("HTTP/1.1 200 OK \r\n\r\n");
			}
			else if (cgi_path != "") cgi(cgi_path, req_path);
			else send_answer("HTTP/1.1 200 OK\r\n" + ret);
		}
	}
}

void Client::create_file(string path) {
	struct stat st;
	if (stat(_route->_upload_folder.c_str(), &st) != 0) {
		send_error(403);
		return;
	}
	if (_route->_upload_folder != "") path = _route->_upload_folder + _uri;
	std::ofstream file(path.c_str());
	if (!file.good()) send_error(403);
	else {
		file << _body;
		file.close();
		send_answer("HTTP/1.1 201 Accepted\r\nContent-Length:0\r\n\r\n");
	}
}

/**
 * @brief Launch cgi binary to parse the file requested by the client.
 *
 * @param cgi_path The cgi binary location specified in configuration file
 * according to the file requested.
 * @param path The path to the file requested.
 */
void Client::cgi(string cgi_path, string path) {
	int pipe_in[2];

	#ifdef __linux__
	send(_fd, "HTTP/1.1 200 OK\r\n", 17, MSG_NOSIGNAL);
	#elif __APPLE__
	write(_fd, "HTTP/1.1 200 OK\r\n", 17);
	#endif
	if (!std::ifstream(cgi_path.c_str()).good()) return send_error(404);
	if (fork() == 0) {
		const char **args = new const char *[path.length() + 1];
		args[0] = path.c_str();
		args[1] = NULL;
		std::vector<string> env_vec;

		env_vec.push_back("REQUEST_METHOD=" + _method);
		env_vec.push_back("QUERY_STRING=" + _query);
		env_vec.push_back("PATH_INFO=" + _route->getRoot());
		env_vec.push_back("PATH_TRANSLATED=" + path);
		env_vec.push_back("SCRIPT_NAME=" + _uri);
		env_vec.push_back("REDIRECT_STATUS=200");
		string multi_part = header_pick("Content-Type:", 0) + " " + header_pick("Content-Type:", 1);
		std::cout << multi_part << std::endl;
		env_vec.push_back("CONTENT_TYPE=" + multi_part); 
		std::stringstream tmp;
		tmp << "CONTENT_LENGTH=" << _body.length();
		env_vec.push_back(tmp.str());
		size_t env_size = 0;
		for (std::vector<string>::iterator it = env_vec.begin(); it != env_vec.end(); it++)
			env_size += (*it).size() + 1;
		const char **env = new const char *[env_size];
		int i = 0;
		for (std::vector<string>::iterator it = env_vec.begin(); it != env_vec.end(); it++)
			env[i++] = (*it).c_str();
		env[i] = NULL;
		pipe(pipe_in);
		if (_body.size())
			write(pipe_in[1], _body.c_str(), _body.size());
		close(pipe_in[1]);
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[0]);
		dup2(_fd, STDOUT_FILENO);
		close(_fd);
		execve(path.c_str(), (char **)args, (char **)env);
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
	string error_path, body;
	error_path = _route->_err_page[error_code];
	if (error_path == "") {
		error_path = _server->_err_page[error_code];
		if (error_path != "") body = file_answer(_server->correctUri(error_path));
	} else body = file_answer(_route->correctUri(error_path));
	std::map<int, string> error_strings;
	error_strings[301] = "Moved Permanently";
	error_strings[302] = "Found";
	error_strings[307] = "Temporary Redirect";
	error_strings[308] = "Permanent Redirect";
	error_strings[400] = "Bad Request";
	error_strings[403] = "Forbidden";
	error_strings[404] = "Not Found";
	error_strings[405] = "Method Not Allowed";
	error_strings[408] = "Request Timeout";
	error_strings[413] = "Payload Too Large";
	error_strings[429] = "Too Many Requests";
	std::stringstream ret;
	if (error_code >= 300 && error_code < 400) ret << "HTTP/1.1 " << error_code << " " << error_strings[error_code] << "\r\nLocation: " << opt << "\r\n\r\n";
	else if (error_path != "") ret << "HTTP/1.1 " << error_code << " " << error_strings[error_code] << "\r\n" << body << "\r\n";
	else ret << "HTTP/1.1 " << error_code << " " << error_strings[error_code] << "\r\nContent-Length:0\r\n\r\n";
	return send_answer(ret.str());
}

/**
 * @brief Send an answer to the client.
 *
 * @param msg The HTTP message to send.
 */
void Client::send_answer(string msg) {
	#ifdef __linux__
		send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
	#elif __APPLE__
		write(_fd, msg.c_str(), msg.length());
	#endif
		init();
		if (!_keepalive) _finish = true;
}
