/**
 * @file webserv.cpp
 * @brief A http server written in c++
 * @author Narnaud
 * @version 0.1
 * @date 2023-01-12
 */

#include "webserv.hpp"

fd_set Master::_readfds;					/// < The sockets fd which will be select
int		 Master::_min_fd = INT_MAX; /// < The lower socket fd
int		 Master::_max_fd = 0;				/// < The higher one

/* *******************************/
/**
 * @brief The server launcher
 *
 * @param ac arguments count, either 1 or 2.
 * @param av arguments values: executable name and an optional config file
 *
 * @return return EXIT_FAILURE if an exception occur
 */
/* *******************************/
int main(int ac, char **av) {
	try {
		// Parse config file
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
		// Here we start the server and his environment using conf
		cout << "Initialization of server...\n";
		Env env(conf);
		while (1) env.cycle();
	} catch (const std::exception &e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
