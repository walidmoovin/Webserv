#include "webserv.hpp"

Master::Master(listen_t listen) : _listen(listen) {}
Master::~Master(void) {
	close(_fd);
	cout << "Destroyed master socket\n";
}

int Master::launch(void) {
	int	   opt = 1;
	string ip = _listen.ip;
	int	   port = _listen.port;

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

void Master::set_fds(void) {
	FD_SET(_fd, &_readfds);

	int child_fd;
	for (std::vector< Client * >::iterator it = _childs.begin();
		 it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		FD_SET(child_fd, &_readfds);
		if (child_fd > _max_fd)
			_max_fd = child_fd;
	}
}

void Master::refresh(Env *env) {
	int	 valread;
	int	 addrlen = sizeof(_address);
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
		_childs.push_back(new Client(new_socket, this));
	}
	int child_fd;
	for (std::vector< Client * >::iterator it = _childs.begin();
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

Server *Master::choose_server(Env *env, string host) {
	std::vector< Server * > exact;
	std::vector< Server * > inrange;
	std::vector< string >	ip_listen;
	std::vector< string >	ip_required;
	// string ip = inet_ntoa(sock->_address.sin_addr);
	// int port = ntohs(sock->_address.sin_port);

	// cout << "Which server for " << ip << ":" << port << "?\n";
	cout << "Socket: " << _listen.ip << ":" << _listen.port << "\n";
	ip_required = split(_listen.ip, '.');
	for (std::vector< Server * >::iterator sit = env->_servers.begin();
		 sit < env->_servers.end(); sit++) {

		std::vector< listen_t > serv_listens = (*sit)->_listens;
		for (std::vector< listen_t >::iterator it = serv_listens.begin();
			 it < serv_listens.end(); it++) {

			if (_listen.port != (*it).port)
				continue;
			if (_listen.ip == (*it).ip) {
				exact.push_back(*sit);
				continue;
			}
			bool is_inrange = true;
			ip_listen = split((*it).ip, '.');
			std::vector< string >::iterator r = ip_required.begin();
			for (std::vector< string >::iterator l = ip_listen.begin();
				 l < ip_listen.end(); l++) {
				if (*l != *r && *l != "0")
					is_inrange = false;
			}
			if (is_inrange == true)
				inrange.push_back(*sit);
		}
	}
	if (exact.size() == 0) {
		for (std::vector< Server * >::iterator sit = inrange.begin();
			 sit < inrange.end(); sit++) {
			if (host == (*sit)->getName())
				return *sit;
		}
		return inrange.front();
	} else
		return exact.front();
}
