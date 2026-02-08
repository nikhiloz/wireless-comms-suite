/**
 * @file demo.c
 * @brief Chapter 12 — Interleaving & Burst Error Protection
 *
 * Build:  make build/bin/12-interleaving
 * Run:    ./build/bin/12-interleaving
 */

#include <stdio.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/coding.h"

#define DATA_LEN 48

int main(void)
{
    rng_seed(12);
    print_separator("Chapter 12: Interleaving & Burst Error Protection");

    /* ── Block interleaver ───────────────────────────────────── */
    printf("1. Block Interleaver (8 rows × 6 cols = 48 bits)\n\n");

    uint8_t data[DATA_LEN];
    random_bits(data, DATA_LEN);
    printf("   Original: ");
    for (int i = 0; i < DATA_LEN; i++) printf("%d", data[i]);
    printf("\n");

    Interleaver itl;
    interleaver_init(&itl, 8, 6);

    uint8_t interleaved[DATA_LEN];
    interleaver_apply(&itl, data, interleaved, DATA_LEN);
    printf("   Interleaved: ");
    for (int i = 0; i < DATA_LEN; i++) printf("%d", interleaved[i]);
    printf("\n");

    /* Simulate burst error: corrupt 6 consecutive bits */
    printf("\n2. Burst Error (6 consecutive bits corrupted)\n");
    uint8_t corrupted[DATA_LEN];
    memcpy(corrupted, interleaved, DATA_LEN);
    for (int i = 10; i < 16; i++) corrupted[i] ^= 1;
    printf("   Corrupted:   ");
    for (int i = 0; i < DATA_LEN; i++) {
        if (i >= 10 && i < 16) printf("*");
        else printf("%d", corrupted[i]);
    }
    printf("\n");

    /* Deinterleave — burst error gets spread */
    uint8_t deinterleaved[DATA_LEN];
    interleaver_deapply(&itl, corrupted, deinterleaved, DATA_LEN);

    int errors = bit_errors(data, deinterleaved, DATA_LEN);
    printf("   After deinterleave: %d scattered errors (from 6-bit burst)\n", errors);
    printf("   Errors now spread: easier for FEC to correct\n");

    interleaver_free(&itl);

    print_separator("End of Chapter 12");
    return 0;
}
