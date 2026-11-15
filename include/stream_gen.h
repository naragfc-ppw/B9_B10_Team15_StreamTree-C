#ifndef STREAM_GEN_H
#define STREAM_GEN_H

#include <stdint.h>

#define ST_FEATURES 3
#define ST_CLASSES 2

typedef struct {
    double x[ST_FEATURES];
    int label;
    uint64_t index;
    int concept_id;
} Sample;

typedef struct {
    uint64_t state;
    double noise_rate;
    unsigned int drift_at;
    int concept_before;
    int concept_after;
} StreamGenerator;

void stream_init(StreamGenerator *gen, uint64_t seed, double noise_rate,
                 unsigned int drift_at, int concept_before, int concept_after);
Sample stream_next(StreamGenerator *gen, uint64_t index);

#endif
