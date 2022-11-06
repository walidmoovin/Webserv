#include "../webserv.hpp"
#include "Route.hpp"

class Server {
	string _name;
	std::vector<string> _listens;
	int _port;
	std::map<string, Route *> _routes;

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
		int master_socket, addrlen, new_socket, client_socket[30],
			max_clients = 30, activity, i, valread, sd;
		struct sockaddr_in address;
		char buffer[30000];
		char _404[72] =
        "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 4\n\n404!";
		int max_sd;
		fd_set readfds;

		for (i = 0; i < max_clients; i++)
			client_socket[i] = 0;

		if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
					   sizeof(opt)) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(_port);

		if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}
		cout << "Listener on port " << _port << "\n";

		if (listen(master_socket, 3) < 0) {
			cout << strerror(errno) << "\n";
			exit(EXIT_FAILURE);
		}

		addrlen = sizeof(address);
		cout << "Waiting for connections ..." << "\n";

		while (1) {
			FD_ZERO(&readfds);
			FD_SET(master_socket, &readfds);
			max_sd = master_socket;

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

			if (FD_ISSET(master_socket, &readfds)) {
				if ((new_socket = accept(master_socket, (struct sockaddr *)&address,
										 (socklen_t *)&addrlen)) < 0) {
					cout << strerror(errno) << "\n";
					exit(EXIT_FAILURE);
				}
#ifdef __APPLE__
				fcntl(new_socket, F_GETNOSIGPIPE);
#endif
				cout << "New connection, socket fd is " << new_socket
						  << ", ip is : " << inet_ntoa(address.sin_addr)
						  << ", port : " << ntohs(address.sin_port) << "\n";

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
						getpeername(sd, (struct sockaddr *)&address,
									(socklen_t *)&addrlen);
						cout << "Host disconnected, ip "
								  << inet_ntoa(address.sin_addr) << ", port "
								  << ntohs(address.sin_port) << "\n";
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

	}
};
