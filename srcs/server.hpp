#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class Route {
		std::string _location;
		std::string _root;
		std::string _index;
	public:
		Route(std::string location, std::string root, std::string index);
		~Route();
};

class Server {
		unsigned int _port;
		std::string _server_name;
		std::vector<Route>	_routes;
	public:
		Server(char *filename);
		~Server();
};

void	*ft_memset(void *b, int c, size_t len);

