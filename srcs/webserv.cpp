#include "webserv.hpp"

int main(int ac, char **av) {

    if (ac > 1) {

        cout << "Parsing configuration file...\n";
        JSONParser parser(av[1]);
        JSONNode *conf = parser.parse();
        cout << "Configuration parsed.\n";

        cout << "Setting environment...\n";
		Env env(conf);
    }
    return (0);
}
