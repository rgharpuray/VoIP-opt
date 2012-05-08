
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <portaudio.h>

#define SAMPLE_RATE         (8000)
#define PA_SAMPLE_TYPE       paFloat32
#define FRAMES_PER_BUFFER   (512)

typedef float SAMPLE;

static int voipCallback(
  const void* inputBuffer,
  void* outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData);

typedef struct {
  SAMPLE* poutdata;
  int poutvalid;
  SAMPLE* pindata;
  int pinvalid;
  int stop;
} VoipData;

static int voipCallback(
  const void* inputBuffer,
  void* outputBuffer,
  unsigned long framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void* userData)
{
  VoipData* data = (VoipData*)userData;
  const SAMPLE* rptr = (SAMPLE*)inputBuffer;
  SAMPLE* wptr = (SAMPLE*)outputBuffer;
  /*
  if(framesPerBuffer != FRAMES_PER_BUFFER) {
    fprintf(stderr,"Error: Unexpected frame buffer size %u.\n", (unsigned int)framesPerBuffer);
    return paAbort;
  }
  */
  if(data->poutvalid) {
    for(int i = 0; i < FRAMES_PER_BUFFER; i++) {
      wptr[i] = data->poutdata[i];
    }
    data->poutvalid = 0;
  }
  else {
    for(int i = 0; i < FRAMES_PER_BUFFER; i++) {
      wptr[i] = 0.0f;
    }
  }
  if(data->pinvalid) {
    for(int i = 0; i < FRAMES_PER_BUFFER; i++) {
      data->pindata[i] = rptr[i];
    }
    data->pinvalid = 0;
  }
  return data->stop ? paComplete : paContinue;
}

int main(int argc, char* argv[])
{
  PaStreamParameters inputParams, outputParams;
  PaStream* stream;
  PaError err;
  
  VoipData data;
  data.poutdata = malloc(sizeof(SAMPLE)*FRAMES_PER_BUFFER);
  data.pindata = malloc(sizeof(SAMPLE)*FRAMES_PER_BUFFER);
  data.poutvalid = 0;
  data.pinvalid = 1;
  
  //initialize PortAudio
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
    NULL);
  if(err != paNoError) goto error;
  
  err = Pa_StartStream(stream);
  if(err != paNoError) goto error;
  
  while(1);
  
  printf("Hit ENTER to stop program.\n");
  getchar();
  err = Pa_CloseStream(stream);
  if(err != paNoError) goto error;
  
  printf("Finished.\n");
  Pa_Terminate();
  return 0;
  
error:
  Pa_Terminate();
  fprintf( stderr, "An error occured while using the portaudio stream\n" );
  fprintf( stderr, "Error number: %d\n", err );
  fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  return -1;
}

