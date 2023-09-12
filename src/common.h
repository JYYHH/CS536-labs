//    struct sockaddr_in {
//        sa_family_t    sin_family; /* address family: AF_INET */
//        in_port_t      sin_port;   /* port in network byte order */
//        struct in_addr sin_addr;   /* internet address */
//    };

//    /* Internet address */
//    struct in_addr {
//        uint32_t       s_addr;     /* address in network byte order */
//    };

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#define MAX_BYTES 1024
#define UDP_TRY 100
#define MY_MASK         ((1 << 8) - 1)
#define HIGH_EIGHT(x)   (((x) >> 8) & MY_MASK)
#define LOW_EIGHT(x)    ((x) & MY_MASK)

struct pthread_params{
	int sfd;
	struct sockaddr_in address;
};

int present_thread;
pthread_t *thread_arr; // define threads DS
int *global_server_sock_p;

// The purpose of this function is helping server jumping out of serving loop safely 
// In default, only TCP-like protocol will wake this up
void sigint_handler(int sig){
    printf("\nCaught SIGINT, AND The server will quit safely!\n");
    // wait for all sub-threads
	for(int i=0;i<present_thread;i++)
		pthread_join(thread_arr[i], NULL);
	// closing the listening socket
	shutdown(*global_server_sock_p, SHUT_RDWR);
    exit(0);   
}     

static inline void client_arg_check(
                        int argc, 
                        char const* argv[]
                    ){

    // check the length of args
	if (argc != 4){
		printf("Usage: %s ip_addr/url port_num msg\n", argv[0]);
		exit(3);
	}
}   

static inline void client_init(
                        int type, 
                        int *sock_p, 
                        struct sockaddr_in *address,  
                        char const* argv[]
                    ){
                            
    // build TCP/UDP sock
	if ((*sock_p = socket(AF_INET, type, 0)) < 0) {
		printf("\n Socket creation error \n");
		exit(1);
	}

	// URL2IP_binary 
	address->sin_family = AF_INET;
	address->sin_port = htons(atoi(argv[2]));
    struct hostent *host = gethostbyname(argv[1]);
    if (host == NULL){
		printf("\nInvalid Address not supported \n");
		exit(2);
    }
    address->sin_addr = *((struct in_addr *)(host->h_addr));
}

static inline void TCP_connect(
                        int *sock_p, 
                        struct sockaddr_in *address
                    ){

    // try to connect the server (TCP only)
	if (
        connect(
                *sock_p, 
                (struct sockaddr *)address, 
                sizeof(*address)
            )
        < 0
       ) {
		printf("\nConnection Failed \n");
		exit(4);
	}
}

static inline void TCP_client_send_filename(
                        int *sock_p, 
                        char const* argv[]
                    ){

    // send the header (here is the type of a file)
// 0 -> none suffix
// 1 -> .txt
// 2 -> .mp4
// 3 -> .jpeg
// 4 -> .html
	int pt = strlen(argv[3]) - 1;
	char new_file_name;
	while(pt >= 0 && argv[3][pt] != '.') pt--;
	if (pt < 0)
		new_file_name = 0;
	else if(argv[3][pt+1] == 't')
		new_file_name = 1;
	else if(argv[3][pt+1] == 'm')
		new_file_name = 2;
	else if(argv[3][pt+1] == 'j')
		new_file_name = 3;
	else if(argv[3][pt+1] == 'h')
		new_file_name = 4;
	send(*sock_p, &new_file_name, 1, 0);
}

static inline void UDP_end(
                        int *sock_p, 
                        struct sockaddr_in *address
                    ){

    char send_buffer[2] = {0};
	for(int i=0;i<UDP_TRY;i++) // try to shut down the server
		sendto(
            *sock_p, 
            send_buffer, 
            2, 
            0, 
			(struct sockaddr *)address, 
            sizeof(*address)
        );
    // We assume that SO MANY empty packages
    // won't influence future serving
}

static inline void server_arg_check(
                        int argc, 
                        char const* argv[]
                    ){
                        
    // check the argv
    if (argc != 2){
		printf("Usage: %s port_num\n", argv[0]);
		exit(3);
	}
}  

