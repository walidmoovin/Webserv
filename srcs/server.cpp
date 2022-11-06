#include "webserv.hpp"
#include "json/Parser.hpp"
#include "load/Env.hpp"

int main(int ac, char **av) {
    Env env;

    if (ac > 1) {

        cout << "Parsing configuration file...\n";
        JSONParser parser(av[1]);
        JSONNode *conf = parser.parse();
        cout << "Configuration parsed.\n";

        cout << "Setting environment...\n";
        env.max_clients = conf->obj()["max_clients"]->nbr();
        JSONList servers = conf->obj()["servers"]->lst();
		int i = 0;
		string th[8]= {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eigth"};
        for (std::vector<JSONNode *>::iterator it = servers.begin();
             it < servers.end(); it++) {
            Server *server = new Server(*it);
			delete *it;
			server->launch();
			cout << th[i] << " server launched."; 
			i++;
        }
		delete conf;
    }
    return (0);
}
