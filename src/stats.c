#include "stats.h"
#include <stddef.h>

void metrics_init(Metrics *m) {
    if (m == NULL) return;
    for (int a = 0; a < ST_CLASSES; ++a)
        for (int p = 0; p < ST_CLASSES; ++p)
            m->confusion[a][p] = 0;
    m->total = 0;
    m->correct = 0;
}

void metrics_update(Metrics *m, int actual, int predicted) {
    if (m == NULL || actual < 0 || actual >= ST_CLASSES ||
        predicted < 0 || predicted >= ST_CLASSES) return;
    m->confusion[actual][predicted]++;
    m->total++;
    if (actual == predicted) m->correct++;
}

double metrics_accuracy(const Metrics *m) {
    if (m == NULL || m->total == 0) return 0.0;
    return (double)m->correct / (double)m->total;
}

double metrics_macro_f1(const Metrics *m) {
    if (m == NULL) return 0.0;
    double sum = 0.0;
    for (int c = 0; c < ST_CLASSES; ++c) {
        uint64_t tp = m->confusion[c][c];
        uint64_t fp = 0, fn = 0;
        for (int k = 0; k < ST_CLASSES; ++k) {
            if (k != c) {
                fp += m->confusion[k][c];
                fn += m->confusion[c][k];
            }
        }
        double denom = (double)(2 * tp + fp + fn);
        sum += denom == 0.0 ? 0.0 : (2.0 * (double)tp) / denom;
    }
    return sum / (double)ST_CLASSES;
}
