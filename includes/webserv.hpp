#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define DEBUG 0

using std::cout;
using std::strerror;
using std::string;

class Env;
class Server;
class Socket;
class Route;
class JSONNode;

typedef std::map<string, JSONNode *> JSONObject;
typedef std::vector<JSONNode *> JSONList;

void *ft_memset(void *b, int c, size_t len);
bool isInt(string str);

#include "Nodes.hpp"
#include "Token.hpp"
#include "Parser.hpp"

#include "Route.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Env.hpp"
