#include "webserv.hpp"

/*|=======================|
 * Environment constructor:
 *
 * Input: The JSONParser output
 * Output: The env object containing servers and sockets vectors defined inside
 * conf file by servers blocks and listens.
 */
Env::Env(JSONNode *conf) {
	try {
		JSONList servers = conf->obj()["servers"]->lst();
		for (std::vector<JSONNode *>::iterator it = servers.begin();
			 it < servers.end(); it++) {
			Server *server = new Server(*it);
			_servers.push_back(server);
			std::vector<Socket *> tmp_s = server->get_sockets(*it);
			_sockets.insert(_sockets.end(), tmp_s.begin(), tmp_s.end());
		}
	} catch (std::exception &e) {
		cout << e.what();
	}
	delete conf;
}

/*|=======================|
 * Append each master_sockets and their clients to list of fds SELECT must look
 * at.
 */

void Env::set_fds(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++)
		(*it)->set_fds();
}

/*|=======================|
 * Refresh all master_sockets and their clients datas (disconnect, new
 * connection, etc..) and parse requests recieved.
 */

void Env::refresh(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++)
		(*it)->refresh(this);
}

/*|=======================|
 * Environment destructor:
 *
 * The destructor call all servers and sockets destructors.
 */
Env::~Env() {
	for (std::vector<Server *>::iterator it = _servers.begin();
		 it < _servers.end(); it++) {
		delete *it;
	}
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++) {
		delete *it;
	}
}
