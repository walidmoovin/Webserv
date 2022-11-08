#pragma once
#include "webserv.hpp"

class Socket {
	int _fd;
	//int _clients_amount;
	Socket *_parent;
	std::vector<Socket *> _childs;
	string _request;
  public:
	struct sockaddr_in _address;
	listen_t _listen;
	static fd_set _readfds;
	static int _max_fd;
	static int _min_fd;
	static int _amount;
	Socket(listen_t listen);
	Socket(int fd, Socket *parent);
	~Socket(void);
	int launch(void);
	void set_fds(void);
	void refresh(Env *env);
	bool isRequestValid(string request);
	int answer(Env *env, string request);
	void send_answer(string msg);
};
