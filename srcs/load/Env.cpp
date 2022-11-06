#include "webserv.hpp"

Env::Env(JSONNode *conf) {
    JSONList servers = conf->obj()["servers"]->lst();
    int i = 0;
    string th[8] = {"first", "second", "third",   "fourth",
                    "fifth", "sixth",  "seventh", "eigth"};
    for (std::vector<JSONNode *>::iterator it = servers.begin();
         it < servers.end(); it++) {
        Server *server = new Server(*it);
        _servers.push_back(server);
        // delete *it;
        cout << th[i] << " server launched.\n";
        i++;
    }
    // delete conf;
}
void Env::set_fds() {
    for (std::vector<Server *>::iterator it = _servers.begin();
         it < _servers.end(); it++) {
        (*it)->set_fds();
    }
}

void Env::refresh() {
    for (std::vector<Server *>::iterator it = _servers.begin();
         it < _servers.end(); it++) {
        (*it)->refresh();
    }
}
