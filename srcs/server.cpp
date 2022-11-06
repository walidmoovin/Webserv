#include "server.hpp"
#define PORT 80

int main(void)
{
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, valread, sd;
	int max_sd;
    struct sockaddr_in address;
     
    char buffer[30000];
     
    fd_set readfds;
     
	char _404[72] = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 4\n\n404!";
 
    for (i = 0; i < max_clients; i++) client_socket[i] = 0;
     
    if( (master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    {
		std::cout << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
 
    if ( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
		std::cout << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
     
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
		std::cout << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
	std::cout << "Listener on port " << PORT << std::endl;
	
    if (listen(master_socket, 3) < 0)
    {
		std::cout << std::strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
     
    addrlen = sizeof(address);
    std::cout << "Waiting for connections ..." << std::endl;
    
	while(1) 
    {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
		
        for ( i = 0 ; i < max_clients ; i++) 
        {
			sd = client_socket[i];
			if(sd > 0) FD_SET( sd, &readfds);
            if(sd > max_sd) max_sd = sd;
        }
 
        activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);
   
        if ((activity < 0) && (errno!=EINTR)) 
            std::cout << std::strerror(errno) << std::endl;
         
        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
				std::cout << std::strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
			fcntl(new_socket, F_GETNOSIGPIPE);
         
			std::cout << "New connection, socket fd is " << new_socket \
				<< ", ip is : " << inet_ntoa(address.sin_addr) \
				<< ", port : " << ntohs(address.sin_port) << std::endl;  
             
            for (i = 0; i < max_clients; i++) 
            {
				if( client_socket[i] == 0 )
                {
                    client_socket[i] = new_socket;
					std::cout << "Adding to list of sockets as " << i << std::endl;
					break;
                }
            }
        }
         
        for (i = 0; i < max_clients; i++) 
        {
            sd = client_socket[i];
            if (FD_ISSET( sd, &readfds)) 
            {
				valread = read(sd, buffer,30000);
                if (valread == 0)
                {
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
					std::cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr)\
							<< ", port " << ntohs(address.sin_port) << std::endl;
                    close( sd );
                    client_socket[i] = 0;
                }
                else send(sd, _404, strlen(_404), 0);
            }
        }
    }
    return (0);
}
