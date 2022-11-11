#pragma once
#include "webserv.hpp"

class Master {
	// int _clients_amount;
	int						_fd;
	std::vector< Client * > _childs;
	struct sockaddr_in		_address;

  public:
	Master(listen_t listen);
	Master(int fd, Master *parent);
	~Master(void);

	int		launch(void);
	void	set_fds(void);
	void	refresh(Env *env);
	Server *choose_server(Env *env, string host);

	listen_t	  _listen;
	static fd_set _readfds;
	static int	  _max_fd;
	static int	  _min_fd;
	static int	  _amount;
};
