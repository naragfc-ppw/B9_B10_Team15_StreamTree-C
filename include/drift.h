#ifndef DRIFT_H
#define DRIFT_H

#include <stdint.h>

typedef enum {
    DRIFT_NONE = 0,
    DRIFT_WARNING = 1,
    DRIFT_CONFIRMED = 2
} DriftStatus;

typedef struct {
    uint64_t min_instances;
    uint64_t n;
    double error_sum;
    double p_min;
    double s_min;
    int has_minimum;
} DDMDetector;

void ddm_init(DDMDetector *detector, uint64_t min_instances);
DriftStatus ddm_update(DDMDetector *detector, int prediction_error);

#endif
