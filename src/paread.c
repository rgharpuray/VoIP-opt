
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portaudio.h>

#define SAMPLE_RATE (8000)
#define FRAMES_PER_BUFFER (512)
#define PA_SAMPLE_TYPE  paFloat32

typedef float SAMPLE;
typedef unsigned char QSAMPLE;

int main(int argc, char* argv[])
{  
  PaStream* stream;
  PaError err;
  
  err = Pa_Initialize();
  if( err != paNoError) goto error;
  
  err = Pa_OpenDefaultStream(
    &stream,
    1, //input channels
    0, //output channels
    PA_SAMPLE_TYPE,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    NULL,
    NULL);
  if(err != paNoError) goto error;
  
  err = Pa_StartStream(stream);
  if(err != paNoError) goto error;
  
  QSAMPLE* qbuf = malloc(sizeof(QSAMPLE)*FRAMES_PER_BUFFER);
  SAMPLE* sbuf = malloc(sizeof(SAMPLE)*FRAMES_PER_BUFFER);
  
  while(1) {
    err = Pa_ReadStream(stream,(void*)sbuf,FRAMES_PER_BUFFER);
    if(err != paNoError) goto error;
    /*
    for(int i = 0; i < FRAMES_PER_BUFFER; i++) {
      qbuf[i] = (signed char)(127.0f*sbuf[i]);
    }
    */
    int nwrite = fwrite((void*)sbuf,sizeof(SAMPLE),FRAMES_PER_BUFFER,stdout);
    if(nwrite != FRAMES_PER_BUFFER) {
      err = Pa_StopStream(stream);
      if(err != paNoError) goto error;
      free(qbuf);
      free(sbuf);
      Pa_Terminate();
      return 0;
    }
    fflush(stdout);
  }
error:
  if(stream) {
    Pa_AbortStream(stream);
    Pa_CloseStream(stream);
  }
  free(qbuf);
  free(sbuf);
  Pa_Terminate();
  fprintf(stderr,"An error occured while using the portaudio stream\n");
  fprintf(stderr,"Error number: %d\n", err);
  fprintf(stderr,"Error message: %s\n", Pa_GetErrorText(err));
  return -1;
}