/**
 * @file demo.c
 * @brief Chapter 10 — Frame Synchronisation (Barker codes, preamble detection)
 *
 * Build:  make build/bin/10-frame-sync
 * Run:    ./build/bin/10-frame-sync
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/sync.h"

#define SIG_LEN 200

int main(void)
{
    rng_seed(10);
    print_separator("Chapter 10: Frame Synchronisation");

    /* ── Barker-13 autocorrelation ───────────────────────────── */
    printf("1. Barker-13 Sequence\n");
    static const int barker13[13] = {1,1,1,1,1,-1,-1,1,1,-1,1,-1,1};
    printf("   Barker-13: ");
    for (int i = 0; i < 13; i++) printf("%+d ", barker13[i]);
    printf("\n");

    /* Compute autocorrelation */
    double autocorr[25];
    for (int lag = -12; lag <= 12; lag++) {
        double sum = 0;
        for (int i = 0; i < 13; i++) {
            int j = i + lag;
            if (j >= 0 && j < 13) sum += barker13[i] * barker13[j];
        }
        autocorr[lag + 12] = sum;
    }
    printf("   Autocorrelation peak = %.0f, max sidelobe = ",
           autocorr[12]);
    double max_side = 0;
    for (int i = 0; i < 25; i++) {
        if (i != 12 && fabs(autocorr[i]) > max_side)
            max_side = fabs(autocorr[i]);
    }
    printf("%.0f (ratio = %.1f)\n\n", max_side, autocorr[12] / max_side);

    /* ── Frame detection in noisy signal ─────────────────────── */
    printf("2. Frame Detection in Noise\n");
    double signal[SIG_LEN];
    for (int i = 0; i < SIG_LEN; i++)
        signal[i] = rng_gaussian() * 0.5;

    /* Embed Barker-13 at position 73 */
    int embed_pos = 73;
    for (int i = 0; i < 13; i++)
        signal[embed_pos + i] += barker13[i];

    int detected = frame_sync_detect(signal, SIG_LEN, BARKER_13, 13, 5.0);
    printf("   Embedded at position: %d\n", embed_pos);
    printf("   Detected at position: %d ", detected);
    printf("%s\n\n", (detected == embed_pos) ? "(correct)" : "(WRONG)");

    /* ── Scrambler demonstration ─────────────────────────────── */
    printf("3. Data Scrambling\n");
    uint8_t data[16];
    random_bits(data, 16);
    printf("   Original: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    printf("\n");

    scrambler(0x48, 0x7F, data, 16);
    printf("   Scrambled: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    printf("\n");

    scrambler(0x48, 0x7F, data, 16);
    printf("   Descrambled: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    printf("\n");

    print_separator("End of Chapter 10");
    return 0;
}
