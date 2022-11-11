#include "webserv.hpp"

fd_set Master::_readfds;
int	   Master::_max_fd;
int	   Master::_min_fd = INT_MAX;
int	   Master::_amount = 0;

int main(int ac, char **av) {
	if (ac != 2) {
		cout << "Usage:\n./webserv CONF.json\n";
		return EXIT_SUCCESS;
	}

	std::ifstream file(av[1]);
	if (!file.good()) {
		cout << "Error: " << av[1] << " is not a valid file\n";
		return EXIT_FAILURE;
	}
	cout << "Parsing configuration file from JSON conf file.\n";
	cout << "You must be sure the syntax is correct\n";
	JSONParser parser(av[1]);
	JSONNode  *conf = parser.parse();
	cout << "Initialization of server...\n";
	Env env(conf);
	while (1)
		env.cycle();
	return EXIT_SUCCESS;
}
