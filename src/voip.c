
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SAMPLE_RATE (8000)
#define FRAMES_PER_BUFFER (512)
#define PA_SAMPLE_TYPE  paFloat32

typedef float SAMPLE;
typedef unsigned char QSAMPLE;

int main(int argc, char* argv[])
{
  if(argc != 3) {
    fprintf(stderr,"Usage: %s [ip-address] [port]\n",argv[0]);
    return 1;
  }
  
  in_addr_t ip_addr = inet_addr(argv[1]);
  if(ip_addr == (in_addr_t)(-1)) {
    fprintf(stderr,"Error: Invalid IP address \"%s\"\n",argv[1]);
    return 1;
  }
    
  int iport = atoi(argv[2]);
  if((iport <= 0)||(iport >= 65536)) {
    fprintf(stderr,"Error: Invalid port \"%s\"\n",argv[2]);
    return 1;
  }
  
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0) {
    fprintf(stderr,"Error: Couldn't open socket.\n");
    return 1;
  }
  
  struct sockaddr_in saddr;
  bzero(&saddr, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons((uint16_t)iport);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  if(bind(sockfd,(struct sockaddr*)&saddr,sizeof(saddr))<0) {
    fprintf(stderr,"Error: Couldn't bind socket.\n");
    return 1;
  }
  
  saddr.sin_addr.s_addr = ip_addr;
  
  const uint8_t init_message[] = {57, 36, 104, 39, 11, 207, 192, 126}; 
  if(sendto(sockfd,init_message,sizeof(init_message),0,(struct sockaddr*)&saddr,sizeof(saddr)) < 0) {
    fprintf(stderr,"Error: Send failed.\n");
    return 1;
  }
  
  struct sockaddr_in caddr;
  socklen_t caddr_len = sizeof(caddr);
  //buffer to store messages
  uint8_t* buf = malloc(4092);
  int len;
  len = recvfrom(sockfd,buf,4092,0,(struct sockaddr*)&caddr,&caddr_len);
  if(len <= 0) {
    fprintf(stderr,"Error: Recieve failed.\n");
    return 1;
  }
  if(len != sizeof(init_message)) {
    fprintf(stderr,"Error: Received message is not the expected length.\n");
    return 1;
  }
  for(int i = 0; i < sizeof(init_message); i++) {
    if(buf[i] != init_message[i]) {
      fprintf(stderr,"Error: Recieved message does not have the expected contents.\n");
      return 1;
    }
  }
  
  fprintf(stderr,"Notice: Recieved initialization message correctly.\n");
  return 0;
  
  PaStream* stream;
  PaError err;
  
  err = Pa_Initialize();
  if( err != paNoError) goto error;
  
  err = Pa_OpenDefaultStream(
    &stream,
    1, //input channels
    1, //output channels
    PA_SAMPLE_TYPE,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    NULL,
    NULL);
  if(err != paNoError) goto error;
  
  err = Pa_StartStream(stream);
  if(err != paNoError) goto error;
  
  SAMPLE* sbuf = malloc(sizeof(SAMPLE)*FRAMES_PER_BUFFER);
  
  while(1) {
    err = Pa_ReadStream(stream,(void*)sbuf,FRAMES_PER_BUFFER);
    if(err != paNoError) {
      //fprintf(stderr,"Warning: PortAudio input overflow.\n");
    }
    int nwrite = fwrite((void*)sbuf,sizeof(SAMPLE),FRAMES_PER_BUFFER,stdout);
    if(nwrite != FRAMES_PER_BUFFER) {
      err = Pa_StopStream(stream);
      if(err != paNoError) goto error;
      free(sbuf);
      Pa_Terminate();
      return 0;
    }
    fflush(stdout);
    int nread = fread((void*)sbuf,sizeof(SAMPLE),FRAMES_PER_BUFFER,stdin);
    if(nread != FRAMES_PER_BUFFER) {
      err = Pa_StopStream(stream);
      if(err != paNoError) goto error;
      free(sbuf);
      Pa_Terminate();
      return 0;
    }
    err = Pa_WriteStream(stream,sbuf,FRAMES_PER_BUFFER);
    if(err != paNoError) {
      //fprintf(stderr,"Warning: PortAudio output underflow.\n");
    }
  }
error:
  if(stream) {
    Pa_AbortStream(stream);
    Pa_CloseStream(stream);
  }
  free(sbuf);
  Pa_Terminate();
  fprintf(stderr,"An error occured while using the portaudio stream\n");
  fprintf(stderr,"Error number: %d\n", err);
  fprintf(stderr,"Error message: %s\n", Pa_GetErrorText(err));
  return -1;
}
