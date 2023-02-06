#pragma once
#ifndef SILENT
	#define SILENT 1
#endif
#ifndef MAX_CLIENTS
	#define MAX_CLIENTS 5000
#endif

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
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
#include <stdlib.h>

using std::cout;
using std::strerror;
using std::string;

typedef struct ip_port_s {
	int fd;
	string ip;
	int		 port;
} ip_port_t;

class JSONNode;
class Env;
class Server;
class Route;
class Master;
class Client;

typedef std::map<string, JSONNode *> JSONObject;
typedef std::vector<JSONNode *>			 JSONList;
typedef std::vector<string>					 vec_string;

void			*ft_memset(void *b, int c, size_t len);
bool			 isAPort(string str);
vec_string split(string str, string delim);
ip_port_t	 get_ip_port_t(string listen);
ip_port_t	 get_ip_port_t(string ip, int port);
string		 getMime(string path);
string		 read_file(string path);
string		 file_answer(string path);
void debug_block(string name, string content);

#include "Client.hpp"
#include "Master.hpp"
#include "Nodes.hpp"
#include "Token.hpp"
#include "Parser.hpp"
#include "Route.hpp"
#include "Server.hpp"
#include "Env.hpp"
