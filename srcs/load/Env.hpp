#include "../webserv.hpp"
#include "Server.hpp"

class Env {
	//int	_max_clients;
	std::vector<Server>	_servers;
  public:
	Env(JSONNode *conf) {
        JSONList servers = conf->obj()["servers"]->lst();
		int i = 0;
		string th[8]= {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eigth"};
        for (std::vector<JSONNode *>::iterator it = servers.begin();
             it < servers.end(); it++) {
            	Server server(*it);
				_servers.push_back(server);
			//delete *it;
			cout << th[i] << " server launched.\n"; 
			i++;
        }
		while(1) {
			for (std::vector<Server>::iterator it = _servers.begin();
				 it < _servers.end(); it++) {
				(*it).check();
			}
		}
		//delete conf;
	}
};
