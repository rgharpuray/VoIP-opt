#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PAYLOAD_SIZE 512

int main(int argc, char **argv)
{
  int sockfd, portnum, clientlen;
  struct sockaddr_in serv_addr, client_addr;
  struct hostent *host_info;
  char payload[PAYLOAD_SIZE];
  char *host_addr_ip;
  int msg_size;

  //get port num as an arg
  portnum = atoi(argv[1]);

  //socket setup
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) printf("Couldn't open socket.\n");
  
  //setup server's inet address info
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons((unsigned short)portnum);

  //bind the parent socket to the port
  int bind_result = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if(bind_result < 0) printf("Bind error.\n");

  clientlen = sizeof(client_addr);
  //main waiting loop for udp datagrams
  while(1) 
  {
    bzero(payload, PAYLOAD_SIZE);
    msg_size = recvfrom(sockfd, payload, PAYLOAD_SIZE, 0, (struct sockaddr *) &client_addr, &clientlen);   
    if(msg_size < 0) printf("Didn't receive correctly");
  
    host_addr_ip = gethostbyaddr((const char *)&client_addr.sin_addr.s_addr, 
			  sizeof(client_addr.sin_addr.s_addr), AF_INET);
    
    printf("server received %d/%d bytes reading: %s\n", strlen(payload), msg_size, payload);

    //send acknoweldgement (echo) input back to the sending client
    int serv_reply_send_result = sendto(sockfd, payload, strlen(payload), 0, 
					(struct sockaddr *) &client_addr, clientlen);
    if(serv_reply_send_result == 0)
    {
      printf("Couldnt echo result back to client\n");
    }
  }  
  
  //we're done
  return 0;
}
