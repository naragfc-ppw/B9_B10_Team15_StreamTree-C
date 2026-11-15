#include "drift.h"
#include <stdio.h>

int main(void) {
    DDMDetector d;
    ddm_init(&d, 30);
    int detected_at = -1;
    for (int i = 0; i < 300; ++i) {
        int error = i < 120 ? 0 : 1;
        DriftStatus status = ddm_update(&d, error);
        if (status == DRIFT_CONFIRMED) {
            detected_at = i;
            break;
        }
    }
    printf("Controlled drift detection sample: %d\n", detected_at);
    return detected_at >= 120 ? 0 : 1;
}
