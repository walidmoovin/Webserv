
#include "webserv.hpp"

Socket::Socket(listen_t listen) : _listen(listen) {}
Socket::Socket(int fd, Socket *parent) : _fd(fd),_parent(parent) {}
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
	int opt_ret = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR,
							 (char *)&opt, sizeof(opt));
	if (opt_ret < 0) {
		cout << "Sockopt: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip.c_str());
	_address.sin_port = htons(port);

	if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) <
		0) {
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
	int child_fd;
	FD_SET(_fd, &_readfds);
	for (std::vector<Socket *>::iterator it = _childs.begin(); it < _childs.end();
		 it++) {
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
		int new_socket = accept(_fd, (struct sockaddr *)&_address,
								(socklen_t *)&addrlen);
		if (new_socket < 0) {
			cout << "Accept: " << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}
#ifdef __APPLE__
		//fcntl(new_socket, F_GETNOSIGPIPE);
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
#endif
		cout << "New connection, socket fd is " << new_socket
			 << ", ip is : " << inet_ntoa(_address.sin_addr)
			 << ", port : " << ntohs(_address.sin_port) << "\n";
		_childs.push_back(new Socket(new_socket, this));
	}
	int child_fd;
	for (std::vector<Socket *>::iterator it = _childs.begin(); it < _childs.end(); it++) {
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
			 	(*it)->answer(env, buffer);
			}
		}
	}
}

bool Socket::isRequestValid(string request) {
	_request += request;
	if (_request.length() < 1)
		return false;
	std::vector<string> lines = split(_request, '\n');
	bool is_valid = false;
	for (std::vector<string>::iterator it = lines.begin(); 
			it < lines.end(); it++) {
		if (*it == "\r" || *it == "") is_valid = true;
	}
	if (!is_valid || lines.at(0) == "")
		return false;
	std::vector<string> head = split(lines.at(0), ' ');
	if ((head.at(0) != "GET" && head.at(0) != "POST" && head.at(0) != "DELETE")
		|| head.size() < 2)
		return false;
	return true;
}

int Socket::answer(Env *env, string request) {
	cout << "|===|Request|===|\n"<< request << "\n|===|===|===|\n";
	if (!isRequestValid(request)) {
		cout << "Bad request recieved\n";
		send_answer("HTTP/1.1 400 Bad Request\n\n");
		return EXIT_FAILURE;
	}
	std::vector<string> lines = split(_request, '\n');
	std::vector<string> head = split(lines.at(0), ' ');
	string uri = head.at(1);
	cout << uri << "\n";
	string ret;
	std::stringstream answer;
	answer << "HTTP/1.1";

	Server *server = env->choose_server(_parent, split(lines.at(1), ' ').at(1));
	Route *route = server->get_route(uri);
	string path = route->correctUri(uri);
	cout << "Path: " << path << "\n";
	ret = route->getIndex(uri, path);
	if (ret == "") {
		cout << "No index: lf file\n";
		ret = read_file(path);
	}
	answer << (ret == "" ? " 404 Not Found\nContent-length: 0\n\n" : " 200 OK\n") << ret;
	cout << "|===|Answer|===|\n" << answer.str() << "\n|===|===|===|\n";
	send_answer(answer.str());
	_request = "";
	return EXIT_SUCCESS;
}

void Socket::send_answer(string msg) {
#ifdef __linux__
	send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(_fd, msg.c_str(), msg.length(), 0);
#endif
}
