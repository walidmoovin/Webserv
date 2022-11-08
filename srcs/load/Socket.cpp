
#include "webserv.hpp"

Socket::Socket(listen_t listen) : _listen(listen) { _clients_amount = 0; }
Socket::~Socket(void) {
	close(_master_socket);
	cout << "Socket destroyed!\n";
}

int Socket::launch(void) {
	int opt = 1;
	string ip = _listen.ip;
	int port = _listen.port;

	_master_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_master_socket == 0) {
		cout << "Socket creation: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	int opt_ret = setsockopt(_master_socket, SOL_SOCKET, SO_REUSEADDR,
							 (char *)&opt, sizeof(opt));
	if (opt_ret < 0) {
		cout << "Sockopt: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip.c_str());
	_address.sin_port = htons(port);

	if (bind(_master_socket, (struct sockaddr *)&_address, sizeof(_address)) <
		0) {
		cout << "Bind: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	cout << "Listener " << ip << " on port " << port << "\n";

	if (listen(_master_socket, 3) < 0) {
		cout << "Listen: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	cout << "Socket::_master_socket: " << _master_socket << "\n";
	if (_master_socket < _min_fd)
		_min_fd = _master_socket;
	_amount++;
	return (EXIT_SUCCESS);
}

void Socket::set_fds(void) {
	FD_SET(_master_socket, &_readfds);

	for (std::vector<int>::iterator it = _clients.begin(); it < _clients.end();
		 it++) {
		FD_SET(*it, &_readfds);
		if (*it > _max_fd)
			_max_fd = *it;
	}
}

void Socket::refresh(Env *env) {
	std::vector<int>::iterator it;
	int valread;
	int addrlen = sizeof(_address);
	char buffer[10000];
	if (FD_ISSET(_master_socket, &_readfds)) {
		int new_socket = accept(_master_socket, (struct sockaddr *)&_address,
								(socklen_t *)&addrlen);
		if (new_socket < 0) {
			cout << "Accept: " << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}
#ifdef __APPLE__
		fcntl(new_socket, F_GETNOSIGPIPE);
#endif
		cout << "New connection, socket fd is " << new_socket
			 << ", ip is : " << inet_ntoa(_address.sin_addr)
			 << ", port : " << ntohs(_address.sin_port) << "\n";
		_clients.push_back(new_socket);
	}
	for (it = _clients.begin(); it < _clients.end(); it++) {
		if (FD_ISSET(*it, &_readfds)) {
			valread = read(*it, buffer, 10000);
			if (valread == 0) {
				getpeername(*it, (struct sockaddr *)&_address,
							(socklen_t *)&addrlen);
				cout << "Host disconnected, ip " << inet_ntoa(_address.sin_addr)
					 << ", port " << ntohs(_address.sin_port) << "\n";
				close(*it);
				_clients.erase(it);
			} else {
				buffer[valread] = '\0';
				if (answer(env, *it, buffer) == EXIT_FAILURE) {
					_clients.erase(it);
					close(*it);
				}

			}
		}
	}
}

int Socket::answer(Env *env, int fd, string request) {
	cout << "|===|Request|===|\n"<< request << "\n|===|===|===|\n";
	std::vector<string> lines = split(request, '\n');
	std::vector<string> head = split(lines.at(0), ' ');
	string uri;
	if (head.size() < 2)
		return EXIT_FAILURE;
	uri = head.at(1);
	cout << uri << "\n";
	string ret;

	std::stringstream answer;
	answer << "HTTP/1.1";

	Server *server = env->choose_server(this, split(lines.at(1), ' ').at(1));
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
	send_answer(fd, answer.str());
	return EXIT_SUCCESS;
}

void Socket::send_answer(int fd, string msg) {
#ifdef __linux__
	send(fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(fd, msg.c_str(), msg.length(), 0);
#endif
}
