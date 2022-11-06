
#include "webserv.hpp"

Socket::Socket(Server *server, string def) : _server(server) {
	size_t sep_pos = def.rfind(':');
	size_t ip6_endpos = def.rfind(']');

	string tmp = def.substr(0, sep_pos);
	if (ip6_endpos > sep_pos)
		_ip = def;
	else
		_ip = isInt(tmp) || tmp == "localhost" ? "127.0.0.1" : tmp;
	tmp = def.substr(sep_pos + 1, def.length() - sep_pos - 1).c_str();
	_port = !isInt(tmp) ? 80 : std::atoi(tmp.c_str());
	_clients_amount = 0;
}
Socket::~Socket() {
	close(_master_socket);
	cout << "Socket destroyed!\n";
}

int Socket::launch() {
	int opt = 1;
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

	if (_ip.at(0) == '[')
		_address.sin_family = AF_INET6;
	else
		_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(_ip.c_str());
	_address.sin_port = htons(_port);

	if (bind(_master_socket, (struct sockaddr *)&_address, sizeof(_address)) <
		0) {
		cout << "Bind: " << strerror(errno) << "\n";
		return (EXIT_FAILURE);
	}
	cout << "Listener " << _ip << " on port " << _port << "\n";

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

void Socket::set_fds() {
	FD_SET(_master_socket, &_readfds);

	for (std::vector<int>::iterator it = _clients.begin(); it < _clients.end();
		 it++) {
		FD_SET(*it, &_readfds);
		if (*it > _max_fd)
			_max_fd = *it;
	}
}

void Socket::refresh() {
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
				answer(*it, buffer);
			}
		}
	}
}

void Socket::answer(int fd, string request) {
	cout << request << "\n|===|===|===|\n";
	std::stringstream answer;
	answer << "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
	string uri = "docs/";
	Route *route = _server->get_route(uri);
	answer << route->getAutoindex(uri);
	cout << answer.str() << "\n|===|===|===|\n";
	send_answer(fd, answer.str());
}
void Socket::send_answer(int fd, string msg) {
#ifdef __linux__
	send(fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);
#elif __APPLE__
	send(fd, msg.c_str(), msg.length(), 0);
#endif
}
