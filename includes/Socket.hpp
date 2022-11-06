#pragma once
#include "webserv.hpp"

class Socket {
	string				_ip;
	int					_port;
	int					_master_socket;
	struct sockaddr_in	_address;
	int 				_max_clients;
	int 				_client_socket[30];
  public:
	static fd_set _readfds;
	static int _max_sd;
	static int _min_sd;
	static int _amount;
	Socket(string def);
	~Socket();
	int launch();
	void check();
	void answer();
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



