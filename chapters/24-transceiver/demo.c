/**
 * @file demo.c
 * @brief Chapter 24 — Full Transceiver Capstone
 *
 * Puts together the entire TX → Channel → RX chain:
 *   Source → FEC → Interleave → Modulate → OFDM → Channel → RX pipeline
 *
 * Build:  make build/bin/24-transceiver
 * Run:    ./build/bin/24-transceiver
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/coding.h"
#include "../../include/modulation.h"
#include "../../include/ofdm.h"
#include "../../include/channel.h"
#include "../../include/sync.h"

#define MSG_BYTES  20
#define MSG_BITS   (MSG_BYTES * 8)

int main(void)
{
    rng_seed(24);
    print_separator("Chapter 24: Full Transceiver Capstone");

    printf("Pipeline: Source → Conv encode → Interleave → QPSK → OFDM → AWGN → RX\n\n");

    /* ════════════════════ TRANSMITTER ════════════════════════ */
    printf("═══ TRANSMITTER ═══\n");

    /* 1. Source message */
    uint8_t message[MSG_BYTES];
    for (int i = 0; i < MSG_BYTES; i++) message[i] = 'A' + (i % 26);
    printf("  Message: \"");
    for (int i = 0; i < MSG_BYTES; i++) printf("%c", message[i]);
    printf("\" (%d bytes)\n", MSG_BYTES);

    uint8_t info_bits[MSG_BITS];
    bits_from_bytes(message, MSG_BYTES, info_bits);

    /* 2. Convolutional encoding (rate 1/2) */
    uint8_t coded_bits[1024];
    conv_encode(info_bits, MSG_BITS, coded_bits);
    int coded_len = 2 * MSG_BITS;
    printf("  Conv coded: %d → %d bits (rate %.2f)\n",
           MSG_BITS, coded_len, (double)MSG_BITS / coded_len);

    /* 3. Interleaving */
    int rows = 8, cols = (coded_len + 7) / 8;
    int padded_len = rows * cols;
    uint8_t padded[1024];
    memset(padded, 0, padded_len);
    memcpy(padded, coded_bits, coded_len);

    Interleaver itl;
    interleaver_init(&itl, rows, cols);
    uint8_t interleaved[1024];
    interleaver_apply(&itl, padded, interleaved, padded_len);
    printf("  Interleaved: %d×%d block\n", rows, cols);

    /* 4. QPSK modulation */
    int nsyms = padded_len / 2;
    Cplx *qpsk_syms = (Cplx *)calloc(nsyms, sizeof(Cplx));
    mod_modulate(MOD_QPSK, interleaved, padded_len, qpsk_syms);
    printf("  QPSK: %d symbols\n", nsyms);

    /* 5. OFDM modulation */
    OfdmParams ofdm;
    ofdm_init(&ofdm, 64, 16, 4);
    int n_ofdm_sym = (nsyms + ofdm.n_data - 1) / ofdm.n_data;
    int n_total_data = n_ofdm_sym * ofdm.n_data;

    /* Pad QPSK symbols */
    Cplx *ofdm_data = (Cplx *)calloc(n_total_data, sizeof(Cplx));
    memcpy(ofdm_data, qpsk_syms, nsyms * sizeof(Cplx));

    int tx_samples_len = n_ofdm_sym * (ofdm.n_fft + ofdm.n_cp);
    Cplx *tx_signal = (Cplx *)calloc(tx_samples_len, sizeof(Cplx));
    ofdm_modulate_block(&ofdm, n_ofdm_sym, ofdm_data, tx_signal);
    printf("  OFDM: %d symbols × %d samples = %d total\n\n",
           n_ofdm_sym, ofdm.n_fft + ofdm.n_cp, tx_samples_len);

    /* ════════════════════ CHANNEL ════════════════════════════ */
    double snr_db = 15.0;
    printf("═══ CHANNEL (AWGN, SNR = %.1f dB) ═══\n\n", snr_db);
    Cplx *rx_signal = (Cplx *)calloc(tx_samples_len, sizeof(Cplx));
    channel_awgn(tx_signal, tx_samples_len, snr_db, rx_signal);

    /* ════════════════════ RECEIVER ═══════════════════════════ */
    printf("═══ RECEIVER ═══\n");

    /* 1. OFDM demodulate */
    Cplx *rx_data = (Cplx *)calloc(n_total_data, sizeof(Cplx));
    ofdm_demodulate_block(&ofdm, n_ofdm_sym, rx_signal, rx_data);
    printf("  OFDM demod: %d data symbols\n", n_total_data);

    /* 2. QPSK demodulate */
    uint8_t demod_bits[1024];
    mod_demodulate(MOD_QPSK, rx_data, nsyms, demod_bits);
    printf("  QPSK demod: %d bits\n", padded_len);

    /* 3. Deinterleave */
    uint8_t deinterleaved[1024];
    interleaver_deapply(&itl, demod_bits, deinterleaved, padded_len);
    printf("  Deinterleaved\n");
    interleaver_free(&itl);

    /* 4. Viterbi decode */
    uint8_t decoded_bits[MSG_BITS];
    viterbi_decode(deinterleaved, coded_len, decoded_bits);
    printf("  Viterbi decoded: %d bits\n", MSG_BITS);

    /* 5. Bits to bytes */
    uint8_t rx_message[MSG_BYTES];
    bytes_from_bits(decoded_bits, MSG_BITS, rx_message);
    printf("  Received: \"");
    for (int i = 0; i < MSG_BYTES; i++) printf("%c", rx_message[i]);
    printf("\"\n\n");

    /* ════════════════════ RESULTS ════════════════════════════ */
    printf("═══ RESULTS ═══\n");
    int bit_err = bit_errors(info_bits, decoded_bits, MSG_BITS);
    int byte_err = 0;
    for (int i = 0; i < MSG_BYTES; i++)
        if (message[i] != rx_message[i]) byte_err++;

    printf("  Bit errors:  %d / %d (BER = %.4e)\n", bit_err, MSG_BITS,
           (double)bit_err / MSG_BITS);
    printf("  Byte errors: %d / %d\n", byte_err, MSG_BYTES);
    printf("  Message %s\n", byte_err == 0 ? "INTACT ✓" : "CORRUPTED ✗");

    free(qpsk_syms); free(ofdm_data); free(tx_signal);
    free(rx_signal); free(rx_data);

    print_separator("End of Chapter 24 — Capstone Complete");
    return 0;
}
