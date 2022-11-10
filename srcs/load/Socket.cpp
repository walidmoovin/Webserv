#include "webserv.hpp"

Socket::Socket(listen_t listen) : _listen(listen) {}
Socket::Socket(int fd, Socket *parent) : _fd(fd), _parent(parent) {}
Socket::~Socket(void) {
	close(_fd);
	cout << "Socket destroyed!\n";
}

int Socket::launch(void) {
	int opt = 1;
	string ip = _listen.ip;
	int port = _listen.port;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == 0) {
		cout << "Socket creation: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	int opt_ret =
		setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
	if (opt_ret < 0) {
		cout << "Sockopt: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip.c_str());
	_address.sin_port = htons(port);

	if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
		cout << "Bind: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	cout << "Listener " << ip << " on port " << port << "\n";

	if (listen(_fd, 3) < 0) {
		cout << "Listen: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	cout << "Master: " << _fd << "\n";
	if (_fd < _min_fd)
		_min_fd = _fd;
	_amount++;
	return (EXIT_SUCCESS);
}

void Socket::set_fds(void) {
	FD_SET(_fd, &_readfds);

	int child_fd;
	for (std::vector<Socket *>::iterator it = _childs.begin();
		 it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		FD_SET(child_fd, &_readfds);
		if (child_fd > _max_fd)
			_max_fd = child_fd;
	}
}

void Socket::refresh(Env *env) {
	int valread;
	int addrlen = sizeof(_address);
	char buffer[10000];
	if (FD_ISSET(_fd, &_readfds)) {
		int new_socket =
			accept(_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen);
		if (new_socket < 0) {
			cout << "Accept: " << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}
#ifdef __APPLE__
		// fcntl(new_socket, F_GETNOSIGPIPE);
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
#endif
		cout << "New connection, socket fd is " << new_socket
			 << ", ip is : " << inet_ntoa(_address.sin_addr)
			 << ", port : " << ntohs(_address.sin_port) << "\n";
		_childs.push_back(new Socket(new_socket, this));
	}
	int child_fd;
	for (std::vector<Socket *>::iterator it = _childs.begin();
		 it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		if (FD_ISSET(child_fd, &_readfds)) {
			valread = read(child_fd, buffer, 10000);
			if (valread == 0) {
				getpeername(child_fd, (struct sockaddr *)&_address,
							(socklen_t *)&addrlen);
				cout << "Host disconnected, ip " << inet_ntoa(_address.sin_addr)
					 << ", port " << ntohs(_address.sin_port) << "\n";
				delete (*it);
				_childs.erase(it);
			} else {
				buffer[valread] = '\0';
				if ((*it)->getRequest(buffer))
					(*it)->answer(env);
			}
		}
	}
}

Server *Socket::choose_server(Env *env, string host) {
	std::vector<Server *> exact;
	std::vector<Server *> inrange;
	std::vector<string> ip_listen;
	std::vector<string> ip_required;
	// string ip = inet_ntoa(sock->_address.sin_addr);
	// int port = ntohs(sock->_address.sin_port);

	// cout << "Which server for " << ip << ":" << port << "?\n";
	cout << "Socket: " << _listen.ip << ":" << _listen.port << "\n";
	ip_required = split(_listen.ip, '.');
	for (std::vector<Server *>::iterator sit = env->_servers.begin();
		 sit < env->_servers.end(); sit++) {

		std::vector<listen_t> serv_listens = (*sit)->_listens;
		for (std::vector<listen_t>::iterator it = serv_listens.begin();
			 it < serv_listens.end(); it++) {

			if (_listen.port != (*it).port)
				continue;
			if (_listen.ip == (*it).ip) {
				exact.push_back(*sit);
				continue;
			}
			bool is_inrange = true;
			ip_listen = split((*it).ip, '.');
			std::vector<string>::iterator r = ip_required.begin();
			for (std::vector<string>::iterator l = ip_listen.begin();
				 l < ip_listen.end(); l++) {
				if (*l != *r && *l != "0")
					is_inrange = false;
			}
			if (is_inrange == true)
				inrange.push_back(*sit);
		}
	}
	if (exact.size() == 0) {
		for (std::vector<Server *>::iterator sit = inrange.begin();
			 sit < inrange.end(); sit++) {
			if (host == (*sit)->getName())
				return *sit;
		}
		return inrange.front();
	} else
		return exact.front();
}
bool Socket::parseHeader() {
	std::vector<string> lines = split(_header, '\n');
	std::vector<string> line;
	if (lines.size() > 0) {
		for (std::vector<string>::iterator it = lines.begin() + 1;
			 it < lines.end(); it++) {
			line = split(*it, ' ');
			cout << line.at(0) << "scraped from header\n";
			_request[line.at(0)] =
				std::vector<string>(line.begin() + 1, line.end());
		}
	}
	std::vector<string> method = split(lines.at(0), ' ');
	if (method.at(0) == "POST" &&
		_request.find("Content-Length:") == _request.end() &&
		_request.find("Transfer-Encoding:") == _request.end())
		return false;
	_request["Method:"] = method;
	return true;
}

bool Socket::getRequest(string paquet) {
	cout << "|===|paquet|===>" << paquet << "|===||\n";
	if (paquet.length() < 1) // HTTPS?
		return false;
	std::vector<string> lines = split(paquet, '\n');
	long chunk_len =
		(_content.length() > 0 && _request["Transfer-Encoding:"].size() &&
		 _request["Transfer-Encoding:"].at(0) == "chunked")
			? std::strtol(lines.at(0).substr(1).c_str(), 0, 16)
			: -1;
	cout << "Chunk length: " << chunk_len << "\n";
	for (std::vector<string>::iterator it = lines.begin(); it < lines.end();
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

int Socket::answer(Env *env) {
	cout << "Method: " << _request["Method:"].at(0) << "\n";
	cout << "URI: " << _request["Method:"].at(1) << "\n";
	cout << "Host: " << _request["Host:"].at(0) << "\n";
	string ret;
	std::stringstream answer;
	answer << "HTTP/1.1";

	Server *server = _parent->choose_server(env, _request["Host:"].at(0));
	Route *route = server->choose_route(_request["Method:"].at(1));
	string method = _request["Method:"].at(0);
	std::vector<string> allowed;
	if (method != "GET" && method != "POST" && method != "DELETE")
		send_answer(
			"HTTP/1.1 405 Method Not Allowed\r\nContent-length: 0\r\n\r\n");
	else if ((allowed = route->getHeadersLst()).size() > 0) {
		if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
			send_answer(
				"HTTP/1.1 405 Method Not Allowed\r\nContent-length: 0\r\n\r\n");
	} else if ((allowed = server->getHeadersLst()).size() > 0) {
		if (std::find(allowed.begin(), allowed.end(), method) == allowed.end())
			send_answer(
				"HTTP/1.1 405 Method Not Allowed\r\nContent-length: 0\r\n\r\n");
	}

	string path = route->correctUri(_request["Method:"].at(1));
	cout << "Path: " << path << "\n";
	ret = route->getIndex(_request["Method:"].at(1), path);
	if (ret == "") {
		cout << "No index: lf file\n";
		ret = read_file(path);
	}
	answer << (ret == "" ? " 404 Not Found\r\nContent-length: 0\r\n\r\n"
						 : " 200 OK\r\n")
		   << ret;
	cout << "|===|Answer|===|\n" << answer.str() << "|===|End of answer|===|\n";
	send_answer(answer.str());
	_content = "";
	_header = "";
	return EXIT_SUCCESS;
}

void Socket::send_answer(string msg) {
#ifdef __linux__
	send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(_fd, msg.c_str(), msg.length(), 0);
#endif
}
