#ifndef PREQUENTIAL_H
#define PREQUENTIAL_H

#include <stdint.h>
#include "stream_gen.h"
#include "hoeffding_tree.h"
#include "stats.h"
#include "drift.h"

int prequential_run(StreamGenerator *gen, uint64_t samples, HoeffdingTree *tree,
                    Metrics *metrics, DDMDetector *detector, const char *csv_path,
                    uint64_t *warning_count, uint64_t *drift_count,
                    uint64_t *first_drift_sample);

#endif
