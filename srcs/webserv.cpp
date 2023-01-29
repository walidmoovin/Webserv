#include "webserv.hpp"

int						 Master::_poll_id_amount = 0; // number of sockets
int						 Master::_first_cli_id = 0;
struct pollfd *Master::_pollfds = new struct pollfd[MAX_CLIENTS + 1];

/**
 * @brief The server launcher
 *
 * @param ac arguments count, either 1 or 2, if 1, the default config file is used
 * @param av arguments values: executable name and an optional config file
 *
 * @return return EXIT_FAILURE if an exception occur
 */
/* *******************************/
int main(int ac, char **av) {
	try {
		if (ac > 2) throw std::runtime_error("Too many arguments");
		std::string config_file = "default.json";
		if (ac == 2) config_file = av[1];
		std::ifstream file(config_file.c_str());
		if (!file.good()) throw std::runtime_error("File not found");
		cout << "Parsing configuration file from JSON conf file.\n";
		cout << "You must be sure the syntax is correct\n";
		JSONParser parser(config_file);
		JSONNode	*conf = parser.parse();
		if (!conf) return EXIT_FAILURE;
		cout << "Initialization of server...\n";
		// here we memset the pollfds to 0, because we don't want to have garbage values
		std::memset(Master::_pollfds, 0, sizeof(*Master::_pollfds) * (MAX_CLIENTS));
		Env env(conf);
		while (1) env.cycle();
    	delete[] Master::_pollfds;
	} catch (const std::exception &e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
