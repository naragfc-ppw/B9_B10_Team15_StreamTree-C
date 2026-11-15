#include "drift.h"
#include <math.h>
#include <string.h>

void ddm_init(DDMDetector *detector, uint64_t min_instances) {
    if (detector == NULL) return;
    memset(detector, 0, sizeof(*detector));
    detector->min_instances = min_instances < 2 ? 30 : min_instances;
}

DriftStatus ddm_update(DDMDetector *detector, int prediction_error) {
    if (detector == NULL) return DRIFT_NONE;
    if (prediction_error != 0 && prediction_error != 1) return DRIFT_NONE;

    detector->n++;
    detector->error_sum += (double)prediction_error;
    if (detector->n < detector->min_instances) return DRIFT_NONE;

    double p = detector->error_sum / (double)detector->n;
    double s = sqrt(p * (1.0 - p) / (double)detector->n);

    if (!detector->has_minimum || p + s < detector->p_min + detector->s_min) {
        detector->p_min = p;
        detector->s_min = s;
        detector->has_minimum = 1;
        return DRIFT_NONE;
    }

    if (p + s > detector->p_min + 3.0 * detector->s_min)
        return DRIFT_CONFIRMED;
    if (p + s > detector->p_min + 2.0 * detector->s_min)
        return DRIFT_WARNING;
    return DRIFT_NONE;
}
