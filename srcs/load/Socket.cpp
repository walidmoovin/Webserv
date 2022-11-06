
#include "webserv.hpp"

Socket::Socket(string def) {
    size_t split = def.rfind(':');

    string tmp = def.substr(0, split);
    _ip = isInt(tmp)  || tmp == "localhost" ? "127.0.0.1" : tmp;
    tmp = def.substr(split + 1, def.length() - split - 1).c_str();
    _port = !isInt(tmp) ? 80 : std::atoi(tmp.c_str());

    _max_clients = 30;
    for (int i = 0; i < _max_clients; i++)
        _client_socket[i] = 0;
}
Socket::~Socket() {
	close(_master_socket);
	cout << "Socket destroyed!\n";
}

int Socket::launch() {
    int opt = 1;
    _master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_master_socket == 0) {
        cout << "Socket creation: " << strerror(errno) << "\n";
        return (EXIT_FAILURE);
    }
    int opt_ret = setsockopt(_master_socket, SOL_SOCKET, SO_REUSEADDR,
                             (char *)&opt, sizeof(opt));
    if (opt_ret < 0) {
        cout << "Sockopt: " << strerror(errno) << "\n";
        return (EXIT_FAILURE);
    }

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = inet_addr(_ip.c_str());
    _address.sin_port = htons(_port);

    if (bind(_master_socket, (struct sockaddr *)&_address, sizeof(_address)) <
        0) {
        cout << "Bind: " << strerror(errno) << "\n";
        return (EXIT_FAILURE);
    }
    cout << "Listener " << _ip << " on port " << _port << "\n";

    if (listen(_master_socket, 3) < 0) {
        cout << "Listen: " << strerror(errno) << "\n";
        return (EXIT_FAILURE);
    }
	cout << "Socket::_master_socket: " << _master_socket << "\n";
	if (_master_socket < _min_sd)
		_min_sd = _master_socket;
	_amount++;
	return (EXIT_SUCCESS);
}

void Socket::check() {
    int sd;

    FD_SET(_master_socket, &_readfds);
	cout << "sd_set " << _master_socket << "\n";

    for (int i = 0; i < _max_clients; i++) {
        sd = _client_socket[i];
		//cout << "id: " << i << " -> sd: " << sd << "\n";
        if (sd > 0)
		{
            FD_SET(sd, &_readfds);
			cout << "fd_set " << sd << "\n";
		}
        if (sd > _max_sd)
            _max_sd = sd;
    }
	cout << "Socket checked\n";
}


void Socket::answer() {
	int i, sd, valread;
    int addrlen = sizeof(_address);
    char buffer[1024];
    char r404[72] =
        "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 4\n\n404!";
    if (FD_ISSET(_master_socket, &_readfds)) {
        int new_socket = accept(_master_socket, (struct sockaddr *)&_address,
                            (socklen_t *)&addrlen);
        if (new_socket < 0) {
            cout << "Accept: " << strerror(errno) << "\n";
            exit(EXIT_FAILURE);
        }
#ifdef __APPLE__
        fcntl(new_socket, F_GETNOSIGPIPE);
#endif
        cout << "New connection, socket fd is " << new_socket
             << ", ip is : " << inet_ntoa(_address.sin_addr)
             << ", port : " << ntohs(_address.sin_port) << "\n";
        for (i = 0; i < _max_clients; i++) {
            if (_client_socket[i] == 0) {
                _client_socket[i] = new_socket;
                cout << "Adding to list of sockets as " << i << "\n";
                break;
            }
        }
    }
    cout << "Socket: " << _ip << ":" << _port << "\n";
    for (i = 0; i < _max_clients; i++) {
        sd = _client_socket[i];
        if (FD_ISSET(sd, &_readfds)) {
            cout << "Client " << i << ": set\n";
            valread = read(sd, buffer, 1024);
            if (valread == 0) {
                getpeername(sd, (struct sockaddr *)&_address,
                            (socklen_t *)&addrlen);
                cout << "Host disconnected, ip " << inet_ntoa(_address.sin_addr)
                     << ", port " << ntohs(_address.sin_port) << "\n";
                close(sd);
                _client_socket[i] = 0;
            } else {
                cout << buffer << "\n";
#ifdef __linux__
                send(sd, r404, strlen(r404), MSG_NOSIGNAL);
#elif __APPLE__
                send(sd, r404, strlen(r404), 0);
#endif
            }
        }
    }
	cout << "Socket answered\n";
}
