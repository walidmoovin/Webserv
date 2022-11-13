#include "webserv.hpp"

fd_set Master::_readfds;
int	   Master::_min_fd = INT_MAX;
int	   Master::_max_fd = 0;
int	   Master::_amount = 0;

int main(int ac, char **av) {
	try {
		if (ac > 2)
			throw std::runtime_error("Too many arguments");

		std::string config_file = "default.json";
		if (ac == 2)
			config_file = av[1];

		std::ifstream file(config_file.c_str());
		if (!file.good())
			throw std::runtime_error("File not found");

		cout << "Parsing configuration file from JSON conf file.\n";
		cout << "You must be sure the syntax is correct\n";
		JSONParser parser(config_file);
		JSONNode  *conf = parser.parse();

		cout << "Initialization of server...\n";
		Env env(conf);
		while (1)
			env.cycle();
	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
