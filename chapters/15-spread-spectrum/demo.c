/**
 * @file demo.c
 * @brief Chapter 15 — Spread Spectrum (DSSS, FHSS, PN Sequences)
 *
 * Build:  make build/bin/15-spread-spectrum
 * Run:    ./build/bin/15-spread-spectrum
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/spread_spectrum.h"

#define N_DATA 8

int main(void)
{
    rng_seed(15);
    print_separator("Chapter 15: Spread Spectrum (DSSS & FHSS)");

    /* ── PN sequence generation ──────────────────────────────── */
    printf("1. M-Sequence (5-bit LFSR, length 31)\n");
    int mseq[31];
    int len = pn_msequence(0x12, 5, mseq);  /* 5-bit LFSR → 31 chips */
    printf("   Length = %d, first 31: ", len);
    for (int i = 0; i < len; i++) printf("%+d", mseq[i]);
    printf("\n");

    /* Autocorrelation */
    double acorr[63];
    pn_autocorr(mseq, len, acorr);
    printf("   R(0) = %.1f, R(1) = %.1f\n\n", acorr[0], acorr[1]);

    /* ── DSSS spread/despread ────────────────────────────────── */
    printf("2. DSSS (Direct Sequence Spread Spectrum)\n");
    int code[7];
    pn_msequence(0x05, 3, code);  /* 3-bit LFSR → 7 chips */
    printf("   PN code (len=7): ");
    for (int i = 0; i < 7; i++) printf("%+d", code[i]);
    printf("\n");

    uint8_t data_bits[N_DATA] = {1, 0, 1, 1, 0, 0, 1, 0};
    printf("   Data: ");
    for (int i = 0; i < N_DATA; i++) printf("%d ", data_bits[i]);
    printf("\n");

    double spread[56];
    dsss_spread(data_bits, N_DATA, code, 7, spread);
    printf("   Spread: %d chips (processing gain = %.1f dB)\n",
           N_DATA * 7, dsss_processing_gain_db(7));

    uint8_t despread_bits[N_DATA];
    dsss_despread(spread, 56, code, 7, despread_bits);
    printf("   Despread: ");
    for (int i = 0; i < N_DATA; i++) printf("%d ", despread_bits[i]);
    printf("\n\n");

    /* ── FHSS ────────────────────────────────────────────────── */
    printf("3. FHSS (Frequency Hopping)\n");
    FhssParams fh;
    fhss_init(&fh, 20, 15, 12345, 0.001);
    printf("   %d channels, hop sequence: ", 20);
    for (int i = 0; i < 15; i++)
        printf("%d ", fhss_get_channel(&fh, i));
    printf("...\n\n");

    /* ── Gold codes ──────────────────────────────────────────── */
    printf("4. Gold Code (length 31)\n");
    int gold[31];
    int glen = pn_gold(0x12, 0x1E, 5, 0, gold);  /* 5-bit → 31 chips */
    printf("   Length = %d\n", glen);

    print_separator("End of Chapter 15");
    return 0;
}
