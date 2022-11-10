#include "webserv.hpp"

fd_set Socket::_readfds;
int Socket::_max_fd;
int Socket::_min_fd = INT_MAX;
int Socket::_amount = 0;

int main(int ac, char **av) {
	if (ac <= 2) {
		std::string config = "default.json";
		if (ac == 2){
			std::ifstream file(av[1]);
			if (file.good())
				config = av[1];
			else {
				std::cout << "Error: " << av[1] << " is not a valid file" << std::endl;
				return EXIT_FAILURE;
			}
			config = av[1];
		}
		cout << "Parsing configuration file from JSON conf file.\n";
		cout << "You must be sure the syntax is correct\n";
		JSONParser parser(config);
		JSONNode *conf = parser.parse();
		cout << "Initialization of server...\n";
		Env env(conf);

		while (1) {
			FD_ZERO(&Socket::_readfds);
			Socket::_max_fd = Socket::_min_fd;
			cout << "==> Check sockets still alive to listen\n";
			env.set_fds();
			cout << "|===||===| Waiting some HTTP request... |===||===|\n";
			int activity = select(Socket::_max_fd + Socket::_amount,
								&(Socket::_readfds), NULL, NULL, NULL);
			if ((activity < 0) && (errno != EINTR))
				cout << "Select: " << strerror(errno) << "\n";
			cout << "==> Handle requests and answers:\n";
			env.refresh();
		}
	}
	else
		cout << "Usage:\n./webserv CONF.json\n";
	return (0);
}
