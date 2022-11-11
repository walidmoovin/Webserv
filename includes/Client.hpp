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
	int	 answer(Env *env);
	void call_cgi(string cgi, string path);
	void send_404();
	void send_answer(string msg);
	bool waitHeader();
	friend class Master;
};
