#pragma once
#include "json/Parser.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Route {
    std::vector<std::string> _location;
    std::vector<std::string> _index;
    std::string _root;

  public:
    Route(std::string location, std::string root, std::string index);
    ~Route();
};

class Server {
    unsigned int _port;
    std::string _server_name;
    std::vector<Route> *_routes;

  public:
    Server(std::string name, std::string root, std::string return,
           JSONNode *listens, JSONNode *indexs, JSONNode *locations);
    ~Server();
};

class Env {
  public:
    std::vector<Server> *servers;
    int max_clients;
};

void *ft_memset(void *b, int c, size_t len);
