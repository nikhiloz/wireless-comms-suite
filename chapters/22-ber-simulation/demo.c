/**
 * @file demo.c
 * @brief Chapter 22 — BER/PER Monte Carlo Simulation Framework
 *
 * Build:  make build/bin/22-ber-simulation
 * Run:    ./build/bin/22-ber-simulation
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"
#include "../../include/coding.h"

#define N_BITS      10000
#define MIN_ERRORS  50

int main(void)
{
    rng_seed(22);
    print_separator("Chapter 22: BER Monte Carlo Simulation");

    /* ── BPSK BER simulation ─────────────────────────────────── */
    printf("1. BPSK over AWGN — Monte Carlo BER\n\n");
    printf("  Eb/N0(dB)  Simulated    Theoretical  Ratio\n");
    printf("  ─────────  ──────────   ──────────   ─────\n");

    for (double ebn0 = 0; ebn0 <= 12; ebn0 += 1) {
        int total_bits = 0, total_errors = 0;

        /* Run until MIN_ERRORS or max iterations */
        while (total_errors < MIN_ERRORS && total_bits < 10000000) {
            uint8_t tx[N_BITS];
            random_bits(tx, N_BITS);

            Cplx syms[N_BITS], rx[N_BITS];
            mod_modulate(MOD_BPSK, tx, N_BITS, syms);
            channel_awgn(syms, N_BITS, ebn0_to_snr(ebn0, 1, 1.0, 1), rx);

            uint8_t dec[N_BITS];
            mod_demodulate(MOD_BPSK, rx, N_BITS, dec);
            total_errors += bit_errors(tx, dec, N_BITS);
            total_bits += N_BITS;
        }

        double ber_sim = (double)total_errors / total_bits;
        double ber_th = ber_bpsk_theory(ebn0);
        double ratio = (ber_th > 0) ? ber_sim / ber_th : 0;

        printf("  %5.1f      %.4e    %.4e    %.2f\n", ebn0, ber_sim, ber_th, ratio);
    }

    /* ── Multi-scheme comparison ─────────────────────────────── */
    printf("\n2. Modulation Comparison at Eb/N0 = 8 dB\n\n");
    double ebn0 = 8.0;
    struct { ModScheme mod; const char *name; int bps; } mods[] = {
        {MOD_BPSK,  "BPSK",   1},
        {MOD_QPSK,  "QPSK",   2},
        {MOD_8PSK,  "8-PSK",  3},
        {MOD_16QAM, "16-QAM", 4},
    };

    for (int m = 0; m < 4; m++) {
        uint8_t tx[N_BITS];
        random_bits(tx, N_BITS);
        int nbits = (N_BITS / mods[m].bps) * mods[m].bps;
        int nsyms = nbits / mods[m].bps;

        Cplx *syms = malloc(nsyms * sizeof(Cplx));
        Cplx *rx = malloc(nsyms * sizeof(Cplx));
        mod_modulate(mods[m].mod, tx, nbits, syms);
        channel_awgn(syms, nsyms, ebn0_to_snr(ebn0, mods[m].bps, 1.0, 1), rx);

        uint8_t dec[N_BITS];
        mod_demodulate(mods[m].mod, rx, nsyms, dec);
        int errs = bit_errors(tx, dec, nbits);
        printf("  %-8s BER = %.4e (%d errors / %d bits)\n",
               mods[m].name, (double)errs / nbits, errs, nbits);
        free(syms);
        free(rx);
    }

    print_separator("End of Chapter 22");
    return 0;
}
