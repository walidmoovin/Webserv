
#include "webserv.hpp"

Socket::Socket(string def) {
    size_t split = def.rfind(':');

    string tmp = def.substr(0, split - 1);
    _ip = isInt(tmp) ? "localhost" : tmp;
    tmp = def.substr(split + 1, def.length() - split - 1).c_str();
    _port = !isInt(tmp) ? 80 : std::atoi(tmp.c_str());

    _max_clients = 30;

    int opt = 1;
    _master_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_master_socket == 0) {
        cout << "Socket creation: " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }
    int opt_ret = setsockopt(_master_socket, SOL_SOCKET, SO_REUSEADDR,
                             (char *)&opt, sizeof(opt));
    if (opt_ret < 0) {
        cout << "Sockopt: " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);

    if (bind(_master_socket, (struct sockaddr *)&_address, sizeof(_address)) <
        0) {
        cout << "Bind: " << strerror(errno) << "\n";
        cout << "Socket destroyed!\n";
        return;
        exit(EXIT_FAILURE);
    }
    cout << "Listener on port " << _port << "\n";

    if (listen(_master_socket, 3) < 0) {
        cout << "Listen: " << strerror(errno) << "\n";
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < _max_clients; i++)
        _client_socket[i] = 0;
}
void Socket::check() {
    int new_socket, activity, i, valread, sd;
    char buffer[1024];
    char r404[72] =
        "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 4\n\n404!";

    fd_set readfds;
    int max_sd = _master_socket;

    int addrlen = sizeof(_address);
    FD_ZERO(&readfds);
    FD_SET(_master_socket, &readfds);

    for (i = 0; i < _max_clients; i++) {
        sd = _client_socket[i];
        if (sd > 0)
            FD_SET(sd, &readfds);
        if (sd > max_sd)
            max_sd = sd;
    }

    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR))
        cout << "Select: " << strerror(errno) << "\n";

    if (FD_ISSET(_master_socket, &readfds)) {
        new_socket = accept(_master_socket, (struct sockaddr *)&_address,
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
        if (FD_ISSET(sd, &readfds)) {
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
}
