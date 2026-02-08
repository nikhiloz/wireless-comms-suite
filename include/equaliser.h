/**
 * @file equaliser.h
 * @brief Channel equalisation — ZF, MMSE, LMS/RLS adaptive.
 *
 * Covers:
 *   - Zero-forcing (ZF) equaliser
 *   - Minimum mean-square error (MMSE)
 *   - LMS adaptive equaliser
 *   - RLS adaptive equaliser
 *   - Decision-feedback equaliser (DFE)
 */

#ifndef EQUALISER_H
#define EQUALISER_H

#include "comms_utils.h"

/* ── Zero-Forcing Equaliser ──────────────────────────────────────── */

/**
 * @brief ZF equalisation in frequency domain.
 * @param rx       Received signal (complex)
 * @param h        Channel impulse response (complex)
 * @param n        Number of samples
 * @param out      Equalised output
 */
void eq_zf_freq(const Cplx *rx, const Cplx *h, int n, Cplx *out);

/**
 * @brief ZF equalisation — single-tap (flat fading).
 */
void eq_zf_flat(const Cplx *rx, Cplx h, int n, Cplx *out);

/* ── MMSE Equaliser ──────────────────────────────────────────────── */

/**
 * @brief MMSE equalisation in frequency domain.
 * @param snr_linear  Signal-to-noise ratio (linear)
 */
void eq_mmse_freq(const Cplx *rx, const Cplx *h, int n,
                  double snr_linear, Cplx *out);

/* ── LMS Adaptive Equaliser ──────────────────────────────────────── */

typedef struct {
    int    n_taps;          /* number of equaliser taps              */
    Cplx  *w;              /* tap weights                           */
    Cplx  *buf;            /* delay line (circular buffer)          */
    int    idx;            /* write pointer                         */
    double mu;             /* step size                             */
} LmsEqualiser;

/**
 * @brief Initialise LMS equaliser.
 * @param eq      Struct
 * @param n_taps  Number of taps
 * @param mu      Step size (try 0.01)
 * @return 0 on success
 */
int eq_lms_init(LmsEqualiser *eq, int n_taps, double mu);
void eq_lms_free(LmsEqualiser *eq);

/**
 * @brief Process one sample through LMS equaliser (training mode).
 * @param eq       Equaliser state
 * @param input    New input sample
 * @param desired  Desired (training) symbol
 * @param error    Output: error = desired - output
 * @return Equaliser output
 */
Cplx eq_lms_step(LmsEqualiser *eq, Cplx input, Cplx desired, Cplx *error);

/**
 * @brief Process one sample (decision-directed, no training).
 */
Cplx eq_lms_dd_step(LmsEqualiser *eq, Cplx input, Cplx *error);

/* ── RLS Adaptive Equaliser ──────────────────────────────────────── */

typedef struct {
    int    n_taps;
    Cplx  *w;              /* tap weights                           */
    Cplx  *buf;            /* delay line                            */
    int    idx;
    double lambda;          /* forgetting factor (e.g. 0.99)        */
    double delta;           /* initialisation constant               */
    double *P;             /* inverse correlation matrix (flat)     */
} RlsEqualiser;

int  eq_rls_init(RlsEqualiser *eq, int n_taps, double lambda, double delta);
void eq_rls_free(RlsEqualiser *eq);
Cplx eq_rls_step(RlsEqualiser *eq, Cplx input, Cplx desired, Cplx *error);

/* ── Decision-Feedback Equaliser ─────────────────────────────────── */

typedef struct {
    LmsEqualiser ff;        /* feedforward filter                    */
    LmsEqualiser fb;        /* feedback filter                       */
} DfeEqualiser;

int  eq_dfe_init(DfeEqualiser *eq, int ff_taps, int fb_taps, double mu);
void eq_dfe_free(DfeEqualiser *eq);
Cplx eq_dfe_step(DfeEqualiser *eq, Cplx input, Cplx desired, Cplx *error);

#endif /* EQUALISER_H */
