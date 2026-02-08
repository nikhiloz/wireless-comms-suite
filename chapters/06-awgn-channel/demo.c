/**
 * @file demo.c
 * @brief Chapter 06 — AWGN Channel Simulation
 *
 * Demonstrates Gaussian noise generation, SNR control, and BER vs Eb/N0.
 *
 * Build:  make build/bin/06-awgn-channel
 * Run:    ./build/bin/06-awgn-channel
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"

#define N_BITS  5000

int main(void)
{
    rng_seed(6);
    print_separator("Chapter 06: AWGN Channel Simulation");

    printf("BPSK BER vs Eb/N0 — Simulation vs Theory\n\n");
    printf("  Eb/N0(dB)  Simulated    Theoretical\n");
    printf("  ─────────  ──────────   ──────────\n");

    for (double ebn0 = 0; ebn0 <= 12; ebn0 += 2) {
        uint8_t tx_bits[N_BITS];
        random_bits(tx_bits, N_BITS);

        Cplx syms[N_BITS];
        mod_modulate(MOD_BPSK, tx_bits, N_BITS, syms);

        Cplx rx[N_BITS];
        double snr = ebn0_to_snr(ebn0, 1, 1.0, 1);
        channel_awgn(syms, N_BITS, snr, rx);

        uint8_t rx_bits[N_BITS];
        mod_demodulate(MOD_BPSK, rx, N_BITS, rx_bits);

        int errs = bit_errors(tx_bits, rx_bits, N_BITS);
        double ber_sim   = (double)errs / N_BITS;
        double ber_theory = ber_bpsk_theory(ebn0);

        printf("  %5.1f      %.4e    %.4e\n", ebn0, ber_sim, ber_theory);
    }

    /* Noise statistics */
    printf("\nNoise statistics at SNR=10 dB:\n");
    int N = 10000;
    double noise_re[10000];
    for (int i = 0; i < N; i++) noise_re[i] = 0;
    double *noise = noise_re;
    channel_awgn_real(noise, N, 10.0, noise);
    double mean = 0, var = 0;
    for (int i = 0; i < N; i++) mean += noise[i];
    mean /= N;
    for (int i = 0; i < N; i++) var += (noise[i] - mean) * (noise[i] - mean);
    var /= N;
    printf("  Mean = %.4f, Variance = %.4f\n", mean, var);

    print_separator("End of Chapter 06");
    return 0;
}
