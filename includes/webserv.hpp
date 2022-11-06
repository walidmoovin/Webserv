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

class JSONNode;
typedef std::map<string, JSONNode *> JSONObject;
typedef std::vector<JSONNode *> JSONList;

void *ft_memset(void *b, int c, size_t len);
bool isInt(string str);

#include "Token.hpp"
#include "Parser.hpp"
#include "Nodes.hpp"

#include "Env.hpp"
#include "Route.hpp"
#include "Socket.hpp"
#include "Server.hpp"

