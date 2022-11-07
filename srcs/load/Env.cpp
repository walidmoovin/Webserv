#include "webserv.hpp"

Env::Env(JSONNode *conf) {
	try {
		JSONList servers = conf->obj()["servers"]->lst();
		int i = 0;
		string th[8] = {"first", "second", "third",	  "fourth",
						"fifth", "sixth",  "seventh", "eigth"};
		for (std::vector<JSONNode *>::iterator it = servers.begin();
			 it < servers.end(); it++) {
			Server *server = new Server(*it);
			_servers.push_back(server);
			std::vector<Socket *> tmp_s = server->get_sockets(*it);
			_sockets.insert(_sockets.end(), tmp_s.begin(), tmp_s.end());
			// delete *it;
			cout << th[i] << " server launched.\n";
			i++;
		}
	} catch (std::exception &e) {
		cout << e.what();
	}
	delete conf;
}

Server *Env::choose_server(Socket *sock, string host) {
	std::vector<Server *> exact;
	std::vector<Server *> inrange;
	std::vector<string> ip_list;
	std::vector<string> ip_requ;
	// string ip = inet_ntoa(sock->_address.sin_addr);
	// int port = ntohs(sock->_address.sin_port);

	// cout << "Which server for " << ip << ":" << port << "?\n";
	cout << "Socket: " << sock->_listen.ip << ":" << sock->_listen.port << "\n";
	ip_requ = split(sock->_listen.ip, '.');
	for (std::vector<Server *>::iterator sit = _servers.begin();
		 sit < _servers.end(); sit++) {

		std::vector<listen_t> serv_listens = (*sit)->_listens;
		for (std::vector<listen_t>::iterator it = serv_listens.begin();
			 it < serv_listens.end(); it++) {

			ip_list = split((*it).ip, '.');
			if (sock->_listen.port != (*it).port)
				continue;
			if (sock->_listen.ip == (*it).ip) {
				exact.push_back(*sit);
				continue;
			}
			bool is_inrange = true;
			ip_list = split((*it).ip, '.');
			std::vector<string>::iterator r = ip_requ.begin();
			for (std::vector<string>::iterator l = ip_list.begin();
				 l < ip_list.end(); l++) {
				if (*l != *r && *l != "0")
					is_inrange = false;
			}
			if (is_inrange == true)
				inrange.push_back(*sit);
			// else if (is_ip_into(sock->_listen.ip, (*it).ip))
		}
	}
	if (exact.size() == 0) {
		for (std::vector<Server *>::iterator sit = inrange.begin();
			 sit < inrange.end(); sit++) {
			if (host == (*sit)->getName())
				return *sit;
		}
		return inrange.front();
	} else
		return exact.front();
}

void Env::set_fds(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++)
		(*it)->set_fds();
}

void Env::refresh(void) {
	for (std::vector<Socket *>::iterator it = _sockets.begin();
		 it < _sockets.end(); it++)
		(*it)->refresh(this);
}
