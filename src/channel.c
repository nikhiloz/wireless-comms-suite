/**
 * @file channel.c
 * @brief Channel models — AWGN, Rayleigh, Rician, multipath, Doppler.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   AWGN channel      → chapters/06-awgn-channel/tutorial.md
 *   Fading channels   → chapters/07-fading-channels/tutorial.md
 *
 * References:
 *   Rappaport, Wireless Communications (2nd ed.), Ch. 4–5.
 *   Sklar, Digital Communications, Ch. 10.
 */

#include "../include/channel.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ════════════════════════════════════════════════════════════════════
 *  Signal power measurement
 * ════════════════════════════════════════════════════════════════════ */

double signal_power(const Cplx *x, int n)
{
    double p = 0;
    for (int i = 0; i < n; i++)
        p += cplx_mag2(x[i]);
    return p / n;
}

double signal_power_real(const double *x, int n)
{
    double p = 0;
    for (int i = 0; i < n; i++)
        p += x[i] * x[i];
    return p / n;
}

double compute_snr_db(const Cplx *signal, const Cplx *noisy, int n)
{
    double sig_pow = 0, noise_pow = 0;
    for (int i = 0; i < n; i++) {
        sig_pow += cplx_mag2(signal[i]);
        Cplx diff = cplx_sub(noisy[i], signal[i]);
        noise_pow += cplx_mag2(diff);
    }
    if (noise_pow < 1e-30) return 100.0;
    return 10.0 * log10(sig_pow / noise_pow);
}

/* ════════════════════════════════════════════════════════════════════
 *  AWGN channel
 * ════════════════════════════════════════════════════════════════════ */

double channel_awgn(const Cplx *in, int n, double snr_db, Cplx *out)
{
    /* Compute signal power */
    double sig_pow = signal_power(in, n);
    if (sig_pow < 1e-30) sig_pow = 1.0;

    /* Compute noise variance from SNR */
    double snr_lin = pow(10.0, snr_db / 10.0);
    double noise_var = sig_pow / snr_lin;
    double sigma = sqrt(noise_var / 2.0); /* per dimension */

    for (int i = 0; i < n; i++) {
        out[i].re = in[i].re + sigma * rng_gaussian();
        out[i].im = in[i].im + sigma * rng_gaussian();
    }
    return noise_var;
}

double channel_awgn_real(const double *in, int n, double snr_db, double *out)
{
    double sig_pow = signal_power_real(in, n);
    if (sig_pow < 1e-30) sig_pow = 1.0;

    double snr_lin = pow(10.0, snr_db / 10.0);
    double noise_var = sig_pow / snr_lin;
    double sigma = sqrt(noise_var);

    for (int i = 0; i < n; i++)
        out[i] = in[i] + sigma * rng_gaussian();

    return noise_var;
}

/* ════════════════════════════════════════════════════════════════════
 *  Eb/N0 ↔ SNR conversion
 * ════════════════════════════════════════════════════════════════════ */

double ebn0_to_snr(double ebn0_db, int bits_per_sym, double code_rate,
                   int samples_per_sym)
{
    return ebn0_db + 10.0 * log10((double)bits_per_sym * code_rate /
                                  (double)samples_per_sym);
}

double snr_to_ebn0(double snr_db, int bits_per_sym, double code_rate,
                   int samples_per_sym)
{
    return snr_db - 10.0 * log10((double)bits_per_sym * code_rate /
                                 (double)samples_per_sym);
}

/* ════════════════════════════════════════════════════════════════════
 *  Rayleigh fading
 * ════════════════════════════════════════════════════════════════════ */

void channel_rayleigh_gen(int n, Cplx *coeffs)
{
    double sigma = 1.0 / sqrt(2.0); /* unit average power */
    for (int i = 0; i < n; i++)
        coeffs[i] = cplx(sigma * rng_gaussian(), sigma * rng_gaussian());
}

void channel_rayleigh_flat(RayleighChannel *ch, const Cplx *in, int n,
                           Cplx *out, Cplx *h_est)
{
    /* Generate one fading coefficient per block */
    ch->last_coeff = cplx(ch->sigma * rng_gaussian(),
                          ch->sigma * rng_gaussian());
    if (h_est) *h_est = ch->last_coeff;

    for (int i = 0; i < n; i++)
        out[i] = cplx_mul(in[i], ch->last_coeff);
}

/* ════════════════════════════════════════════════════════════════════
 *  Rician fading
 * ════════════════════════════════════════════════════════════════════ */

void channel_rician_flat(RicianChannel *ch, const Cplx *in, int n,
                         Cplx *out, Cplx *h_est)
{
    double K = ch->k_factor;
    double los_gain = sqrt(K / (K + 1.0));
    double nlos_sigma = sqrt(1.0 / (2.0 * (K + 1.0)));

    /* LOS component */
    Cplx los = cplx_from_polar(los_gain, ch->los_phase);

    /* Scattered (NLOS) component */
    Cplx nlos = cplx(nlos_sigma * rng_gaussian(),
                     nlos_sigma * rng_gaussian());

    Cplx h = cplx_add(los, nlos);
    if (h_est) *h_est = h;

    for (int i = 0; i < n; i++)
        out[i] = cplx_mul(in[i], h);
}

/* ════════════════════════════════════════════════════════════════════
 *  Multipath (tapped delay line)
 * ════════════════════════════════════════════════════════════════════ */

void channel_multipath_init(MultipathChannel *ch, int n_taps,
                            const int *delays, const double *gains_db)
{
    ch->n_taps = (n_taps > MULTIPATH_MAX_TAPS) ? MULTIPATH_MAX_TAPS : n_taps;
    for (int i = 0; i < ch->n_taps; i++) {
        ch->delays[i] = delays[i];
        ch->gains_db[i] = gains_db[i];

        /* Random complex coefficient with given power */
        double gain_lin = pow(10.0, gains_db[i] / 20.0);
        double sigma = gain_lin / sqrt(2.0);
        ch->coeffs[i] = cplx(sigma * rng_gaussian(), sigma * rng_gaussian());
    }
}

void channel_multipath_apply(const MultipathChannel *ch,
                             const Cplx *in, int n,
                             Cplx *out, int *out_len)
{
    int max_delay = 0;
    for (int t = 0; t < ch->n_taps; t++)
        if (ch->delays[t] > max_delay) max_delay = ch->delays[t];

    *out_len = n + max_delay;
    memset(out, 0, (*out_len) * sizeof(Cplx));

    for (int t = 0; t < ch->n_taps; t++) {
        int d = ch->delays[t];
        Cplx c = ch->coeffs[t];
        for (int i = 0; i < n; i++) {
            Cplx prod = cplx_mul(in[i], c);
            out[i + d] = cplx_add(out[i + d], prod);
        }
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Doppler shift
 * ════════════════════════════════════════════════════════════════════ */

void channel_doppler(const Cplx *in, int n, double fd, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        Cplx shift = cplx_exp_j(2.0 * M_PI * fd * i);
        out[i] = cplx_mul(in[i], shift);
    }
}
