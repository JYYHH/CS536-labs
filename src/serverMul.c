#include "common.h"
#define NUM_TOTAL_THREAD 200 // define the max number of threads to receive client request

int main(int argc, char const* argv[]){
	// sign the memory for thread data structure
	thread_arr 
		= (pthread_t *)
			malloc(
				sizeof(pthread_t) 
					* 
				NUM_TOTAL_THREAD
			);

	// check the param's lengths
	server_arg_check(argc, argv);

    // define variables
	int sock = 0;
	struct sockaddr_in address;

	// server side init
	server_init(SOCK_STREAM, &sock, &address, argv);

	// Wait From the client
	while (1){ // handling loop
		// params DS
		struct pthread_params *ptr 
			= (struct pthread_params *)
				malloc(
					sizeof(
						struct pthread_params
					)
				);
		// server_file_descriptor
		ptr->sfd = TCP_accept_with_server_fd(&sock, &address);
		// client address captured
		ptr->address = address;
		// In this case, create a thread to handle this request
		pthread_create(
			&thread_arr[present_thread ++], 
			NULL, 
			TCP_server_handling, 
			(void *)ptr
		);
	}
	// We know we won't exit here..., so no close/shut_down/return 0
}

// 0 -> none suffix
// 1 -> .txt
// 2 -> .mp4
// 3 -> .jpeg
// 4 -> .html

//    struct sockaddr_in {
//        sa_family_t    sin_family; /* address family: AF_INET */
//        in_port_t      sin_port;   /* port in network byte order */
//        struct in_addr sin_addr;   /* internet address */
//    };

//    /* Internet address */
//    struct in_addr {
//        uint32_t       s_addr;     /* address in network byte order */
//    };