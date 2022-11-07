#pragma once

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <map>
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
std::vector<string> split(string str, char delim);

#include "Nodes.hpp"
#include "Token.hpp"
#include "Parser.hpp"

#include "Route.hpp"
#include "Socket.hpp"
#include "Server.hpp"
#include "Env.hpp"
