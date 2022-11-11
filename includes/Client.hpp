#pragma once
#include "webserv.hpp"

typedef std::map< string, std::vector< string > > request_t;

class Client {
	int		  _fd;
	Master	 *_parent;
	string	  _header;
	string	  _content;
	request_t _request;

  public:
	Client(int fd, Master *parent);
	~Client(void);
	bool getRequest(string paquet);
	bool parseHeader();
	void answer(Env *env);
	bool check_method(Server *server, Route *route, string method);
	void send_cgi(string cgi, string path);
	void send_error(int error_code);
	void send_answer(string msg);
	friend class Master;
};
