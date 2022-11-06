#pragma once
#include "webserv.hpp"


class JSONNode;
class Server;

class Env {
    std::vector<Server> _servers;
  public:
    Env(JSONNode *conf);
};
