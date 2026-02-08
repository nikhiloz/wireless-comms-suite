/**
 * @file demo.c
 * @brief Chapter 19 — LoRa PHY (CSS Chirps, Spreading Factors)
 *
 * Build:  make build/bin/19-lora-phy
 * Run:    ./build/bin/19-lora-phy
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"

int main(void)
{
    rng_seed(19);
    print_separator("Chapter 19: LoRa PHY — Chirp Spread Spectrum");

    /* ── SF comparison ───────────────────────────────────────── */
    printf("Spreading Factor Comparison:\n\n");
    printf("  SF   Symbols  Bits/sym  Chirp len  Symbol rate (125kHz BW)\n");
    printf("  ──   ───────  ────────  ─────────  ──────────────────────\n");
    for (int sf = 7; sf <= 12; sf++) {
        int N = 1 << sf;
        double sym_rate = 125000.0 / N;
        printf("  %2d   %5d    %d bits    %5d      %.1f sym/s\n",
               sf, N, sf, N, sym_rate);
    }
    printf("\n");

    /* ── SF7 modulation ──────────────────────────────────────── */
    printf("1. LoRa SF7 Modulate → Demodulate\n");
    LoraParams lp;
    lora_init(&lp, 7, 125000, 1);
    printf("   N_FFT = %d, BW = %d Hz\n", lp.n_fft, lp.bw);

    int test_syms[] = {0, 42, 100, 127};
    for (int t = 0; t < 4; t++) {
        Cplx chirp[128];
        lora_modulate_symbol(&lp, test_syms[t], chirp);
        int decoded = lora_demodulate_symbol(&lp, chirp);
        printf("   Symbol %3d → demod = %3d %s\n",
               test_syms[t], decoded,
               (decoded == test_syms[t]) ? "✓" : "✗");
    }

    /* ── Build LoRa frame ────────────────────────────────────── */
    printf("\n2. LoRa Frame (8 preamble + payload)\n");
    uint8_t payload[5] = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; /* "Hello" */
    Cplx frame[4096];
    int frame_len = lora_build_frame(&lp, payload, 5, frame);
    printf("   Payload: 5 bytes → %d samples\n", frame_len);
    printf("   Duration: %.2f ms (at %d Hz)\n",
           1000.0 * frame_len / lp.fs, lp.fs);

    print_separator("End of Chapter 19");
    return 0;
}
