#include "../webserv.hpp"
#include "Route.hpp"

class Server {
	string _name;
	std::vector<string> _listens;
	int _port;
	std::map<string, Route *> _routes;
	int _master_socket;
	struct sockaddr_in _address;

  public:
	Server(JSONNode *server) {
		_port = 80;
        JSONObject datas = server->obj();
		if (datas["server_name"])
			_name = datas["server_name"]->str();
		if (datas["listens"]) {
			JSONList listens = datas["listens"]->lst();
			for (JSONList::iterator i = listens.begin(); i < listens.end(); i++)
				_listens.push_back((*i)->str());
			_port = std::atoi(_listens.front().c_str());
		}
		//_routes["default"] = new Route(datas["root"], datas["return"], datas["index"], datas["autoindex"]);
	}
	void launch() {
		int opt = 1;

		_master_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (_master_socket == 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		if (setsockopt(_master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
					   sizeof(opt)) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		_address.sin_family = AF_INET;
		_address.sin_addr.s_addr = INADDR_ANY;
		_address.sin_port = htons(_port);

		if (bind(_master_socket, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}
		cout << "Listener on port " << _port << "\n";

		if (listen(_master_socket, 3) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		cout << "Waiting for connections ..." << "\n";
	}

	void check() {	
		int new_socket, activity, i, valread, sd;
		char buffer[30000];
		char _404[72] =
        "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 4\n\n404!";

		int max_clients = 30;
		int client_socket[30];
		fd_set readfds;
		int max_sd = _master_socket;



		int addrlen = sizeof(_address);
		for (i = 0; i < max_clients; i++)
			client_socket[i] = 0;
		FD_ZERO(&readfds);
		FD_SET(_master_socket, &readfds);

		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (sd > 0)
				FD_SET(sd, &readfds);
			if (sd > max_sd)
				max_sd = sd;
		}

		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))
			cout << strerror(errno) << "\n";

		if (FD_ISSET(_master_socket, &readfds)) {
			if ((new_socket = accept(_master_socket, (struct sockaddr *)&_address,
									 (socklen_t *)&addrlen)) < 0) {
				cout << strerror(errno) << "\n";
				exit(EXIT_FAILURE);
			}
#ifdef __APPLE__
			fcntl(new_socket, F_GETNOSIGPIPE);
#endif
			cout << "New connection, socket fd is " << new_socket
					  << ", ip is : " << inet_ntoa(_address.sin_addr)
					  << ", port : " << ntohs(_address.sin_port) << "\n";

			for (i = 0; i < max_clients; i++) {
				if (client_socket[i] == 0) {
					client_socket[i] = new_socket;
					cout << "Adding to list of sockets as " << i
							  << "\n";
					break;
				}
			}
		}

		for (i = 0; i < max_clients; i++) {
			sd = client_socket[i];
			if (FD_ISSET(sd, &readfds)) {
				valread = read(sd, buffer, 30000);
				if (valread == 0) {
					getpeername(sd, (struct sockaddr *)&_address,
								(socklen_t *)&addrlen);
					cout << "Host disconnected, ip "
							  << inet_ntoa(_address.sin_addr) << ", port "
							  << ntohs(_address.sin_port) << "\n";
					close(sd);
					client_socket[i] = 0;
				} else
#ifdef __linux__
					send(sd, _404, strlen(_404), MSG_NOSIGNAL);
#elif __APPLE__
					send(sd, _404, strlen(_404), 0);
#endif
					
				}
			}
		}

	
};
