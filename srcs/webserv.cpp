#include "webserv.hpp"
#include <time.h>

fd_set Socket::_readfds;
int Socket::_max_sd;
int Socket::_min_sd = INT_MAX;
int Socket::_amount = 0;

int main(int ac, char **av) {

    if (ac > 1) {

        cout << "Parsing configuration file...\n";
        JSONParser parser(av[1]);
        JSONNode *conf = parser.parse();
        cout << "Configuration parsed.\n";

        cout << "Setting environment...\n";
        Env env(conf);
        while (1) {
			cout << "Cycling...\n";
			FD_ZERO(&Socket::_readfds);
			Socket::_max_sd = Socket::_min_sd;
            env.listen();
			cout << "Socket::_max_sd " << Socket::_max_sd << "\n";
			int activity = select(Socket::_max_sd + Socket::_amount, &(Socket::_readfds), NULL, NULL, NULL);
			if ((activity < 0) && (errno != EINTR))
				cout << "Select: " << strerror(errno) << "\n";
			env.answer();
		}
	}
    return (0);
}
