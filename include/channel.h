/**
 * @file channel.h
 * @brief Channel models — AWGN, Rayleigh, Rician, multipath.
 *
 * Provides:
 *   - AWGN additive noise (complex and real)
 *   - Rayleigh flat-fading channel
 *   - Rician fading channel (with K-factor)
 *   - Tapped delay-line multipath channel
 *   - Doppler shift simulation
 *   - SNR / Eb/N0 conversions
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include "comms_utils.h"

/* ── AWGN ────────────────────────────────────────────────────────── */

/**
 * @brief Add AWGN to complex signal.
 * @param in       Input signal
 * @param n        Number of samples
 * @param snr_db   Desired SNR in dB
 * @param out      Output (may alias in)
 * @return Actual noise variance (sigma^2)
 */
double channel_awgn(const Cplx *in, int n, double snr_db, Cplx *out);

/** Real-valued AWGN. */
double channel_awgn_real(const double *in, int n, double snr_db, double *out);

/* ── Eb/N0 ↔ SNR conversion ─────────────────────────────────────── */

/** SNR(dB) = Eb/N0(dB) + 10*log10(bits_per_sym * code_rate / oversample). */
double ebn0_to_snr(double ebn0_db, int bits_per_sym, double code_rate,
                   int samples_per_sym);
double snr_to_ebn0(double snr_db, int bits_per_sym, double code_rate,
                   int samples_per_sym);

/* ── Rayleigh fading ─────────────────────────────────────────────── */

typedef struct {
    double sigma;        /* Rayleigh parameter (RMS / sqrt(2))      */
    Cplx   last_coeff;   /* channel coefficient for flat fading      */
} RayleighChannel;

/** Flat Rayleigh fade: multiply each sample by a complex gain. */
void channel_rayleigh_flat(RayleighChannel *ch, const Cplx *in, int n,
                           Cplx *out, Cplx *h_est);

/** Generate n independent Rayleigh fading coefficients. */
void channel_rayleigh_gen(int n, Cplx *coeffs);

/* ── Rician fading ───────────────────────────────────────────────── */

typedef struct {
    double k_factor;     /* Rician K = |LOS|^2 / (2 * sigma^2)      */
    double los_phase;    /* LOS component phase (radians)            */
} RicianChannel;

void channel_rician_flat(RicianChannel *ch, const Cplx *in, int n,
                         Cplx *out, Cplx *h_est);

/* ── Multipath (tapped delay line) ───────────────────────────────── */

#define MULTIPATH_MAX_TAPS 32

typedef struct {
    int    n_taps;
    int    delays[MULTIPATH_MAX_TAPS];    /* delay in samples       */
    double gains_db[MULTIPATH_MAX_TAPS];  /* tap gain in dB         */
    Cplx   coeffs[MULTIPATH_MAX_TAPS];    /* complex coefficients   */
} MultipathChannel;

/**
 * @brief Initialise multipath channel from a power-delay profile.
 * @param ch       Channel struct to populate
 * @param n_taps   Number of taps
 * @param delays   Delay per tap (samples)
 * @param gains_db Power per tap (dB, 0 dB = strongest)
 */
void channel_multipath_init(MultipathChannel *ch, int n_taps,
                            const int *delays, const double *gains_db);

/**
 * @brief Apply multipath channel to signal.
 * @param out_len  Receives output length (= n + max_delay)
 */
void channel_multipath_apply(const MultipathChannel *ch,
                             const Cplx *in, int n,
                             Cplx *out, int *out_len);

/* ── Doppler ─────────────────────────────────────────────────────── */

/**
 * @brief Apply frequency (Doppler) shift to signal.
 * @param fd   Normalised Doppler freq (fd / fs)
 */
void channel_doppler(const Cplx *in, int n, double fd, Cplx *out);

/* ── Signal power measurement ────────────────────────────────────── */

double signal_power(const Cplx *x, int n);
double signal_power_real(const double *x, int n);
double compute_snr_db(const Cplx *signal, const Cplx *noisy, int n);

#endif /* CHANNEL_H */
