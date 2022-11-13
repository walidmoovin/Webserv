#pragma once
#include "webserv.hpp"

typedef std::map< string, vec_string > request_t;

class Client {
	int		  _fd;
	ip_port_t _ip_port;
	Master	 *_parent;
	Server	 *_server;
	Route	 *_route;
	string	  _method, _uri, _host, _header, _body;
	int		  _len;
	bool	  _last_chunk;
	request_t _request;

	void   clean(void);
	bool   getBody(string paquet);
	bool   parseHeader(Env *env);
	string header_pick(string key, size_t id);
	bool   check_method(void);
	void   create_file(string path);
	void   send_cgi(string cgi, string path);
	void   send_redir(int redir_code, string opt);
	void   send_error(int error_code);
	void   send_answer(string msg);

  public:
	Client(int fd, ip_port_t ip_port, Master *parent);
	~Client(void);
	bool getHeader(Env *env, string paquet);
	void answer(void);

	friend class Master;
};
