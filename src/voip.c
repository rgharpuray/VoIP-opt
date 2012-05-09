
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
typedef float QSAMPLE;

inline QSAMPLE sample_to_qsample(SAMPLE x);
inline SAMPLE qsample_to_sample(QSAMPLE x);

QSAMPLE sample_to_qsample(SAMPLE x) {
  return x;
}

SAMPLE qsample_to_sample(QSAMPLE x) {
  return x;
}

static int voipCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData );

typedef struct {
  QSAMPLE* inbuf;
  volatile int inbuf_valid;
  QSAMPLE* outbuf;
  volatile int outbuf_valid;
} VoipData;

static int voipCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
  VoipData* data = (VoipData*)userData;
  
  if((inputBuffer != NULL)||(data->inbuf_valid == 0)) {
    const SAMPLE* rptr = (const SAMPLE*)inputBuffer;
    for(int i = 0; i < framesPerBuffer; i++) {
      data->inbuf[i] = sample_to_qsample(rptr[i]);
    }
    data->inbuf_valid = 1;
  }
  if(data->outbuf_valid == 1) {
    SAMPLE* wptr = (SAMPLE*)outputBuffer;
    for(int i = 0; i < framesPerBuffer; i++) {
      wptr[i] = qsample_to_sample(data->outbuf[i]);
    }
    data->outbuf_valid = 0;
  }
  return paContinue;
}

typedef struct {
  int is_server;
  in_addr_t ip_addr;
  uint16_t ip_port;
} VoipArgs;

typedef struct {
  int sockfd;
  struct sockaddr_in saddr;
  socklen_t saddrlen;
} ConnectionData;

//returns 0 for success, 1 for failure
int parse_args(VoipArgs* pva, int argc, char* argv[]);

//returns 0 for success, 1 for failure
int connection_init(ConnectionData* pcd, const VoipArgs* pva);
int connection_send(ConnectionData* pcd, const void* buf, size_t length);
int connection_recv(ConnectionData* pcd, void* buf, size_t length);

int main(int argc, char* argv[])
{
  //parse the arguments
  VoipArgs va;
  if(parse_args(&va,argc,argv) != 0) {
    return 1;
  }
  //initialize the connection  
  ConnectionData cd;
  if(connection_init(&cd,&va) != 0) {
    return 1;
  }
  
  //initialize VOIP data
  VoipData vd;
  vd.inbuf = malloc(FRAMES_PER_BUFFER*sizeof(QSAMPLE));
  vd.inbuf_valid = 0;
  vd.outbuf = malloc(FRAMES_PER_BUFFER*sizeof(QSAMPLE));
  vd.outbuf_valid = 0;
  
  fprintf(stderr,"Starting audio stream...\n");
  
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
    voipCallback,
    &vd);
  if(err != paNoError) goto error;
  
  err = Pa_StartStream(stream);
  if(err != paNoError) goto error;
  
  while(1) {
    //wait for input data
    while(vd.inbuf_valid == 0);
    //send it on the network
    if(connection_send(&cd,vd.inbuf,FRAMES_PER_BUFFER*sizeof(QSAMPLE))!=0) goto error;
    //signal that the input data has been used
    vd.inbuf_valid = 0;
    //wait for output data to be consumed
    while(vd.outbuf_valid == 1);
    //receive some from the network
    if(connection_recv(&cd,vd.outbuf,FRAMES_PER_BUFFER*sizeof(QSAMPLE))!=0) goto error;
    //signal that the output data is now valid
    vd.outbuf_valid = 1;
  }
  
error:
  if(stream) {
    Pa_AbortStream(stream);
    Pa_CloseStream(stream);
  }
  free(vd.inbuf);
  free(vd.outbuf);
  Pa_Terminate();
  if(shutdown(cd.sockfd,SHUT_RDWR)!=0) {
    fprintf(stderr,"Error: Socket shutdown failed.\n");
  }
  fprintf(stderr,"An error occured while using the portaudio stream\n");
  fprintf(stderr,"Error number: %d\n", err);
  fprintf(stderr,"Error message: %s\n", Pa_GetErrorText(err));
  return -1;
}

