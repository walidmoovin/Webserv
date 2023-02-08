#include "webserv.hpp"

int						 Master::_poll_id_amount = 0;
int						 Master::_first_cli_id = 0;
struct pollfd *Master::_pollfds = new struct pollfd[MAX_CLIENTS + 1];


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
