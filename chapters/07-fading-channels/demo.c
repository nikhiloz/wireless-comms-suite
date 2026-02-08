/**
 * @file demo.c
 * @brief Chapter 07 — Fading Channels (Rayleigh, Rician, Multipath)
 *
 * Build:  make build/bin/07-fading-channels
 * Run:    ./build/bin/07-fading-channels
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"

#define N_SYMS 1000

int main(void)
{
    rng_seed(7);
    print_separator("Chapter 07: Fading Channels");

    /* ── Rayleigh flat fading ────────────────────────────────── */
    printf("1. Rayleigh Flat Fading\n");
    Cplx tx[N_SYMS], rx_ray[N_SYMS];
    uint8_t bits[N_SYMS];
    random_bits(bits, N_SYMS);
    mod_modulate(MOD_BPSK, bits, N_SYMS, tx);

    RayleighChannel rch = {0};
    Cplx h_ray[N_SYMS];
    channel_rayleigh_flat(&rch, tx, N_SYMS, rx_ray, h_ray);

    /* Add AWGN on top */
    Cplx rx_noisy[N_SYMS];
    channel_awgn(rx_ray, N_SYMS, 10.0, rx_noisy);

    uint8_t rx_bits[N_SYMS];
    mod_demodulate(MOD_BPSK, rx_noisy, N_SYMS, rx_bits);
    int errs = bit_errors(bits, rx_bits, N_SYMS);
    printf("   BER (Rayleigh + AWGN 10dB) = %.4f (%d errors / %d)\n\n",
           (double)errs / N_SYMS, errs, N_SYMS);

    /* ── Rician fading ───────────────────────────────────────── */
    printf("2. Rician Fading (K=5 dB)\n");
    Cplx rx_ric[N_SYMS];
    RicianChannel ric = {0};
    ric.k_factor = 5.0;
    ric.los_phase = 0.0;
    Cplx h_ric[N_SYMS];
    channel_rician_flat(&ric, tx, N_SYMS, rx_ric, h_ric);
    channel_awgn(rx_ric, N_SYMS, 10.0, rx_noisy);
    mod_demodulate(MOD_BPSK, rx_noisy, N_SYMS, rx_bits);
    errs = bit_errors(bits, rx_bits, N_SYMS);
    printf("   BER (Rician K=5dB + AWGN 10dB) = %.4f\n\n",
           (double)errs / N_SYMS);

    /* ── Multipath channel ───────────────────────────────────── */
    printf("3. Multipath Channel (3-tap)\n");
    MultipathChannel mch;
    int delays[] = {0, 2, 5};
    double gains_db[] = {0.0, -4.4, -10.5};  /* dB relative to first */
    channel_multipath_init(&mch, 3, delays, gains_db);
    printf("   Taps: [%.1f, %.1f, %.1f] dB at delays [%d, %d, %d]\n",
           gains_db[0], gains_db[1], gains_db[2], delays[0], delays[1], delays[2]);

    Cplx impulse[64], response[64];
    for (int i = 0; i < 32; i++) impulse[i] = cplx(i == 0 ? 1.0 : 0.0, 0.0);
    int out_len;
    channel_multipath_apply(&mch, impulse, 32, response, &out_len);

    printf("   Impulse response (first 8):\n   ");
    for (int i = 0; i < 8 && i < out_len; i++)
        printf("  h[%d]=%.2f+j%.2f", i, response[i].re, response[i].im);
    printf("\n");

    print_separator("End of Chapter 07");
    return 0;
}
