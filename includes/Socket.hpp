#pragma once
#include "webserv.hpp"

class Socket {
	string				_ip;
	int					_port;
	int					_master_socket;
	struct sockaddr_in	_address;
	int 				_clients_amount;
	std::vector<int>	_clients;
  public:
	static fd_set _readfds;
	static int _max_fd;
	static int _min_fd;
	static int _amount;
	Socket(string def);
	~Socket();
	int launch();
	void set_fds();
	void refresh();
	void answer(int fd, string request);
	/*
	Socket& operator=(Socket &src) {
		_ip = src._ip;
		_port = src._port;
		_master_socket = src._master_socket;
		_address = src._address;
		return src;
	}
*/
};



