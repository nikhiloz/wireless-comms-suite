/**
 * @file demo.c
 * @brief Chapter 09 — Carrier Synchronisation (Costas Loop, PLL)
 *
 * Build:  make build/bin/09-carrier-sync
 * Run:    ./build/bin/09-carrier-sync
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/sync.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N_SYMS  200

int main(void)
{
    rng_seed(9);
    print_separator("Chapter 09: Carrier Synchronisation");

    /* Generate BPSK signal with frequency offset */
    uint8_t bits[N_SYMS];
    random_bits(bits, N_SYMS);
    Cplx syms[N_SYMS];
    mod_modulate(MOD_BPSK, bits, N_SYMS, syms);

    /* Add frequency offset: 0.01 rad/sample */
    double freq_offset = 0.01;
    Cplx offset_syms[N_SYMS];
    for (int i = 0; i < N_SYMS; i++) {
        Cplx rot = cplx_exp_j(freq_offset * i);
        offset_syms[i] = cplx_mul(syms[i], rot);
    }

    /* ── Costas loop BPSK ────────────────────────────────────── */
    printf("1. Costas Loop (BPSK) — freq offset = %.3f rad/sample\n", freq_offset);
    CarrierSync cs;
    carrier_init(&cs, 0.02, 0.707);

    Cplx corrected[N_SYMS];
    carrier_costas_bpsk(&cs, offset_syms, N_SYMS, corrected);

    /* Check convergence */
    int correct = 0;
    for (int i = N_SYMS / 2; i < N_SYMS; i++) {
        int det = (corrected[i].re > 0) ? 0 : 1;
        if (det == bits[i]) correct++;
    }
    printf("   Post-loop accuracy (second half): %d/%d (%.1f%%)\n",
           correct, N_SYMS / 2, 100.0 * correct / (N_SYMS / 2));
    printf("   Final freq estimate: %.4f rad/sample\n\n", cs.freq);

    /* ── QPSK Costas loop ────────────────────────────────────── */
    printf("2. Costas Loop (QPSK)\n");
    uint8_t qbits[N_SYMS * 2];
    random_bits(qbits, N_SYMS * 2);
    Cplx qsyms[N_SYMS];
    mod_modulate(MOD_QPSK, qbits, N_SYMS * 2, qsyms);

    for (int i = 0; i < N_SYMS; i++) {
        Cplx rot = cplx_exp_j(freq_offset * i);
        offset_syms[i] = cplx_mul(qsyms[i], rot);
    }

    carrier_init(&cs, 0.02, 0.707);
    carrier_costas_qpsk(&cs, offset_syms, N_SYMS, corrected);

    printf("   Final freq estimate: %.4f rad/sample\n", cs.freq);

    print_separator("End of Chapter 09");
    return 0;
}
