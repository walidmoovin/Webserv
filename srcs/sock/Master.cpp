#include "webserv.hpp"

/**
 * @brief Constructor
 *
 * -Create a master socket that listen to ip and port defined by input.
 * -Socket is ready to listen for new clients if no exception is thrown.
 *
 * @param list An ip_port_t struct which contain the ip and the port the master
 * listen.
 */
Master::Master(ip_port_t list) : _listen(list) {
	int		 x = 1, port = _listen.port;
	string ip = _listen.ip;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == 0) throw std::runtime_error("socket() error" + string(strerror(errno)));
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&x, sizeof(x)) < 0 && close(_fd) <= 0)
		throw std::runtime_error("setsockopt() error: " + string(strerror(errno)));
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip.c_str());
	_address.sin_port = htons(port);
	if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) && close(_fd) <= 0)
		throw std::runtime_error("bind() error: " + string(strerror(errno)));
	if (listen(_fd, 3) < 0 && close(_fd) <= 0) throw std::runtime_error("listen() error: " + string(strerror(errno)));
	#ifdef __APPLE__
		fcntl(socket, F_SETFL, O_NONBLOCK);
	#endif
	cout << "New master socket with fd " << _fd << " which listen " << ip << ":" << port << "\n";
	_pollfds[_poll_id_amount].fd = _fd;
	_pollfds[_poll_id_amount].events = POLLIN | POLLPRI;
	_poll_id = _poll_id_amount;
	_poll_id_amount++;
}

Master::~Master(void) {
	close(_fd);
}

void Master::check_socket(void) {
	int addrlen = sizeof(_address);

	if (_pollfds[_poll_id].revents & POLLIN) {
		int new_socket = accept(_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen);
		if (new_socket < 0) throw std::runtime_error("accept() error:" + string(strerror(errno)));
		#ifdef __APPLE__
			fcntl(new_socket, F_SETFL, O_NONBLOCK);
		#endif
		Client	 *new_cli = new Client(new_socket, (ip_port_t){0, inet_ntoa(_address.sin_addr), ntohs(_address.sin_port)}, this);
		if (_poll_id_amount > MAX_CLIENTS) {
			new_cli->send_error(503);
			delete new_cli;
		} else {
			_childs.push_back(new_cli);
			for (int i = _first_cli_id; i < MAX_CLIENTS; i++) {
				if (_pollfds[i].fd != 0) continue;
				_pollfds[i].fd = new_socket;
				_pollfds[i].events = POLLIN | POLLPRI;
				new_cli->_poll_id = i;
				_poll_id_amount++;
				break;
			}
		}
	}
}

void Master::check_childs(Env *env) {
	int child_fd;
	for (std::vector<Client *>::iterator it = _childs.begin(); it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		int i = (*it)->_poll_id;
		if (_pollfds[i].fd > 0 && _pollfds[i].revents & POLLIN) {
			char buffer[1024];
			int	 valread = read(child_fd, buffer, 1023);
			buffer[valread] = '\0';
			if (valread == 0) {
				delete (*it);
				_childs.erase(it);
			} else if ((*it)->getRequest(env, buffer)) {
				(*it)->debug(true);
				(*it)->handleRequest();
				_pollfds[i].events = POLLIN | POLLPRI | POLLOUT;
				(*it)->debug(false);
				if ((*it)->_finish) {
					delete (*it);
					_childs.erase(it);
				}
			} else _pollfds[i].events = POLLIN | POLLPRI;
		}
	}
}

Server *Master::choose_server(Env *env, string host) {
	std::vector<Server *> exact, inrange;
	vec_string						ip_listen, ip_required;

	ip_required = split(_listen.ip, ".");
	for (std::vector<Server *>::iterator server = env->_servers.begin(); server < env->_servers.end(); server++) {
		std::vector<ip_port_t> serv_listens = (*server)->_listens;
		for (std::vector<ip_port_t>::iterator it = serv_listens.begin(); it < serv_listens.end(); it++) {
			if (_listen.port != (*it).port) continue;
			if (_listen.ip == (*it).ip) {
				exact.push_back(*server);
				continue;
			}
			bool is_inrange = true;
			ip_listen = split((*it).ip, ".");
			vec_string::iterator r = ip_required.end();
			vec_string::iterator l = ip_listen.end();
			while (r > ip_required.begin()) {
				if (*(--l) != *(--r) && *l != "0") is_inrange = false;
			}
			if (is_inrange) inrange.push_back(*server);
		}
	}
	if (exact.size() == 0) {
		for (std::vector<Server *>::iterator server = inrange.begin(); server < inrange.end(); server++) {
			if (host == (*server)->getName()) return *server;
		}
		return inrange.front();
	} else {
		for (std::vector<Server *>::iterator server = exact.begin(); server < exact.end(); server++) {
			if (host == (*server)->getName()) return *server;
		}
		return exact.front();
	}
}
