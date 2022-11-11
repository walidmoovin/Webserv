#include "webserv.hpp"

/*|==========|
 * Environment destructor:
 *
 * The destructor call all servers and sockets destructors.
 */
Env::~Env() {
	for (std::vector< Server * >::iterator it = _servers.begin();
		 it < _servers.end(); it++) {
		delete *it;
	}
	for (std::vector< Master * >::iterator it = _masters.begin();
		 it < _masters.end(); it++) {
		delete *it;
	}
}

/*|==========|
 * Environment constructor:
 *
 * Input: The JSONParser output
 * Output: The env object containing servers and sockets vectors defined inside
 * conf file by servers blocks and listens.
 */
Env::Env(JSONNode *conf) {
	try {

		JSONList servers = conf->obj()["servers"]->lst();
		for (std::vector< JSONNode * >::iterator it = servers.begin();
			 it < servers.end(); it++) {
			Server *server = new Server(*it);
			_servers.push_back(server);
			std::vector< Master * > tmp_s = server->get_sockets(*it);
			_masters.insert(_masters.end(), tmp_s.begin(), tmp_s.end());
		}
	} catch (std::exception &e) {
		std::cerr << e.what() << "\n";
	}
	delete conf;
}
/*|==========|
 * One server cycle
 * - append sockets to listen to select list
 * - select them
 * - refresh and handle requests
 */

void Env::cycle(void) {
	FD_ZERO(&Master::_readfds);
	Master::_max_fd = Master::_min_fd;
	cout << "==> Check sockets still alive to listen\n";
	set_fds();
	cout << "|===||===| Waiting some HTTP request... |===||===|\n";
	int activity = select(Master::_max_fd + Master::_amount,
						  &(Master::_readfds), NULL, NULL, NULL);
	if ((activity < 0) && (errno != EINTR))
		std::cerr << "Select: " << strerror(errno) << "\n";
	cout << "==> Handle requests and answers:\n";
	refresh();
}
/*|==========|
 * Append each master_sockets and their clients to list of fds SELECT must look
 * at.
 */

void Env::set_fds(void) {
	for (std::vector< Master * >::iterator it = _masters.begin();
		 it < _masters.end(); it++) {
		(*it)->set_fds();
	}
}
/*|==========|
 * Refresh all master_sockets and their clients datas (disconnect, new
 * connection, etc..) and parse requests recieved.
 */

void Env::refresh(void) {
	for (std::vector< Master * >::iterator it = _masters.begin();
		 it < _masters.end(); it++)
		try {
			(*it)->refresh(this);
		} catch (std::exception &e) {
			std::cerr << e.what();
		}
}
