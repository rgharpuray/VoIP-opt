
typedef enum mmvalue {
  MM_ACCEPT,
  MM_DROP
} mmvalue;

void markov_initmodel_lowhigh(float low_lr, float high_lr, float low_meanlength, float high_meanlength);

void markov_initmodel_fromfile(FILE* f);

void markov_transition();
mmvalue markov_emission();

float randf();
float randgaussian();

