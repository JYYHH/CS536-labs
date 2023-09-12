#include "common.h"

int main(int argc, char const* argv[]){
    // check the param's lengths
	server_arg_check(argc, argv);

    // define variables
	int sock = 0;
	struct sockaddr_in address;

	// server side init
	server_init(SOCK_DGRAM, &sock, &address, argv);

	// server side serving
	UDP_server_handling(&sock, &address);
	// closing the listening socket
	shutdown(sock, SHUT_RDWR);
	return 0;
}

//    struct sockaddr_in {
//        sa_family_t    sin_family; /* address family: AF_INET */
//        in_port_t      sin_port;   /* port in network byte order */
//        struct in_addr sin_addr;   /* internet address */
//    };

//    /* Internet address */
//    struct in_addr {
//        uint32_t       s_addr;     /* address in network byte order */
//    };