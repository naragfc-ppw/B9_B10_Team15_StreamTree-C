#include "stream_gen.h"
#include <stddef.h>

static uint32_t next_u32(StreamGenerator *gen) {
    /* xorshift64*: deterministic pseudo-random generator for reproducible runs */
    uint64_t x = gen->state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    gen->state = x;
    return (uint32_t)((x * UINT64_C(2685821657736338717)) >> 32);
}

static double uniform01(StreamGenerator *gen) {
    return (double)next_u32(gen) / 4294967296.0;
}

void stream_init(StreamGenerator *gen, uint64_t seed, double noise_rate,
                 unsigned int drift_at, int concept_before, int concept_after) {
    if (gen == NULL) return;
    gen->state = seed ? seed : UINT64_C(88172645463393265);
    gen->noise_rate = noise_rate < 0.0 ? 0.0 : (noise_rate > 1.0 ? 1.0 : noise_rate);
    gen->drift_at = drift_at;
    gen->concept_before = concept_before;
    gen->concept_after = concept_after;
}

Sample stream_next(StreamGenerator *gen, uint64_t index) {
    Sample s = {{0.0, 0.0, 0.0}, 0, index, 0};
    if (gen == NULL) return s;

    for (int i = 0; i < ST_FEATURES; ++i) s.x[i] = uniform01(gen) * 10.0;
    s.concept_id = (gen->drift_at > 0 && index >= gen->drift_at)
                       ? gen->concept_after : gen->concept_before;

    /* SEA-style concepts: class 1 when x[0] + x[1] <= threshold. */
    static const double thresholds[4] = {8.0, 9.0, 7.0, 9.5};
    int c = s.concept_id % 4;
    if (c < 0) c += 4;
    s.label = (s.x[0] + s.x[1] <= thresholds[c]) ? 1 : 0;

    if (uniform01(gen) < gen->noise_rate) s.label = 1 - s.label;
    return s;
}
