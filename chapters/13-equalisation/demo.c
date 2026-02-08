/**
 * @file demo.c
 * @brief Chapter 13 — Channel Equalisation (ZF, MMSE, Adaptive)
 *
 * Build:  make build/bin/13-equalisation
 * Run:    ./build/bin/13-equalisation
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"
#include "../../include/equaliser.h"
#include "../../include/ofdm.h"

#define N_SYMS 256

int main(void)
{
    rng_seed(13);
    print_separator("Chapter 13: Channel Equalisation");

    /* ── ZF frequency-domain equalisation ────────────────────── */
    printf("1. Zero-Forcing (Frequency Domain)\n");
    Cplx H[N_SYMS];
    for (int i = 0; i < N_SYMS; i++)
        H[i] = cplx(1.0 + 0.5 * cos(2.0 * M_PI * i / N_SYMS),
                     0.3 * sin(2.0 * M_PI * i / N_SYMS));

    Cplx tx[N_SYMS];
    uint8_t bits[N_SYMS];
    random_bits(bits, N_SYMS);
    mod_modulate(MOD_BPSK, bits, N_SYMS, tx);

    /* Channel distortion */
    Cplx rx[N_SYMS];
    for (int i = 0; i < N_SYMS; i++) rx[i] = cplx_mul(tx[i], H[i]);

    Cplx eq_zf[N_SYMS];
    eq_zf_freq(rx, H, N_SYMS, eq_zf);

    /* Check recovery */
    uint8_t dec[N_SYMS];
    mod_demodulate(MOD_BPSK, eq_zf, N_SYMS, dec);
    int errs = bit_errors(bits, dec, N_SYMS);
    printf("   BER after ZF eq: %.4f (%d errors)\n\n",
           (double)errs / N_SYMS, errs);

    /* ── LMS adaptive equaliser ──────────────────────────────── */
    printf("2. LMS Adaptive Equaliser (mu=0.01, 11 taps)\n");
    LmsEqualiser lms;
    eq_lms_init(&lms, 11, 0.01);

    /* Simple 2-tap channel: h = [1, 0.5] */
    printf("   Training on channel h = [1.0, 0.5]\n");
    double mse = 0;
    int count = 0;
    Cplx prev = cplx(0, 0);
    for (int i = 0; i < N_SYMS; i++) {
        Cplx ch_out = cplx_add(tx[i], cplx_scale(prev, 0.5));
        prev = tx[i];

        Cplx eq_out;
        eq_lms_step(&lms, ch_out, tx[i], &eq_out);

        if (i > N_SYMS / 2) {
            double e = cplx_mag2(cplx_sub(eq_out, tx[i]));
            mse += e;
            count++;
        }
    }
    mse /= count;
    printf("   Steady-state MSE: %.6f\n", mse);
    eq_lms_free(&lms);

    print_separator("End of Chapter 13");
    return 0;
}
