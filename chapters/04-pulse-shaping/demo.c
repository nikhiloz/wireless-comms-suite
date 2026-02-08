/**
 * @file demo.c
 * @brief Chapter 04 — Pulse Shaping & Line Coding
 *
 * Demonstrates NRZ/Manchester encoding, raised cosine / root-raised cosine
 * filter design, and eye diagram generation.
 *
 * Build:  make build/bin/04-pulse-shaping
 * Run:    ./build/bin/04-pulse-shaping
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"

#define N_BITS  20
#define SPS     8
#define FILT_LEN 33

int main(void)
{
    rng_seed(4);
    print_separator("Chapter 04: Pulse Shaping & Line Coding");

    /* ── NRZ encoding ────────────────────────────────────────── */
    printf("1. NRZ Line Coding\n");
    uint8_t bits[N_BITS];
    random_bits(bits, N_BITS);
    double nrz[N_BITS];
    nrz_encode(bits, N_BITS, nrz);
    printf("   Bits: ");
    for (int i = 0; i < N_BITS; i++) printf("%d", bits[i]);
    printf("\n   NRZ:  ");
    for (int i = 0; i < N_BITS; i++) printf("%+.0f ", nrz[i]);
    printf("\n\n");

    /* ── Manchester encoding ─────────────────────────────────── */
    printf("2. Manchester Encoding\n");
    double manchester[N_BITS * 2];
    manchester_encode(bits, N_BITS, manchester);
    printf("   Manchester: ");
    for (int i = 0; i < N_BITS * 2 && i < 40; i++)
        printf("%+.0f ", manchester[i]);
    printf("...\n\n");

    /* ── Raised cosine filter ────────────────────────────────── */
    printf("3. Raised Cosine Filter (α=0.35, %d taps, %d sps)\n", FILT_LEN, SPS);
    double rc[FILT_LEN];
    raised_cosine(0.35, SPS, FILT_LEN / SPS, rc);
    printf("   Filter peak at tap %d = %.4f\n", FILT_LEN / 2, rc[FILT_LEN / 2]);

    /* ── Root-raised cosine ──────────────────────────────────── */
    printf("\n4. Root-Raised Cosine Filter (α=0.35)\n");
    double rrc[FILT_LEN];
    root_raised_cosine(0.35, SPS, FILT_LEN / SPS, rrc);
    printf("   Filter peak at tap %d = %.4f\n", FILT_LEN / 2, rrc[FILT_LEN / 2]);

    /* ── Pulse-shaped signal ─────────────────────────────────── */
    printf("\n5. Pulse-Shaped NRZ Signal\n");
    int out_len = N_BITS * SPS + FILT_LEN - 1;
    double shaped[512];
    pulse_shape(nrz, N_BITS, rc, FILT_LEN, SPS, shaped);
    printf("   Output: %d samples (bits×sps + filter - 1)\n", out_len);

    /* ASCII eye diagram */
    printf("\n6. Eye Diagram (2-symbol window)\n");
    print_eye_diagram_ascii(shaped, out_len, SPS, 2);

    print_separator("End of Chapter 04");
    return 0;
}
