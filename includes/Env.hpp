#pragma once
#include "webserv.hpp"

class Env {
  public:
	std::vector< Server * > _servers;
	std::vector< Master * > _masters;
	vec_string				_allowed_methods;


	Env(JSONNode *conf);
	void cycle(void);
	void set_fds(void);
	void refresh(void);
	~Env(void);
};