int connection_send(ConnectionData* pcd, const void* buf, size_t length)
{
  int result = sendto(pcd->sockfd, buf, length, 0, (struct sockaddr*) &pcd->saddr, pcd->saddrlen);
  if(result < 0) {
    fprintf(stderr,"Error: Send call failed.\n");
    return 1;
  }
  return 0;
}

int connection_recv(ConnectionData* pcd, void* buf, size_t length)
{
  int msglen = recvfrom(pcd->sockfd, buf, length, 0, (struct sockaddr*)&pcd->saddr, &pcd->saddrlen);   
  if(msglen <= 0) {
    fprintf(stderr,"Error: Recieve call failed.\n");
    return 1;
  }
  if(msglen != length) {
    fprintf(stderr,"Error: Recieved message has expected length.\n");
    return 1;
  }
  return 0;
}

int connection_init(ConnectionData* pcd, const VoipArgs* pva)
{
  uint8_t msgbuf[256];
  
  const uint8_t client_message[] = {57, 36, 104, 39, 11, 207, 192, 126}; 
  const uint8_t server_message[] = {23, 24, 62, 111, 204, 123, 3, 2};
  
  pcd->saddrlen = sizeof(pcd->saddr); 
  bzero(&pcd->saddr, sizeof(pcd->saddr));
  pcd->saddr.sin_family = AF_INET;
  pcd->saddr.sin_port = htons((uint16_t)pva->ip_port);
  
  pcd->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(pcd->sockfd < 0) {
    fprintf(stderr,"Error: Couldn't open socket.\n");
    return 1;
  }
  
  if(pva->is_server) {
    //set up and bind the socket
    pcd->saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(pcd->sockfd,(struct sockaddr*)&pcd->saddr,sizeof(pcd->saddr))<0) {
      fprintf(stderr,"Error: Couldn't bind socket.\n");
      return 1;
    }
    //read from the socket (wait for a "connection")
    fprintf(stderr,"Waiting for a connection on port %u...\n",pva->ip_port);
    if(connection_recv(pcd,msgbuf,sizeof(client_message))!=0) {
      return 1;
    }
    for(int i = 0; i < sizeof(client_message); i++) {
      if(msgbuf[i] != client_message[i]) {
        fprintf(stderr,"Error: Connection message has unexpected contents.\n");
        return 1;
      }
    }
    //respond with the server message
    fprintf(stderr,"Client found!  Responding...\n");
    if(connection_send(pcd,server_message,sizeof(server_message))!=0) {
      return 1;
    }
    //ready to start audio stream
  }
  else {
    pcd->saddr.sin_addr.s_addr = pva->ip_addr;
    //send client message
    fprintf(stderr,"Sending connection message on port %u...\n",pva->ip_port);
    if(connection_send(pcd,client_message,sizeof(client_message))!=0) {
      return 1;
    }
    //receive server message
    fprintf(stderr,"Waiting for server response...\n");
    if(connection_recv(pcd,msgbuf,sizeof(server_message))!=0) {
      return 1;
    }
    for(int i = 0; i < sizeof(server_message); i++) {
      if(msgbuf[i] != server_message[i]) {
        fprintf(stderr,"Error: Connection message has unexpected contents.\n");
        return 1;
      }
    }
    fprintf(stderr,"Connected!\n");
    //ready to start audio stream
  }
}

int parse_args(VoipArgs* pva, int argc, char* argv[])
{
  if(argc != 3) {
    fprintf(stderr,"Usage: %s [server|ip-address] [port]\n",argv[0]);
    return 1;
  }
  
  in_addr_t ip_addr; int is_server;
  if(strcmp(argv[1],"server")==0) {
    is_server = 1;
    ip_addr = (in_addr_t)0;
  }
  else {
    is_server = 0;
    ip_addr = inet_addr(argv[1]);
    if(ip_addr == (in_addr_t)(-1)) {
      fprintf(stderr,"Error: Invalid IP address \"%s\"\n",argv[1]);
      return 1;
    }
  }
    
  int iport = atoi(argv[2]);
  if((iport <= 0)||(iport >= 65536)) {
    fprintf(stderr,"Error: Invalid port \"%s\"\n",argv[2]);
    return 1;
  }
  
  pva->is_server = is_server;
  pva->ip_addr = ip_addr;
  pva->ip_port = (uint16_t)iport;
  return 0;
}
