/**
 * @file demo.c
 * @brief Chapter 01 — Digital Communication System Overview
 *
 * Demonstrates the complete TX → Channel → RX pipeline:
 *   Source bits → Modulation → AWGN channel → Demodulation → BER
 *
 * Build:  make build/bin/01-system-overview
 * Run:    ./build/bin/01-system-overview
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"

#define N_BITS   1000
#define SNR_DB   10.0

int main(void)
{
    rng_seed(1);
    print_separator("Chapter 01: Digital Communication System Overview");

    printf("TX → AWGN Channel → RX Pipeline\n\n");
    printf("  Bits → [BPSK Mod] → [AWGN %.1f dB] → [Demod] → Bits\n\n", SNR_DB);

    /* 1. Generate random source bits */
    uint8_t tx_bits[N_BITS];
    random_bits(tx_bits, N_BITS);

    /* 2. BPSK modulate */
    Cplx symbols[N_BITS];
    mod_modulate(MOD_BPSK, tx_bits, N_BITS, symbols);
    printf("  TX: %d bits → %d BPSK symbols\n", N_BITS, N_BITS);

    /* 3. AWGN channel */
    Cplx rx_symbols[N_BITS];
    channel_awgn(symbols, N_BITS, SNR_DB, rx_symbols);
    printf("  Channel: AWGN with SNR = %.1f dB\n", SNR_DB);

    /* 4. Demodulate */
    uint8_t rx_bits[N_BITS];
    mod_demodulate(MOD_BPSK, rx_symbols, N_BITS, rx_bits);

    /* 5. Count errors */
    int errors = bit_errors(tx_bits, rx_bits, N_BITS);
    double ber = (double)errors / N_BITS;
    double ber_theory = ber_bpsk_theory(SNR_DB);

    printf("  RX: %d bit errors out of %d → BER = %.6f\n", errors, N_BITS, ber);
    printf("  Theory BER (BPSK, %.1f dB) = %.6f\n\n", SNR_DB, ber_theory);

    /* 6. ASCII constellation */
    print_constellation_ascii(rx_symbols, (N_BITS < 200) ? N_BITS : 200, 21);

    print_separator("End of Chapter 01");
    return 0;
}
