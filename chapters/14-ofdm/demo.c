/**
 * @file demo.c
 * @brief Chapter 14 — OFDM System (FFT/IFFT TX/RX, cyclic prefix, pilots)
 *
 * Build:  make build/bin/14-ofdm
 * Run:    ./build/bin/14-ofdm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/ofdm.h"
#include "../../include/channel.h"

int main(void)
{
    rng_seed(14);
    print_separator("Chapter 14: OFDM System");

    OfdmParams ofdm;
    ofdm_init(&ofdm, 64, 16, 4);
    printf("  N_FFT=%d, N_CP=%d, N_DATA=%d, N_PILOT=%d\n\n",
           ofdm.n_fft, ofdm.n_cp, ofdm.n_data, ofdm.n_pilot);

    /* ── Single OFDM symbol (noiseless) ──────────────────────── */
    printf("1. Single OFDM Symbol (BPSK, no noise)\n");
    Cplx data_in[64];
    for (int i = 0; i < ofdm.n_data; i++)
        data_in[i] = (rng_uniform() > 0.5) ? cplx(1,0) : cplx(-1,0);

    Cplx time_sym[80];
    ofdm_modulate(&ofdm, data_in, time_sym);

    Cplx data_out[64];
    ofdm_demodulate(&ofdm, time_sym, data_out, NULL);

    double mse = 0;
    for (int i = 0; i < ofdm.n_data; i++)
        mse += cplx_mag2(cplx_sub(data_out[i], data_in[i]));
    mse /= ofdm.n_data;
    printf("   Round-trip MSE (noiseless): %.2e\n\n", mse);

    /* ── Multi-symbol OFDM with AWGN ─────────────────────────── */
    printf("2. 10-symbol OFDM block with AWGN (SNR=20 dB)\n");
    int n_ofdm = 10;
    int n_total_data = n_ofdm * ofdm.n_data;
    Cplx *block_data = (Cplx *)malloc(n_total_data * sizeof(Cplx));
    for (int i = 0; i < n_total_data; i++)
        block_data[i] = (rng_uniform() > 0.5) ? cplx(1,0) : cplx(-1,0);

    int n_samples = n_ofdm * (ofdm.n_fft + ofdm.n_cp);
    Cplx *tx_signal = (Cplx *)malloc(n_samples * sizeof(Cplx));
    ofdm_modulate_block(&ofdm, n_ofdm, block_data, tx_signal);

    Cplx *rx_signal = (Cplx *)malloc(n_samples * sizeof(Cplx));
    channel_awgn(tx_signal, n_samples, 20.0, rx_signal);

    Cplx *rx_data = (Cplx *)malloc(n_total_data * sizeof(Cplx));
    ofdm_demodulate_block(&ofdm, n_ofdm, rx_signal, rx_data);

    mse = 0;
    for (int i = 0; i < n_total_data; i++)
        mse += cplx_mag2(cplx_sub(rx_data[i], block_data[i]));
    mse /= n_total_data;
    printf("   MSE at 20 dB: %.4e\n", mse);

    /* BER */
    int errs = 0;
    for (int i = 0; i < n_total_data; i++) {
        int tx_bit = (block_data[i].re > 0) ? 0 : 1;
        int rx_bit = (rx_data[i].re > 0) ? 0 : 1;
        if (tx_bit != rx_bit) errs++;
    }
    printf("   BER: %.4e (%d/%d)\n", (double)errs / n_total_data,
           errs, n_total_data);

    free(block_data); free(tx_signal); free(rx_signal); free(rx_data);

    print_separator("End of Chapter 14");
    return 0;
}
