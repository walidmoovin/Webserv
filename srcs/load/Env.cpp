/**
 * @file Env.cpp
 * @brief The main server object. Contain all servers and sockets.
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-17
 */
#include "webserv.hpp"

/**
 * @brief Constructor
 *
 * The instance contain servers defined in configuration file by servers list and sockets by listens ones.
 *
 * @param conf The JsonParser output
 */
Env::Env(JSONNode *conf) {
	try {
		JSONNode *node;
		JSONList	lst;
		if ((node = conf->obj()["servers"])) {
			lst = conf->obj()["servers"]->lst();
			for (std::vector<JSONNode *>::iterator it = lst.begin(); it < lst.end(); it++) {
				Server *server = new Server(*it);
				this->_servers.push_back(server);
				std::vector<Master *> tmp_s = server->create_masters(*it);
				this->_masters.insert(this->_masters.end(), tmp_s.begin(), tmp_s.end());
			}
		}
		Master::_first_cli_id = Master::_poll_id_amount;
		if ((node = conf->obj()["allowed_methods"])) {
			JSONList lst = node->lst();
			for (JSONList::iterator it = lst.begin(); it < lst.end(); it++) {
				this->_allowed_methods.push_back((*it)->str());
			}
		}
	} catch (std::exception &e) { std::cerr << e.what() << "\n"; }
	delete conf;
}

/**
 * @brief Destructor.
 *
 * The destructor call all servers and sockets destructors.
 */
Env::~Env() {
	for (std::vector<Server *>::iterator it = this->_servers.begin(); it < this->_servers.end(); it++) { delete *it; }
	for (std::vector<Master *>::iterator it = this->_masters.begin(); it < this->_masters.end(); it++) { delete *it; }
}

/**
 * @brief A Wevserv cycle:
 * - poll the Master::pollfds array
 * - for each master socket, call his own post_poll method to check and handle the sockets flaged.
 */
void Env::cycle(void) {
	// if (!SILENT) cout << "|===||===| Waiting some HTTP request... |===||===|\n";
	int pollResult = poll(Master::_pollfds, Master::_poll_id_amount + 1, 5000);
	if ((pollResult < 0) && (errno != EINTR)) std::cerr << "Select: " << strerror(errno) << "\n";
	if (pollResult > 0) {
		// if (!SILENT) cout << "==> Handle requests and answers:\n";
		for (std::vector<Master *>::iterator it = this->_masters.begin(); it < this->_masters.end(); it++) try {
				(*it)->check_socket();
				(*it)->check_childs(this);
			} catch (std::exception &e) { std::cerr << e.what(); }
	}
}
