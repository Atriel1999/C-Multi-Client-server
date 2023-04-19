#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAXLINE 1024
#define PORTNUM 3600
#define FDCNT 5 //socket count = 5 (Maximum connection)

typedef struct{
    int fd;
    char ip[20];
    int port;
    int msgcnt;
    int byte;
} Client;

void * thread_func(Client *data);
void sigint_handler(int signo);

int i=0;
int no=0;
int clientCount =0;
int sockfd_connect[FDCNT];
Client client_data[FDCNT] ={0};


int main(int argc, char **argv)
{
	signal(SIGALRM, sigint_handler);
	alarm(5);

	int listen_fd, client_fd;
    int client_addr_size;
	socklen_t addrlen;
	int readn;
	char buf[MAXLINE];

    struct sockaddr_in server_addr, client_addr;
	pthread_t thread_id[FDCNT]={0};

	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
        printf("Failed to connect Socket!\n");
        return 1; }

	memset((void *)&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORTNUM);

 	if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1){
		perror("bind error"); return 1; }

	if(listen(listen_fd, 5) == -1){perror("listen error"); return 1; }
   
    addrlen = sizeof(client_addr);


	while(1){
		client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen);

        if(clientCount == FDCNT){ //Check Server is Full
            printf("The maximum number of client sessions is reached\n");
			printf("---------------------------------------------------------------------------\n");
            close(client_fd);
            continue;
        }

		if(client_fd < 0){ printf("accept error\n"); } //Looking for error

		//error?
        for(i=0; i<FDCNT; i++){
            if(client_data[i].fd == 0){
                client_data[i].fd = client_fd;

                break;
            }
        }

        strcpy(client_data[i].ip , inet_ntoa(client_addr.sin_addr));
        client_data[i].port = ntohs(client_addr.sin_port);

		pthread_create(thread_id+i, NULL, (void *)thread_func, client_data+i);
		
		printf("Accepted socket: %d\n",++clientCount);
	}

    close(listen_fd);
	return 0;
}


//---------------------------------------------------------------------
void sigint_handler(int signo){
	int totalmsg = 0;
	int totalbytes = 0;

	no++;


	for(int j=0; j <= i;j++){
		totalmsg += client_data[j].msgcnt;
		totalbytes += client_data[j].byte;	
	}
	
	
	printf("no%d\tIP address\t\tPort\tcount\tbytes\t\n",no);
	printf("---------------------------------------------------------------------------\n");
	
	for(int j=0; j<=i;j++){

		printf("[%d]\t%s\t\t(%d)\t%d/%d\t%d/%d\n",j+1,client_data[j].ip,client_data[j].port,client_data[j].msgcnt,totalmsg,client_data[j].byte,totalbytes);
	}

	printf("---------------------------------------------------------------------------\n");
	printf("[*]\t3-clients\t\t\t%d/%d\t%d/%d\t\n",totalmsg,totalmsg,totalbytes,totalbytes);
	

	alarm(5);
}

 


//---------------------------------------------------------------------
void * thread_func(Client *data)
{
	int rst;
	char msg[100];
	char * reply = "data receive\n";

	while(1){
		rst = read(data->fd,msg,sizeof(msg));
		if(rst<=0)
			break;
		else{
			data->msgcnt += 1;
			data->byte += sizeof(msg);
		}
	}
	return 0;

	close(data->fd);
	clientCount--;
	data->fd=0;
	printf("Client Disconnect (%s)\n",data->ip);
	return NULL;
}

