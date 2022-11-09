#pragma once
#include "webserv.hpp"

class Socket {
	int _fd;
	//int _clients_amount;
	Socket *_parent;
	std::vector<Socket *> _childs;
	struct sockaddr_in _address;
	string _tmp;
	string _header;
	string _content;
	string _method;
	string _uri;
	string _host;
	string _extension;
	
	int answer(Env *env, string request);
	void send_answer(string msg);
	bool waitHeader();
  public:
	Socket(listen_t listen);
	Socket(int fd, Socket *parent);
	~Socket(void);
	
	listen_t _listen;
	static fd_set _readfds;
	static int _max_fd;
	static int _min_fd;
	static int _amount;

	int launch(void);
	void set_fds(void);
	void refresh(Env *env);
};
