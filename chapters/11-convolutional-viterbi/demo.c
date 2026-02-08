/**
 * @file demo.c
 * @brief Chapter 11 — Convolutional Codes + Viterbi Decoder
 *
 * Build:  make build/bin/11-convolutional-viterbi
 * Run:    ./build/bin/11-convolutional-viterbi
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/coding.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"

#define N_BITS  100

int main(void)
{
    rng_seed(11);
    print_separator("Chapter 11: Convolutional Codes + Viterbi Decoder");

    printf("Rate 1/2, K=7, generators [133, 171] octal\n\n");

    /* Encode */
    uint8_t info_bits[N_BITS];
    random_bits(info_bits, N_BITS);

    uint8_t coded[512];
    conv_encode(info_bits, N_BITS, coded);
    int coded_len = 2 * N_BITS;
    printf("  Input:  %d bits\n", N_BITS);
    printf("  Coded:  %d bits (rate = %.2f)\n\n", coded_len,
           (double)N_BITS / coded_len);

    /* BER comparison: coded vs uncoded */
    printf("  Eb/N0(dB)  Uncoded BER   Coded BER     Coding Gain\n");
    printf("  ─────────  ──────────    ──────────    ───────────\n");

    for (double ebn0 = 0; ebn0 <= 10; ebn0 += 2) {
        /* Uncoded BPSK */
        Cplx syms_uc[N_BITS], rx_uc[N_BITS];
        mod_modulate(MOD_BPSK, info_bits, N_BITS, syms_uc);
        channel_awgn(syms_uc, N_BITS, ebn0_to_snr(ebn0, 1, 1.0, 1), rx_uc);
        uint8_t dec_uc[N_BITS];
        mod_demodulate(MOD_BPSK, rx_uc, N_BITS, dec_uc);
        double ber_uc = (double)bit_errors(info_bits, dec_uc, N_BITS) / N_BITS;

        /* Coded BPSK: rate 1/2 → Eb/N0 adjusted for code rate */
        uint8_t cb[512];
        conv_encode(info_bits, N_BITS, cb);
        int cl = 2 * N_BITS;
        Cplx syms_c[512], rx_c[512];
        mod_modulate(MOD_BPSK, cb, cl, syms_c);
        double code_rate = 0.5;
        double snr_coded = ebn0_to_snr(ebn0 + 10 * log10(code_rate), 1, 1.0, 1);
        channel_awgn(syms_c, cl, snr_coded, rx_c);
        uint8_t hard_bits[512];
        mod_demodulate(MOD_BPSK, rx_c, cl, hard_bits);
        uint8_t dec_c[N_BITS];
        viterbi_decode(hard_bits, cl, dec_c);
        double ber_c = (double)bit_errors(info_bits, dec_c, N_BITS) / N_BITS;

        double gain = (ber_uc > 0 && ber_c > 0) ?
            10 * log10(ber_uc / ber_c) : 0;
        printf("  %5.1f      %.4e      %.4e      %+.1f dB\n",
               ebn0, ber_uc, ber_c, gain);
    }

    print_separator("End of Chapter 11");
    return 0;
}
