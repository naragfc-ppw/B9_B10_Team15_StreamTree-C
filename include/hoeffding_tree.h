#ifndef HOEFFDING_TREE_H
#define HOEFFDING_TREE_H

#include <stddef.h>
#include <stdint.h>
#include "stream_gen.h"

#define HT_BINS 10

typedef struct HTNode HTNode;
struct HTNode {
    int is_leaf;
    int split_feature;
    double split_threshold;
    HTNode *left;
    HTNode *right;
    uint64_t class_counts[ST_CLASSES];
    uint64_t hist[ST_FEATURES][HT_BINS][ST_CLASSES];
    uint64_t samples_seen;
};

typedef struct {
    HTNode *root;
    size_t node_count;
    size_t max_nodes;
    uint64_t grace_period;
    uint64_t min_child_samples;
    double delta;
    double tie_threshold;
} HoeffdingTree;

int ht_init(HoeffdingTree *tree, size_t max_nodes, uint64_t grace_period,
            uint64_t min_child_samples, double delta, double tie_threshold);
void ht_destroy(HoeffdingTree *tree);
int ht_reset(HoeffdingTree *tree);
int ht_predict(const HoeffdingTree *tree, const double x[ST_FEATURES]);
void ht_train(HoeffdingTree *tree, const double x[ST_FEATURES], int label);

#endif
