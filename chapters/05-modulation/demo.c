/**
 * @file demo.c
 * @brief Chapter 05 — Digital Modulation (BPSK, QPSK, 16-QAM)
 *
 * Demonstrates constellation diagrams, modulation/demodulation, and BER curves.
 *
 * Build:  make build/bin/05-modulation
 * Run:    ./build/bin/05-modulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"

#define N_BITS  10000

int main(void)
{
    rng_seed(5);
    print_separator("Chapter 05: Digital Modulation — BPSK, QPSK, 16-QAM");

    /* ── BPSK/QPSK/16-QAM comparison ────────────────────────── */
    ModScheme schemes[] = {MOD_BPSK, MOD_QPSK, MOD_16QAM};
    int bps[] = {1, 2, 4};

    printf("BER vs Eb/N0 (Monte Carlo, %d bits per point)\n\n", N_BITS);
    printf("  Eb/N0(dB)   BPSK         QPSK         16-QAM\n");
    printf("  ─────────   ──────────   ──────────   ──────────\n");

    double snr_vals[] = {0, 2, 4, 6, 8, 10, 12};
    int n_snr = 7;

    for (int s = 0; s < n_snr; s++) {
        double ebn0 = snr_vals[s];
        printf("  %5.1f      ", ebn0);

        for (int m = 0; m < 3; m++) {
            uint8_t tx_bits[N_BITS];
            random_bits(tx_bits, N_BITS);

            int nsyms = N_BITS / bps[m];
            Cplx *syms = (Cplx *)malloc(nsyms * sizeof(Cplx));
            mod_modulate(schemes[m], tx_bits, N_BITS, syms);

            /* AWGN */
            double snr = ebn0_to_snr(ebn0, bps[m], 1.0, 1);
            Cplx *rx = (Cplx *)malloc(nsyms * sizeof(Cplx));
            channel_awgn(syms, nsyms, snr, rx);

            uint8_t rx_bits[N_BITS];
            mod_demodulate(schemes[m], rx, nsyms, rx_bits);

            int errs = bit_errors(tx_bits, rx_bits, N_BITS);
            double ber = (double)errs / N_BITS;
            printf(" %.4e   ", ber);

            free(syms);
            free(rx);
        }
        printf("\n");
    }

    /* ── Show QPSK constellation at 10 dB ────────────────────── */
    printf("\nQPSK Constellation at Eb/N0 = 10 dB:\n");
    uint8_t bits[200];
    random_bits(bits, 200);
    Cplx syms[100];
    mod_modulate(MOD_QPSK, bits, 200, syms);
    Cplx rx[100];
    channel_awgn(syms, 100, ebn0_to_snr(10.0, 2, 1.0, 1), rx);
    print_constellation_ascii(rx, 100, 21);

    print_separator("End of Chapter 05");
    return 0;
}
