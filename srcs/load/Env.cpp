#include "webserv.hpp"

/*|==========|
 * Environment constructor:
 *
 * Input: The JSONParser output
 * Output: The env object containing servers and sockets vectors defined inside
 * conf file by servers blocks and listens.
 */
Env::Env(JSONNode *conf)
{
	try {
		JSONNode *node;
		JSONList lst;
		if ((node = conf->obj()["servers"]))
		{
			lst = conf->obj()["servers"]->lst();
			for (std::vector< JSONNode * >::iterator it = lst.begin(); it < lst.end(); it++)
			{
				Server *server = new Server(*it);
				this->_servers.push_back(server);
				std::vector< Master * > tmp_s = server->get_sockets(*it);
				this->_masters.insert(this->_masters.end(), tmp_s.begin(), tmp_s.end());
			}
		}
		if ((node = conf->obj()["allowed_methods"]))
		{
			JSONList lst = node->lst();
			for (JSONList::iterator it = lst.begin(); it < lst.end(); it++)
			{
				this->_allowed_methods.push_back((*it)->str());
			}
		}
	} catch (std::exception &e)
	{
		std::cerr << e.what() << "\n";
	}
	delete conf;
}

/*|==========|
 * Environment destructor:
 *
 * The destructor call all servers and sockets destructors.
 */
Env::~Env()
{
	for (std::vector< Server * >::iterator it = this->_servers.begin();
		 it < this->_servers.end(); it++) {
		delete *it;
	}
	for (std::vector< Master * >::iterator it = this->_masters.begin();
		 it < this->_masters.end(); it++) {
		delete *it;
	}
}

/*|==========|
 * One server cycle
 * - append sockets to listen to select list
 * - select them
 * - refresh and handle requests
 */

void Env::cycle(void)
{
	FD_ZERO(&Master::_readfds);
	Master::_max_fd = Master::_min_fd;
	cout << "==> Check sockets still alive to listen\n";
	set_fds();
	cout << "|===||===| Waiting some HTTP request... |===||===|\n";
	int activity = select(Master::_max_fd + Master::_amount, &(Master::_readfds), NULL, NULL, NULL);
	if ((activity < 0) && (errno != EINTR))
		std::cerr << "Select: " << strerror(errno) << "\n";
	cout << "==> Handle requests and answers:\n";
	refresh();
}
/*|==========|
 * Append each master_sockets and their clients to list of fds SELECT must look
 * at.
 */

void Env::set_fds(void)
{
	for (std::vector< Master * >::iterator it = this->_masters.begin(); it < this->_masters.end(); it++)
	{
		(*it)->set_fds();
	}
}
/*|==========|
 * Refresh all master_sockets and their clients datas (disconnect, new
 * connection, etc..) and parse requests recieved.
 */

void Env::refresh(void)
{
	for (std::vector< Master * >::iterator it = this->_masters.begin(); it < this->_masters.end(); it++)
		try {
			(*it)->refresh(this);
		} catch (std::exception &e) {
			std::cerr << e.what();
		}
}
