#include "hoeffding_tree.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FEATURE_MIN 0.0
#define FEATURE_MAX 10.0

static HTNode *node_create(void) {
    HTNode *node = (HTNode *)calloc(1, sizeof(HTNode));
    if (node != NULL) {
        node->is_leaf = 1;
        node->split_feature = -1;
    }
    return node;
}

static void node_destroy(HTNode *node) {
    if (node == NULL) return;
    node_destroy(node->left);
    node_destroy(node->right);
    free(node);
}

int ht_init(HoeffdingTree *tree, size_t max_nodes, uint64_t grace_period,
            uint64_t min_child_samples, double delta, double tie_threshold) {
    if (tree == NULL || max_nodes < 1 || grace_period < 1 ||
        min_child_samples < 1 || delta <= 0.0 || delta >= 1.0 || tie_threshold < 0.0)
        return 0;
    memset(tree, 0, sizeof(*tree));
    tree->root = node_create();
    if (tree->root == NULL) return 0;
    tree->node_count = 1;
    tree->max_nodes = max_nodes;
    tree->grace_period = grace_period;
    tree->min_child_samples = min_child_samples;
    tree->delta = delta;
    tree->tie_threshold = tie_threshold;
    return 1;
}

void ht_destroy(HoeffdingTree *tree) {
    if (tree == NULL) return;
    node_destroy(tree->root);
    memset(tree, 0, sizeof(*tree));
}

int ht_reset(HoeffdingTree *tree) {
    if (tree == NULL) return 0;
    HTNode *new_root = node_create();
    if (new_root == NULL) return 0;
    node_destroy(tree->root);
    tree->root = new_root;
    tree->node_count = 1;
    return 1;
}

static int majority_class(const HTNode *node) {
    return node->class_counts[1] > node->class_counts[0] ? 1 : 0;
}

static HTNode *find_leaf(const HoeffdingTree *tree, const double x[ST_FEATURES]) {
    HTNode *node = tree == NULL ? NULL : tree->root;
    while (node != NULL && !node->is_leaf) {
        int f = node->split_feature;
        node = (x[f] <= node->split_threshold) ? node->left : node->right;
    }
    return node;
}

int ht_predict(const HoeffdingTree *tree, const double x[ST_FEATURES]) {
    HTNode *leaf = find_leaf(tree, x);
    return leaf == NULL ? 0 : majority_class(leaf);
}

static double gini(uint64_t c0, uint64_t c1) {
    double total = (double)(c0 + c1);
    if (total == 0.0) return 0.0;
    double p0 = (double)c0 / total;
    double p1 = (double)c1 / total;
    return 1.0 - p0 * p0 - p1 * p1;
}

typedef struct {
    int feature;
    int cut_bin;
    double gain;
    uint64_t left[ST_CLASSES];
    uint64_t right[ST_CLASSES];
} SplitCandidate;

static SplitCandidate best_split(const HTNode *leaf) {
    SplitCandidate best = {-1, -1, -1.0, {0, 0}, {0, 0}};
    double second_gain = -1.0;
    uint64_t total0 = leaf->class_counts[0];
    uint64_t total1 = leaf->class_counts[1];
    double parent = gini(total0, total1);
    uint64_t n = total0 + total1;

    for (int f = 0; f < ST_FEATURES; ++f) {
        uint64_t left0 = 0, left1 = 0;
        for (int b = 0; b < HT_BINS - 1; ++b) {
            left0 += leaf->hist[f][b][0];
            left1 += leaf->hist[f][b][1];
            uint64_t right0 = total0 - left0;
            uint64_t right1 = total1 - left1;
            uint64_t nl = left0 + left1;
            uint64_t nr = right0 + right1;
            if (nl == 0 || nr == 0) continue;
            double weighted = ((double)nl / (double)n) * gini(left0, left1) +
                              ((double)nr / (double)n) * gini(right0, right1);
            double gain = parent - weighted;
            if (gain > best.gain) {
                second_gain = best.gain;
                best.feature = f;
                best.cut_bin = b;
                best.gain = gain;
                best.left[0] = left0; best.left[1] = left1;
                best.right[0] = right0; best.right[1] = right1;
            } else if (gain > second_gain) {
                second_gain = gain;
            }
        }
    }
    /* Store the runner-up gain in an otherwise unused slot via a static-free
       recomputation in try_split; this function returns only the best candidate. */
    (void)second_gain;
    return best;
}

