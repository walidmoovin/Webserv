#pragma once
#include "webserv.hpp"

class Master {
	int										_poll_id;
	int										_fd;
	std::vector<Client *> _childs;
	struct sockaddr_in		_address;

public:
	Master(ip_port_t listen);
	~Master(void);

	void		post_poll(Env *env);
	Server *choose_server(Env *env, string host);

	ip_port_t						 _listen;
	static int					 _poll_id_amount;
	static int					 _first_cli_id;
	static struct pollfd *_pollfds;
};
