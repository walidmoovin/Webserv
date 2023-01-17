#pragma once
#include "webserv.hpp"

class Client {
	int													 _fd;
	ip_port_t										 _ip_port;
	Master											*_parent;
	Server											*_server;
	Env													*_env;
	Route												*_route;
	string											 _method, _uri, _query, _host, _header, _body;
	int													 _len;
	bool												 _last_chunk;
	std::map<string, vec_string> _request;

	void	 init(void);
	bool	 getBody(string paquet);
	bool	 parseHeader(Env *env);
	string header_pick(string key, size_t id);
	bool	 check_method(void);
	void	 create_file(string path);
	void	 cgi(string cgi_path, string path);
	void	 send_redir(int redir_code, string opt);
	void	 send_error(int error_code);
	void	 send_answer(string msg);

#ifdef DEBUG
	void debug_header(void);
#endif

public:
	Client(int fd, ip_port_t ip_port, Master *parent);
	~Client(void);
	bool getRequest(Env *env, string paquet);
	void handleRequest(void);

	friend class Master;
};
