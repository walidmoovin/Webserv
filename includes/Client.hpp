#pragma once
#include "webserv.hpp"

typedef std::map< string, std::vector< string > > request_t;

class Client {
	int		  _fd;
	ip_port_t _ip_port;
	Master	 *_parent;

	string _method;
	string _uri;
	string _host;
	int	   _len;
	int	   _last_len;

	Server *_server;
	Route  *_route;

	string	  _header;
	string	  _body;
	request_t _request;

  public:
	Client(int fd, ip_port_t ip_port, Master *parent);
	~Client(void);
	void   clean(void);
	bool   parseHeader(Env *env);
	string header_pick(string key, size_t id);
	void   answer();
	bool   check_method();
	void   create_file(string path);
	void   send_cgi(string cgi, string path);
	void   send_cgi_redir(string cgi, string path);
	void   send_error(int error_code);
	void   send_answer(string msg);

	bool getHeader(Env *env, string paquet);
	bool getBody(string paquet);

	friend class Master;
};
