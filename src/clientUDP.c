#include "common.h"

int main(int argc, char const* argv[]){
	// check the param's length
	client_arg_check(argc, argv);

    // define variables
	int sock = 0;
	struct sockaddr_in serv_addr;

	// client side init	
	client_init(SOCK_DGRAM, &sock, &serv_addr, argv);
    
    // communicate
    FILE *local_file_p = fopen(argv[3], "rb");
    char send_buffer[MAX_BYTES + 10];

		// send the file content
    for(
		int msg_length, seq_num = 0;
		msg_length = fread(send_buffer + 2, 1, MAX_BYTES, local_file_p); 
		seq_num ++
	){
		send_buffer[0] = HIGH_EIGHT(seq_num);
		send_buffer[1] = LOW_EIGHT(seq_num);
		sendto(
				sock, 
				send_buffer, 
				msg_length + 2, 
				0, 
				(struct sockaddr*) &serv_addr, 
				sizeof(serv_addr)
			);

		// printf(
		// 	"Client Send to %s with Bytes %d in seq_num %d\n", 
		// 	inet_ntoa(serv_addr.sin_addr), 
		// 	msg_length + 2,
		// 	seq_num
		// );
	}

	// close the server-side connection
	UDP_end(&sock, &serv_addr);

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