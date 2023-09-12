#include "common.h"

int main(int argc, char const* argv[]){
	// check the param's length
	client_arg_check(argc, argv);

	// define variables
	int sock = 0;
	struct sockaddr_in serv_addr;

	// client side init	
	client_init(SOCK_STREAM, &sock, &serv_addr, argv);

	// TCP socket will first build the connection
	TCP_connect(&sock, &serv_addr);
    
	// communication
    	// define communication variables
    FILE *local_file_p = fopen(argv[3], "rb");
    char send_buffer[MAX_BYTES + 10];
    int msg_length;
		// send file name type first
	TCP_client_send_filename(&sock, argv);
		// send the file content
    while(msg_length = fread(send_buffer, 1, MAX_BYTES, local_file_p))
        send(sock, send_buffer, msg_length, 0);

	// CLOSE
    shutdown(sock, SHUT_RDWR);
	fclose(local_file_p);
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