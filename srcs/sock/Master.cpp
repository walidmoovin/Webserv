#include "webserv.hpp"
/* Master destructor */

Master::~Master(void) {
	close(_fd);
	cout << "Destroyed master socket\n";
}
/* |==========|
 * Master constructor
 * Try to create a socket listening to ip and port defined by input.
 * If the creation success, the socket is then ready to select for new clients.
 *
 * Input: A listen_t structure which contain the ip and the port the master care
 * about.
 * Output: A Master object.
 */

Master::Master(listen_t list) : _listen(list) {
	int	   opt = 1;
	string ip = _listen.ip;
	int	   port = _listen.port;

	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd == 0)
		throw std::runtime_error("socket() error" + string(strerror(errno)));
	int opt_ret =
		setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
	if (opt_ret < 0)
		throw std::runtime_error("setsockopt() error: " +
								 string(strerror(errno)));

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = inet_addr(ip.c_str());
	_address.sin_port = htons(port);

	if (bind(_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
		throw std::runtime_error("bind() error: " + string(strerror(errno)));

	if (listen(_fd, 3) < 0)
		throw std::runtime_error("listen() error: " + string(strerror(errno)));
	cout << "New master socket with fd " << _fd << " which listen " << ip << ":"
		 << port << "\n";
	if (_fd < _min_fd)
		_min_fd = _fd;
	_amount++;
}
/* Set into static Master::readfds the active fds which will be select.*/

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
/* |==========|
 * Refresh master socket datas after select()
 * - look first for new clients
 * - look then if known clients sent requests or disconnected
 * - if client sent request, handle it to generate answer adapted
 */

void Master::refresh(Env *env) {
	int	 valread;
	int	 addrlen = sizeof(_address);
	char buffer[10000];
	if (FD_ISSET(_fd, &_readfds)) {
		int new_socket =
			accept(_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen);
		if (new_socket < 0)
			throw std::runtime_error("accept() error:" +
									 string(strerror(errno)));
#ifdef __APPLE__
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
#endif
		listen_t cli_listen = get_listen_t(inet_ntoa(_address.sin_addr),
										   ntohs(_address.sin_port));
		_childs.push_back(new Client(new_socket, cli_listen, this));
	}
	int child_fd;
	for (std::vector< Client * >::iterator it = _childs.begin();
		 it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		if (FD_ISSET(child_fd, &_readfds)) {
			valread = read(child_fd, buffer, 10000);
			buffer[valread] = '\0';
			if (valread == 0) {
				getpeername(child_fd, (struct sockaddr *)&_address,
							(socklen_t *)&addrlen);
				delete (*it);
				_childs.erase(it);
			} else if ((*it)->getRequest(buffer))
				(*it)->answer(env);
		}
	}
}

Server *Master::choose_server(Env *env, string host) {
	std::vector< Server * > exact;
	std::vector< Server * > inrange;
	std::vector< string >	ip_listen;
	std::vector< string >	ip_required;

	ip_required = split(_listen.ip, ".");
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
			ip_listen = split((*it).ip, ".");
			std::vector< string >::iterator r = ip_required.begin();
			for (std::vector< string >::iterator l = ip_listen.end();
				 l >= ip_listen.begin(); --l) {
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
	} else {
		for (std::vector< Server * >::iterator sit = exact.begin();
			 sit < exact.end(); sit++) {
			if (host == (*sit)->getName())
				return *sit;
		}
		return exact.front();
	}
}