static inline void server_init(
                        int type, 
                        int *sock_p, 
                        struct sockaddr_in *address,  
                        char const* argv[]
                    ){
    
    int opt = 1;
    present_thread = 0; // in order to quit correctly, we need to set this
    global_server_sock_p = sock_p;

    // Creating TCP/UDP socket
	if ((*sock_p = socket(AF_INET, type, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(*sock_p, SOL_SOCKET, 
            SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY; // any incoming client will be served
	address->sin_port = htons(atoi(argv[1]));

	// BIND
	if (
        bind(
            *sock_p, 
            (struct sockaddr *)address,
            sizeof(*address)
        ) 
        < 0
       ) {
	    perror("bind failed");
		exit(EXIT_FAILURE);
	}

    if (type == SOCK_STREAM){
        // Only TCP needs Listening to the port
        printf("Server is listening on port %s\n", argv[1]);
        if (listen(*sock_p, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        // TCP also needs to set the signal behaving
        // Ctrl+C sends SIGINT to this program, 
        // and calls sigint_handler to end the server correctly and safely
        if (signal(SIGINT, sigint_handler) == SIG_ERR)
	        perror("signal error");  
    }
}

static inline int TCP_accept_with_server_fd(
                        int *sock_p, 
                        struct sockaddr_in *address
                    ){
    
    // return the server_file_descriptor

    int ret_fd, addrlen = sizeof(*address);

    if (
        (
            ret_fd
              = 
            accept(
                *sock_p, 
                (struct sockaddr *)address, 
                (socklen_t *)&addrlen
            )
        ) 
        < 0
       ){
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    return ret_fd;
}

static inline void TCP_server_handle_names(
                        int server_fd,
                        struct sockaddr_in *address,
                        char *client_whole_name,
                        char *recv_buff
                    ){

	// successfully connected to a client, now get the info of the client side
    // first get the ip and port, in order to form the name
	sprintf(
            client_whole_name, 
            "%s:%d", 
            inet_ntoa(address->sin_addr), 
            ntohs(address->sin_port)
        );
	printf("Client connected from %s\n", client_whole_name);

	// then build the file name by firstly strcpy
	strcpy(recv_buff, client_whole_name);
	int offset = strlen(recv_buff);
	recv_buff[offset++] = '_'; // split sign
	char extr;
    
    // recv the file type
	read(server_fd, &extr, 1);

	// printf("%d\n", extr);

	if (extr == 0)
		strcat(recv_buff, "recv");
	else if (extr == 1)
		strcat(recv_buff, "recv.txt");
	else if (extr == 2)
		strcat(recv_buff, "recv.mp4");
	else if (extr == 3)
		strcat(recv_buff, "recv.jpeg");
	else if (extr == 4)
		strcat(recv_buff, "recv.html");

// 0 -> none suffix
// 1 -> .txt
// 2 -> .mp4
// 3 -> .jpeg
// 4 -> .html
}

void *TCP_server_handling(void *params){
    // define the variables
    char client_whole_name[45] = {0}, recv_buff[MAX_BYTES + 25] = {0};
	FILE *local_file_p;
	struct pthread_params *ptr = (struct pthread_params *)params;
	int server_fd = ptr->sfd, valread = 0, package_cnt = 0;

    // first handle the file name and server name
    TCP_server_handle_names(
        server_fd, 
        &(ptr->address),
        client_whole_name, 
        recv_buff
    );
    // then open the local file in order to write into it
	local_file_p = fopen(recv_buff, "wb");

    // recv msg
	while(valread = read(server_fd, recv_buff, MAX_BYTES)){
		// default blocking mode with function "read"
		package_cnt ++;
		fwrite(recv_buff, 1, valread, local_file_p);
		if (package_cnt % 1000 == 1)
			printf("Packet %d received from client %s\n", package_cnt, client_whole_name);
	}

	// file end, closing the connected socket, and flush both recv content and logging into files
    // Also free the space of the params variable
	close(server_fd);
	fclose(local_file_p);
    free(params);
	printf("Client %s disconnected\n", client_whole_name);
	fflush(stdout); // flush the stdout to the logging file
}

static inline void UDP_server_handling(
                        int *sock_p, 
                        struct sockaddr_in *address
                    ){

    // define variables
	char recv_buffer[MAX_BYTES + 10];
    int addrlen = sizeof(*address), valread;
    // open local file in order to write to 
	FILE *local_file_p = fopen("received_video.mp4", "wb");

    // Waiting for the client sending message
	while (1){
		valread = recvfrom(
                    *sock_p, 
                    recv_buffer, 
                    MAX_BYTES + 2, 
                    0,
        	        (struct sockaddr *)address, 
                    (socklen_t*)&addrlen
                );
        // printf("valread = %d\n", valread);
        // if recv an empty message, means the end
		if (valread <= 2) break; // We assume that previous empty package 
                                 // won't influence present serving
		int seq_num = 
            (
                (recv_buffer[0] & MY_MASK) 
                << 8
            ) 
              | 
            (
                recv_buffer[1] & MY_MASK
            );
		printf("Received sequence number %d\n", seq_num);
		fwrite(recv_buffer + 2, 1, valread - 2, local_file_p);
	}
	// closing the listening socket
	fclose(local_file_p);
	printf("Video received and saved as 'received_video.mp4'\n");
	fflush(stdout); // flush to the logging file
}