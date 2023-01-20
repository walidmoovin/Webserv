/**
 * @file Master.cpp
 * @brief The master sockets class which receive each incomming new client.
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-12
 */
#include "webserv.hpp"

/**
 * @brief Destructor
 * Close master socket descriptor.
 */
Master::~Master(void) {
	close(_fd);
	cout << "Destroyed master socket\n";
}

/**
 * @brief Constructor
 * Try to create a socket listening to ip and port defined by input.
 * If no exception if thrown, the creation success and the socket is then ready to select for new clients.
 *
 * @param list An ip_port_t struct which contain the ip and the port the master listen.
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
	if (_fd < _min_fd) _min_fd = _fd;
}

/**
 * @brief The pre select operations:
 * Add master's socket descriptor and each one of his childs to the select list of descriptor.
 */
void Master::pre_select(void) {
	FD_SET(_fd, &_readfds);
	if (_fd > _max_fd) _max_fd = _fd;
	for (std::vector<Client *>::iterator child = _childs.begin(); child < _childs.end(); child++) {
		FD_SET((*child)->_fd, &_readfds);
		if ((*child)->_fd > _max_fd) _max_fd = (*child)->_fd;
	}
}

/* |==========|
 * Refresh master socket datas after select()
 * - look first for new clients
 * - look then if known clients sent requests or disconnected
 * - if client sent request, handle it to generate answer adapted
 */
/**
 * @brief Checkk master and his clients sockets after select performed.
 * - First look for new clients
 * - Then handle clients awaiting action:
 *   - Disconnect them if nothing's new on the socket (request altready handled)
 *   - Read and parse request else until it's ready to answer and does it.
 *
 * @param env The environment object which contain the liste of servers to know which one the client is trying to reach.
 */
void Master::post_select(Env *env) {
	int	 valread;
	int	 addrlen = sizeof(_address);
	char buffer[128];

	if (FD_ISSET(_fd, &_readfds)) { /// < incomming master request
		int new_socket = accept(_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen);
		if (new_socket < 0) throw std::runtime_error("accept() error:" + string(strerror(errno)));
#ifdef __APPLE__
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
#endif
		ip_port_t cli_listen = get_ip_port_t(inet_ntoa(_address.sin_addr), ntohs(_address.sin_port));
		_childs.push_back(new Client(new_socket, cli_listen, this));
	}
	int child_fd;
	for (std::vector<Client *>::iterator it = _childs.begin(); it < _childs.end(); it++) {
		child_fd = (*it)->_fd;
		if (FD_ISSET(child_fd, &_readfds)) {
			valread = read(child_fd, buffer, 127);
			buffer[valread] = '\0';
			if (valread == 0) {
				getpeername(child_fd, (struct sockaddr *)&_address, (socklen_t *)&addrlen);
				delete (*it);
				_childs.erase(it);
			} else if ((*it)->getRequest(env, buffer)) {
				(*it)->handleRequest();
			}
		}
	}
}

/**
 *
 * @brief Choose the server which must handle a request
 * Each server can lsiten multiple range_ip:port and each range_it:port can be listen by multiple servers.
 * So for each request, we must look at the socket which given us the client to know how the client came. If multiple
 * servers listen the range from where the client came, ones with exact correspondance are prefered. If there are
 * multiples servers listening exactly the ip the client try to reach or which listen a range which contain it, the
 * first one which have the same server_name as the host the client used to reach server is used, else it's the first
 * one of exact correspondance or first one which have the ip requested in his listen range.
 *
 * @param env The environment object.
 * @param host The host the client used to reached the server.
 *
 * @return The server object choosen to handle the request.
 */
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
			vec_string::iterator r = ip_required.begin();
			for (vec_string::iterator l = ip_listen.end(); l >= ip_listen.begin(); --l) {
				if (*l != *r && *l != "0") is_inrange = false;
			}
			if (is_inrange) inrange.push_back(*server);
		}
	}
	if (DEBUG) std::cout << "req: " << _listen.ip << ":" << _listen.port << "\n";
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
