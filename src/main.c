#include "stream_gen.h"
#include "hoeffding_tree.h"
#include "stats.h"
#include "prequential.h"
#include "drift.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_SAMPLES 3000UL
#define DEFAULT_SEED 42UL
#define DEFAULT_DRIFT_AT 1500UL
#define DEFAULT_NOISE 0.05
#define DEFAULT_MAX_NODES 101UL
#define DEFAULT_GRACE 100UL

int main(int argc, char **argv) {
    unsigned long samples = DEFAULT_SAMPLES;
    unsigned long drift_at = DEFAULT_DRIFT_AT;
    unsigned long seed = DEFAULT_SEED;
    unsigned long max_nodes = DEFAULT_MAX_NODES;
    unsigned long grace = DEFAULT_GRACE;
    double noise = DEFAULT_NOISE;
    const char *output_path = "data/prequential_results.csv";

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--samples") == 0 && i + 1 < argc) samples = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--drift-at") == 0 && i + 1 < argc) drift_at = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) seed = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--noise") == 0 && i + 1 < argc) noise = strtod(argv[++i], NULL);
        else if (strcmp(argv[i], "--max-nodes") == 0 && i + 1 < argc) max_nodes = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--grace") == 0 && i + 1 < argc) grace = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc) output_path = argv[++i];
        else {
            fprintf(stderr, "Usage: %s [--samples N] [--drift-at N] [--seed N] [--noise 0..1] [--max-nodes N] [--grace N] [--output FILE]\n", argv[0]);
            return 1;
        }
    }
    if (samples == 0 || max_nodes < 1 || grace == 0 || noise < 0.0 || noise > 1.0) {
        fprintf(stderr, "Invalid arguments: samples, max-nodes and grace must be positive; noise must be 0..1.\n");
        return 1;
    }

    StreamGenerator gen;
    stream_init(&gen, seed, noise, (unsigned int)drift_at, 0, 1);

    HoeffdingTree tree;
    if (!ht_init(&tree, (size_t)max_nodes, grace, 10, 1e-7, 0.05)) {
        fprintf(stderr, "Could not initialize decision tree.\n");
        return 1;
    }
    Metrics metrics;
    metrics_init(&metrics);
    DDMDetector detector;
    ddm_init(&detector, 30);
    uint64_t warning_count = 0, drift_count = 0, first_drift_sample = UINT64_MAX;

    if (!prequential_run(&gen, samples, &tree, &metrics, &detector, output_path,
                         &warning_count, &drift_count, &first_drift_sample)) {
        perror("Could not write prequential results");
        ht_destroy(&tree);
        return 1;
    }

    printf("StreamTree-C | Incremental Hoeffding Tree Prototype\n");
    printf("Samples evaluated : %lu\n", samples);
    printf("Random seed       : %lu\n", seed);
    printf("Noise rate        : %.2f\n", noise);
    printf("Drift point       : %lu\n", drift_at);
    printf("Prequential acc.  : %.4f (%.2f%%)\n", metrics_accuracy(&metrics), metrics_accuracy(&metrics) * 100.0);
    printf("Macro-F1          : %.4f\n", metrics_macro_f1(&metrics));
    printf("Tree nodes        : %zu / %zu\n", tree.node_count, tree.max_nodes);
    printf("DDM warnings      : %llu\n", (unsigned long long)warning_count);
    printf("Drift alarms      : %llu\n", (unsigned long long)drift_count);
    if (first_drift_sample != UINT64_MAX)
        printf("First drift alarm : %llu\n", (unsigned long long)first_drift_sample);
    else
        printf("First drift alarm : none\n");
    printf("Adaptation        : full-tree reset on confirmed drift\n");
    printf("Result CSV        : %s\n", output_path);

    ht_destroy(&tree);
    return 0;
}
