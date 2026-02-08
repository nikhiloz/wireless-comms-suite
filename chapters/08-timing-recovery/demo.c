/**
 * @file demo.c
 * @brief Chapter 08 — Symbol Timing Recovery (Gardner, Mueller-Muller)
 *
 * Build:  make build/bin/08-timing-recovery
 * Run:    ./build/bin/08-timing-recovery
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/sync.h"

#define N_BITS  100
#define SPS     4

int main(void)
{
    rng_seed(8);
    print_separator("Chapter 08: Symbol Timing Recovery");

    /* Generate BPSK signal at SPS samples/symbol */
    uint8_t bits[N_BITS];
    random_bits(bits, N_BITS);
    double nrz[N_BITS];
    nrz_encode(bits, N_BITS, nrz);

    /* Upsample */
    int sig_len = N_BITS * SPS;
    Cplx signal[1024];
    for (int i = 0; i < sig_len; i++) {
        int sym_idx = i / SPS;
        signal[i] = cplx(nrz[sym_idx], 0.0);
    }

    /* ── Gardner timing recovery ─────────────────────────────── */
    printf("1. Gardner Timing Recovery (SPS=%d)\n", SPS);
    TimingRecovery tr;
    timing_init(&tr, SPS, 0.01, 0.707);

    Cplx recovered[N_BITS];
    int n_recovered = timing_recover_gardner(&tr, signal, sig_len, recovered);

    printf("   Recovered %d symbols from %d samples\n", n_recovered, sig_len);

    /* Check quality */
    int correct = 0;
    int count = (n_recovered < N_BITS) ? n_recovered : N_BITS;
    for (int i = 0; i < count; i++) {
        int det = (recovered[i].re > 0) ? 1 : 0;
        if (det == bits[i]) correct++;
    }
    printf("   Accuracy: %d/%d (%.1f%%)\n\n", correct, count,
           100.0 * correct / count);

    /* ── Mueller-Muller timing recovery ──────────────────────── */
    printf("2. Mueller-Muller Timing Recovery\n");
    timing_init(&tr, SPS, 0.01, 0.707);
    n_recovered = timing_recover_mm(&tr, signal, sig_len, recovered);
    printf("   Recovered %d symbols\n", n_recovered);

    print_separator("End of Chapter 08");
    return 0;
}
