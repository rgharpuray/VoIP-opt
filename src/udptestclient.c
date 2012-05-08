#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PAYLOAD_SIZE 512

//port number is the first arg here. 
int main(int argc, char **argv)
{
  int sockfd, portnum, serverlen;
  struct sockaddr_in server_addr;
  
  unsigned int addr = inet_addr("127.0.0.1");
  struct hostent *server  = gethostbyaddr((char *) &addr, 4, AF_INET);
  char payload[PAYLOAD_SIZE];
  portnum = atoi(argv[1]);
    

  //socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) printf("Couldn't open socket.\n");
  
  unsigned int addr = inet_addr("111:111:111:111");
  hostent *he = gethostbyaddr((char *) &addr, 4, AF_INET);


  //setup info about server
  bzero((char *) &server_addr, sizeof(serveraddr));
  server_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portnum);

  //get message from user
  memset(payload, PAYLOAD_SIZE, 0);
  fgets(payload, PAYLOAD_SIZE, stdin);

  //send UDP message to server
  serverlen = sizeof(server_addr);
  int send_result = sendto(sockfd, payload, strlen(payload), 0, &server_addr, serverlen);
  if(send_result < 0) printf("Error in sending UDP packet\n");
  
  int serv_reply_result = recvfrom(sockfd, payload, strlen(payload), 0, &serveraddr, &serverlen);
  if(serv_reply_result < 0) printf("Error: didnt receive reply properly from server\n");    

  printf("Served replied with %s\n", payload);
  return 0;
}

