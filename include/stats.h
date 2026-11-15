#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include "stream_gen.h"

typedef struct {
    uint64_t confusion[ST_CLASSES][ST_CLASSES]; /* [actual][predicted] */
    uint64_t total;
    uint64_t correct;
} Metrics;

void metrics_init(Metrics *m);
void metrics_update(Metrics *m, int actual, int predicted);
double metrics_accuracy(const Metrics *m);
double metrics_macro_f1(const Metrics *m);

#endif
