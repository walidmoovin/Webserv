#pragma once

#define DEBUG 0

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
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

using std::cout;
using std::strerror;
using std::string;

typedef struct listen_s {
	int	   fd;
	string ip;
	int	   port;
} listen_t;

class JSONNode;
class Env;
class Server;
class Route;
class Master;
class Client;

typedef std::map< string, JSONNode * > JSONObject;
typedef std::vector< JSONNode * >	   JSONList;

void				 *ft_memset(void *b, int c, size_t len);
bool				  isInt(string str);
std::vector< string > split(string str, string delim);
listen_t			  get_listen_t(string listen);
listen_t			  get_listen_t(string ip, int port);
string				  getMime(string path);
string				  read_file(string path);

#include "Client.hpp"
#include "Master.hpp"

#include "Nodes.hpp"
#include "Token.hpp"
#include "Parser.hpp"
#include "Route.hpp"
#include "Server.hpp"
#include "Env.hpp"
