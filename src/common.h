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

#define PERSISTENT 0 // define this to control the web server behaving

char Body_404[] = "<html><head><title>404 Not found</title></head><body><h1>404 Not Found</h1></body></html>";
char Body_400[] = "<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1></body></html>";
char Body_505[] = "<html><head><title>505 HTTP Version Not Supported</title></head><body><h1>505 HTTP Version Not Supported</h1></body></html>";

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
                        int type, // 1-> normal TCP; 2-> HTTP
                        int *server_fd_p,
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
    if (type == 1)
    	printf("Client connected from %s\n", client_whole_name);
    else{
        if (type == 2)
            printf("Client %s connected\n", client_whole_name);
        // only Normal TCP needs the next steps
        return;
    }
    
    if (server_fd_p == NULL || recv_buff == NULL){
        printf("CALL TCP_server_handle_names incorrectly\n");
        fflush(stdout);
        exit(5);
    }

	// then build the file name by firstly strcpy
	strcpy(recv_buff, client_whole_name);
	int offset = strlen(recv_buff);
	recv_buff[offset++] = '_'; // split sign
	char extr;
    
    // recv the file type
	read(*server_fd_p, &extr, 1);

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

    // first handle the file name and client name
    TCP_server_handle_names(
        1,
        &server_fd, 
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
			printf(
                "Packet %d received from client %s\n", 
                package_cnt, 
                client_whole_name
            );
	}

	// file end, closing the connected socket, and flush both recv content and logging into files
	close(server_fd);
	fclose(local_file_p);
    free(params); // Also free the space of the params variable
	printf("Client %s disconnected\n", client_whole_name);
	fflush(stdout); // flush the stdout to the logging file

    return NULL;
}

static inline void handle_each_HTTP_request(
                        int *server_fd_p,
                        char *client_whole_name
                    ){
    
    // define variables
    char recv_buff[MAX_BYTES + 25] = {0};
    char send_buff[MAX_BYTES + 25] = {0};
    char *path, *method, *protocol, *host_name_with_port, *host_name;
    FILE *local_file_p;
    int valread, msg_length;

    // receive an HTTP request, in this lab the GET request won't exceed 1024B
    valread = read(*server_fd_p, recv_buff, MAX_BYTES);
    printf("message-from-client: %s\n", client_whole_name);
    recv_buff[valread] = '\0';
    // printf("READ\n%s", recv_buff);
    
    // Parse the first line
    path                 = (char *) malloc(1005);
    method               = strtok(recv_buff,  " \t");
    sprintf(path, "www%s", strtok(NULL, " \t"));
    protocol             = strtok(NULL, " \t\r\n"); 
    // Parse Second Line to get host name (just to make sure it's a normal URL_or_IP address)
    strtok(NULL, " \t"); // Host: 
    host_name_with_port  = strtok(NULL, " \t\r\n");
    host_name            = strtok(host_name_with_port, ":");

    // show the first line to stdout
    printf("%s %s %s\n", method, path + 3, protocol);
    // prepare response to client
    printf("message-to-client: %s\n", client_whole_name);


    // clear the send buffer 
    memset(send_buff, 0, MAX_BYTES + 20);


    // confirm whether 400 happens
    if (
        strcmp(method, "GET") != 0 
          || 
        (
            host_name != NULL
              &&
            gethostbyname(host_name) == NULL
        )
    ){
        // 400 Bad Request
        printf("HTTP/1.1 400 Bad Request\n");
        sprintf(
            send_buff, 
            "HTTP/1.1 \t400 \tBad Request \t\r\n\r\n%s", 
            Body_400
        );
        send(*server_fd_p, send_buff, MAX_BYTES, 0);
    }
    // confirm whether 505 happens
    else if (strcmp(protocol, "HTTP/1.1") != 0){
        // 505 HTTP Version Not Supported
        printf("HTTP/1.1 505 HTTP Version Not Supported\n");
        sprintf(
            send_buff, 
            "HTTP/1.1 \t505 \tHTTP Version Not Supported \t\r\n\r\n%s", 
            Body_505
        );
        send(*server_fd_p, send_buff, MAX_BYTES, 0);
    }
    // confirm whether 404 happens
    else if (access(path, F_OK) != 0){
        // 404 Not Found
        printf("HTTP/1.1 404 Not found\n");
        sprintf(
            send_buff, 
            "HTTP/1.1 \t404 \tNot found \t\r\n\r\n%s", 
            Body_404
        );
        send(*server_fd_p, send_buff, MAX_BYTES, 0);
    }
    else{
        // ALL RIGHT, 200 OK
        printf("HTTP/1.1 200 OK\n");
        sprintf(
            send_buff, 
            "HTTP/1.1 \t200 \tOK \t\r\n\r\n" // string length 23
        );
        local_file_p = fopen(path, "rb");
        msg_length = fread(send_buff + 23, 1, MAX_BYTES - 23, local_file_p);
        // combine the first line and the first part of content
        send(*server_fd_p, send_buff, 23 + msg_length, 0);

        while(msg_length = fread(send_buff, 1, MAX_BYTES, local_file_p))
            send(*server_fd_p, send_buff, msg_length, 0);
    }


    // free the space
    free(path);
}   

void *Web_TCP_server_handling(void *params){
    // define the variables
    char client_whole_name[45] = {0};
	struct pthread_params *ptr = (struct pthread_params *)params;
	int server_fd = ptr->sfd;

    // first handle the client name
    TCP_server_handle_names(
        2,
        NULL, 
        &(ptr->address),
        client_whole_name, 
        NULL
    );

    if (!PERSISTENT){
        // only handle one request, and then end the channel
        handle_each_HTTP_request(&server_fd, client_whole_name);
    }

	// END PART
	close(server_fd);
    free(params); // Also free the space of the params variable
	printf("Client %s disconnected\n", client_whole_name);
	fflush(stdout); // flush the stdout to the logging file

    return NULL;
}