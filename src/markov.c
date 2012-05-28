
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "markov.h"

static unsigned int nstates;
static float* drop_rates;
static float* transition_rates;

static int state;

const float pi = 3.1415926f;

float randf()
{
  return ((float)rand())/(float)RAND_MAX; 
}

float randgaussian()
{
  return sqrt(-2*logf(randf()))*cosf(2*pi*randf());
}

/*functions for H.M.M.*/
void markov_transition()
{
  //get random float between 0 and 1 inclusive to update state  
  float random = randf();
  float* ptr = &(transition_rates[nstates*state]);
  for(int i = 0; i < nstates; i++) {
    random -= ptr[i];
    if(random <= 0.0f) {
      state = i;
      return;
    }
  }
  assert(0);
}

/*this function decides whether to drop or accept a packet based on loss rates
associated with each state*/
mmvalue markov_emission()
{
  //we should have transitioned to high loss or low loss state already
  assert((state >= 0)&&(state < nstates));
  float random = randf();
  return (random < drop_rates[state]) ? MM_DROP : MM_ACCEPT;
}

void markov_initmodel_lowhigh(float low_lr, float high_lr, float low_meanlength, float high_meanlength)
{
  //initialize the model
  nstates = 2;
  drop_rates = malloc(2*sizeof(float));
  drop_rates[0] = low_lr;
  drop_rates[1] = high_lr;
  transition_rates = malloc(4*sizeof(float));
  transition_rates[0] = 1.0f - 1.0f/low_meanlength;
  transition_rates[1] = 1.0f/low_meanlength;
  transition_rates[2] = 1.0f/high_meanlength;
  transition_rates[3] = 1.0f - 1.0f/high_meanlength;
  //initialize the state information to the low loss state
  state = 0; 
}

void markov_initmodel_fromfile(FILE* f)
{
  int i,j;
  //seed the random generator
  srand(time(NULL));
  //start reading the file
  if(fscanf(f,"%u",&nstates)!=1) {
    fprintf(stderr,"Error: In reading markov model from file, couldn't read number of states.\n");
    exit(1);    
  }
  if(nstates > 256) {
    fprintf(stderr,"Error: In reading markov model from file, number of states too large (%u).\n",nstates);
    exit(1);
  }
  drop_rates = malloc(nstates*sizeof(float));
  transition_rates = malloc(nstates*nstates*sizeof(float));
  for(i = 0; i < nstates; i++) {
    if(fscanf(f,"%f",&(drop_rates[i]))!=1) {
      fprintf(stderr,"Error: In reading markov model from file, couldn't read drop rate.\n");
      exit(1);
    }
    if((drop_rates[i] < 0.0f)||(drop_rates[i] > 1.0f)) {
      fprintf(stderr,"Error: In reading markov model from file, invalid drop rate (%f).\n",drop_rates[i]);
      exit(1);
    }
  }
  for(i = 0; i < nstates*nstates; i++) {
    if(fscanf(f,"%f",&(transition_rates[i]))!=1) {
      fprintf(stderr,"Error: In reading markov model from file, couldn't read transition rate.\n");
      exit(1);
    }    
    if((transition_rates[i] < 0.0f)||(transition_rates[i] > 1.0f)) {
      fprintf(stderr,"Error: In reading markov model from file, invalid transition rate (%f).\n",transition_rates[i]);
      exit(1);
    }
  }
  for(i = 0; i < nstates; i++) {
    float acc = 0.0f;
    for(j = 0; j < nstates; j++) {
      acc += transition_rates[i*nstates+j];
    }
    if((acc < 0.999f)||(acc > 1.001f)) {
      fprintf(stderr,"Error: In reading markov model from file, expected transition rates to sum to 1.  Instead, they sum to (%f).\n",acc);
      exit(1);
    }
  }
}
