#pragma once
#include "webserv.hpp"

class Socket {
	int _master_socket;
	int _clients_amount;
	std::vector<int> _clients;

  public:
	struct sockaddr_in _address;
	listen_t _listen;
	static fd_set _readfds;
	static int _max_fd;
	static int _min_fd;
	static int _amount;
	Socket(listen_t listen);
	~Socket(void);
	int launch(void);
	void set_fds(void);
	void refresh(Env *env);
	void answer(Env *env, int fd, string request);
	void send_answer(int fd, string msg);
};
