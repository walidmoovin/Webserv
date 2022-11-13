#pragma once
#include "webserv.hpp"

class Master {
	int						_fd;
	std::vector< Client * > _childs;
	struct sockaddr_in		_address;

  public:
	Master(ip_port_t listen);
	Master(int fd, Master *parent);
	~Master(void);

	void	set_fds(void);
	void	refresh(Env *env);
	Server *choose_server(Env *env, string host);

	ip_port_t	  _listen;
	static fd_set _readfds;
	static int	  _max_fd, _min_fd, _amount;
};
