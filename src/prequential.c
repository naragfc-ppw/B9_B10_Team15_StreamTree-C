#include "prequential.h"
#include <stdio.h>

int prequential_run(StreamGenerator *gen, uint64_t samples, HoeffdingTree *tree,
                    Metrics *metrics, DDMDetector *detector, const char *csv_path,
                    uint64_t *warning_count, uint64_t *drift_count,
                    uint64_t *first_drift_sample) {
    if (gen == NULL || tree == NULL || metrics == NULL || detector == NULL ||
        csv_path == NULL || warning_count == NULL || drift_count == NULL ||
        first_drift_sample == NULL) return 0;

    *warning_count = 0;
    *drift_count = 0;
    *first_drift_sample = UINT64_MAX;

    FILE *fp = fopen(csv_path, "w");
    if (fp == NULL) return 0;
    fprintf(fp, "sample,x1,x2,x3,actual,predicted,correct,concept_id,nodes,ddm_status\n");

    for (uint64_t i = 0; i < samples; ++i) {
        Sample s = stream_next(gen, i);
        int prediction = ht_predict(tree, s.x); /* prequential: test first */
        int correct = prediction == s.label;
        metrics_update(metrics, s.label, prediction);

        DriftStatus status = ddm_update(detector, correct ? 0 : 1);
        if (status == DRIFT_WARNING) (*warning_count)++;
        if (status == DRIFT_CONFIRMED) {
            (*drift_count)++;
            if (*first_drift_sample == UINT64_MAX) *first_drift_sample = i;
            /* Reset before training this sample, so it becomes the first
               observation learned by the newly initialized tree. */
            if (!ht_reset(tree)) {
                fclose(fp);
                return 0;
            }
            ddm_init(detector, detector->min_instances);
        }

        ht_train(tree, s.x, s.label); /* train second */
        const char *status_text = status == DRIFT_WARNING ? "WARNING" :
                                  status == DRIFT_CONFIRMED ? "DRIFT" : "NONE";
        fprintf(fp, "%llu,%.6f,%.6f,%.6f,%d,%d,%d,%d,%zu,%s\n",
                (unsigned long long)i, s.x[0], s.x[1], s.x[2], s.label,
                prediction, correct, s.concept_id, tree->node_count, status_text);
    }
    return fclose(fp) == 0;
}