static double second_best_gain(const HTNode *leaf, double best_gain) {
    uint64_t total0 = leaf->class_counts[0], total1 = leaf->class_counts[1];
    uint64_t n = total0 + total1;
    double parent = gini(total0, total1);
    double second = 0.0;
    for (int f = 0; f < ST_FEATURES; ++f) {
        uint64_t l0 = 0, l1 = 0;
        for (int b = 0; b < HT_BINS - 1; ++b) {
            l0 += leaf->hist[f][b][0]; l1 += leaf->hist[f][b][1];
            uint64_t r0 = total0 - l0, r1 = total1 - l1;
            uint64_t nl = l0 + l1, nr = r0 + r1;
            if (nl == 0 || nr == 0 || n == 0) continue;
            double weighted = ((double)nl / (double)n) * gini(l0, l1) +
                              ((double)nr / (double)n) * gini(r0, r1);
            double gain = parent - weighted;
            if (gain < best_gain && gain > second) second = gain;
        }
    }
    return second;
}

static void try_split(HoeffdingTree *tree, HTNode *leaf) {
    if (tree->node_count + 2 > tree->max_nodes || leaf->samples_seen < tree->grace_period)
        return;
    SplitCandidate candidate = best_split(leaf);
    if (candidate.feature < 0) return;
    uint64_t left_n = candidate.left[0] + candidate.left[1];
    uint64_t right_n = candidate.right[0] + candidate.right[1];
    if (left_n < tree->min_child_samples || right_n < tree->min_child_samples) return;

    double second = second_best_gain(leaf, candidate.gain);
    double epsilon = sqrt(log(1.0 / tree->delta) / (2.0 * (double)leaf->samples_seen)) * 0.5;
    if ((candidate.gain - second) <= epsilon && epsilon >= tree->tie_threshold) return;

    HTNode *left = node_create();
    HTNode *right = node_create();
    if (left == NULL || right == NULL) {
        free(left); free(right);
        return;
    }
    left->class_counts[0] = candidate.left[0];
    left->class_counts[1] = candidate.left[1];
    right->class_counts[0] = candidate.right[0];
    right->class_counts[1] = candidate.right[1];

    leaf->is_leaf = 0;
    leaf->split_feature = candidate.feature;
    leaf->split_threshold = FEATURE_MIN +
        ((double)(candidate.cut_bin + 1) / (double)HT_BINS) * (FEATURE_MAX - FEATURE_MIN);
    leaf->left = left;
    leaf->right = right;
    tree->node_count += 2;
}

void ht_train(HoeffdingTree *tree, const double x[ST_FEATURES], int label) {
    if (tree == NULL || x == NULL || label < 0 || label >= ST_CLASSES) return;
    HTNode *leaf = find_leaf(tree, x);
    if (leaf == NULL) return;
    leaf->class_counts[label]++;
    leaf->samples_seen++;

    for (int f = 0; f < ST_FEATURES; ++f) {
        double value = x[f];
        if (value < FEATURE_MIN) value = FEATURE_MIN;
        if (value >= FEATURE_MAX) value = FEATURE_MAX - 1e-9;
        int bin = (int)((value - FEATURE_MIN) / (FEATURE_MAX - FEATURE_MIN) * HT_BINS);
        if (bin < 0) bin = 0;
        if (bin >= HT_BINS) bin = HT_BINS - 1;
        leaf->hist[f][bin][label]++;
    }
    if (leaf->samples_seen % tree->grace_period == 0)
        try_split(tree, leaf);
}
