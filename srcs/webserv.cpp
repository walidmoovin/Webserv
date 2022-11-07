#include "webserv.hpp"

fd_set Socket::_readfds;
int Socket::_max_fd;
int Socket::_min_fd = INT_MAX;
int Socket::_amount = 0;

int main(int ac, char **av) {

	if (ac > 1) {

		cout << "Parsing configuration file...\n";
		JSONParser parser(av[1]);
		JSONNode *conf = parser.parse();
		cout << "Configuration parsed.\n";

		cout << "Setting environment...\n";
		Env env(conf);
		cout << "Environement setup.\n";

		while (1) {
			FD_ZERO(&Socket::_readfds);
			Socket::_max_fd = Socket::_min_fd;
			env.set_fds();
			cout << "|===|===|===| SELECT |===|===|===|\n";
			int activity = select(Socket::_max_fd + Socket::_amount,
								  &(Socket::_readfds), NULL, NULL, NULL);
			if ((activity < 0) && (errno != EINTR))
				cout << "Select: " << strerror(errno) << "\n";
			env.refresh();
		}
	}
	return (0);
}
