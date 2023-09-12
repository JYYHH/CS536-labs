#include "common.h"
#include <pthread.h>
#define NUM_TOTAL_THREAD 200 // define the max number of threads to receive client request

int present_thread = 0;
struct pthread_params{
	int sock;
	struct sockaddr_in address;
};

void *serving(void *params){
	char client_whole_name[45];
	FILE *local_file_p;
	struct pthread_params *ptr = (struct pthread_params *)params;
	int server_fd = ptr->sock;
	struct sockaddr_in address = ptr->address;
	int valread;

	// successfully connected to a client
	sprintf(client_whole_name, "%s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	printf("Client connected from %s\n", client_whole_name);

	char recv_buff[MAX_BYTES + 25] = {0};
	char *name;

	// first recv the file name
	strcpy(recv_buff, client_whole_name);
	int offset = strlen(recv_buff);
	recv_buff[offset++] = '_';
	char extr;
	valread = read(server_fd, &extr, 1);

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

	local_file_p = fopen(recv_buff, "wb");
	
	int package_cnt = 0;

	while(valread = read(server_fd, recv_buff, MAX_BYTES)){
		// default blocking mode with function "read"
		package_cnt ++;
		fwrite(recv_buff, 1, valread, local_file_p);
		if (package_cnt % 1000 == 1)
			printf("Packet %d received from client %s\n", package_cnt, client_whole_name);
	}

	// file end, closing the connected socket, and flush both recv content and logging into files
	close(server_fd);
	fclose(local_file_p);
	printf("Client %s disconnected\n", client_whole_name);
	fflush(stdout); // flush to the logging file
}

int main(int argc, char const* argv[]){
    // check the param's lengths
	server_arg_check(argc, argv);
	
	// define variables
	int sock, server_fd, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// Creating TCP socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // any incoming client will be served
	address.sin_port = htons(atoi(argv[1]));

	// BIND: Forcefully attaching socket to the port
	if (bind(sock, (struct sockaddr*)&address,sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
    // Listening
	printf("Server is listening on port %s\n", argv[1]);
	if (listen(sock, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	// Waiting From the client
	pthread_t thread[NUM_TOTAL_THREAD]; // define threads DS
	while (1){
		if ((server_fd = accept(sock, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
			perror("accept");
			exit(EXIT_FAILURE);
		}
		else{
			struct pthread_params *ptr = (struct pthread_params *)malloc(sizeof(struct pthread_params));
			ptr->sock = server_fd;
			ptr->address = address;
			pthread_create(&thread[present_thread ++], NULL, serving, (void *)ptr);
		}
	}
	// closing the listening socket
	for(int i=0;i<present_thread;i++)
		pthread_join(thread[i], NULL);
	shutdown(sock, SHUT_RDWR);
	return 0;
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