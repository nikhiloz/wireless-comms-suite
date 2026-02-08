/**
 * @file equaliser.c
 * @brief Channel equalisation — ZF, MMSE, LMS/RLS adaptive, DFE.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Equalisation       → chapters/13-equalisation/tutorial.md
 *
 * References:
 *   Haykin, Adaptive Filter Theory (5th ed.), Ch. 9–13.
 *   Proakis & Salehi, Digital Communications (5th ed.), Ch. 10.
 */

#include "../include/equaliser.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ════════════════════════════════════════════════════════════════════
 *  Zero-Forcing equaliser
 * ════════════════════════════════════════════════════════════════════ */

void eq_zf_freq(const Cplx *rx, const Cplx *h, int n, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        double hmag2 = cplx_mag2(h[i]);
        if (hmag2 < 1e-12) hmag2 = 1e-12;
        out[i] = cplx_scale(cplx_mul(rx[i], cplx_conj(h[i])), 1.0 / hmag2);
    }
}

void eq_zf_flat(const Cplx *rx, Cplx h, int n, Cplx *out)
{
    double hmag2 = cplx_mag2(h);
    if (hmag2 < 1e-12) hmag2 = 1e-12;
    Cplx h_inv = cplx_scale(cplx_conj(h), 1.0 / hmag2);
    for (int i = 0; i < n; i++)
        out[i] = cplx_mul(rx[i], h_inv);
}

/* ════════════════════════════════════════════════════════════════════
 *  MMSE equaliser
 * ════════════════════════════════════════════════════════════════════ */

void eq_mmse_freq(const Cplx *rx, const Cplx *h, int n,
                  double snr_linear, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        double hmag2 = cplx_mag2(h[i]);
        double denom = hmag2 + 1.0 / snr_linear;
        if (denom < 1e-12) denom = 1e-12;
        out[i] = cplx_scale(cplx_mul(rx[i], cplx_conj(h[i])), 1.0 / denom);
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  LMS adaptive equaliser
 * ════════════════════════════════════════════════════════════════════ */

int eq_lms_init(LmsEqualiser *eq, int n_taps, double mu)
{
    eq->n_taps = n_taps;
    eq->mu = mu;
    eq->idx = 0;
    eq->w   = (Cplx *)calloc(n_taps, sizeof(Cplx));
    eq->buf = (Cplx *)calloc(n_taps, sizeof(Cplx));
    if (!eq->w || !eq->buf) return -1;

    /* Centre-tap initialisation */
    eq->w[n_taps / 2] = cplx(1.0, 0.0);
    return 0;
}

void eq_lms_free(LmsEqualiser *eq)
{
    free(eq->w);  eq->w = NULL;
    free(eq->buf); eq->buf = NULL;
}

static Cplx lms_filter_output(const LmsEqualiser *eq)
{
    /* y = w^H · x (dot product) */
    Cplx y = cplx(0, 0);
    for (int k = 0; k < eq->n_taps; k++) {
        int idx = (eq->idx - k + eq->n_taps) % eq->n_taps;
        y = cplx_add(y, cplx_mul(cplx_conj(eq->w[k]), eq->buf[idx]));
    }
    return y;
}

Cplx eq_lms_step(LmsEqualiser *eq, Cplx input, Cplx desired, Cplx *error)
{
    /* Insert new sample */
    eq->buf[eq->idx] = input;

    /* Compute output */
    Cplx y = lms_filter_output(eq);

    /* Error */
    Cplx e = cplx_sub(desired, y);
    if (error) *error = e;

    /* Update weights: w = w + mu * e * conj(x) */
    for (int k = 0; k < eq->n_taps; k++) {
        int idx = (eq->idx - k + eq->n_taps) % eq->n_taps;
        Cplx update = cplx_scale(cplx_mul(e, cplx_conj(eq->buf[idx])), eq->mu);
        eq->w[k] = cplx_add(eq->w[k], update);
    }

    eq->idx = (eq->idx + 1) % eq->n_taps;
    return y;
}

Cplx eq_lms_dd_step(LmsEqualiser *eq, Cplx input, Cplx *error)
{
    eq->buf[eq->idx] = input;
    Cplx y = lms_filter_output(eq);

    /* Decision-directed: use hard decision as desired */
    Cplx decision = cplx(y.re > 0 ? 1.0 : -1.0, y.im > 0 ? 1.0 : -1.0);
    Cplx e = cplx_sub(decision, y);
    if (error) *error = e;

    for (int k = 0; k < eq->n_taps; k++) {
        int idx = (eq->idx - k + eq->n_taps) % eq->n_taps;
        Cplx update = cplx_scale(cplx_mul(e, cplx_conj(eq->buf[idx])), eq->mu);
        eq->w[k] = cplx_add(eq->w[k], update);
    }

    eq->idx = (eq->idx + 1) % eq->n_taps;
    return y;
}

/* ════════════════════════════════════════════════════════════════════
 *  RLS adaptive equaliser
 * ════════════════════════════════════════════════════════════════════ */

int eq_rls_init(RlsEqualiser *eq, int n_taps, double lambda, double delta)
{
    eq->n_taps = n_taps;
    eq->lambda = lambda;
    eq->delta = delta;
    eq->idx = 0;
    eq->w   = (Cplx *)calloc(n_taps, sizeof(Cplx));
    eq->buf = (Cplx *)calloc(n_taps, sizeof(Cplx));
    eq->P   = (double *)calloc(n_taps * n_taps, sizeof(double));
    if (!eq->w || !eq->buf || !eq->P) return -1;

    /* Initialise P = delta * I */
    for (int i = 0; i < n_taps; i++)
        eq->P[i * n_taps + i] = delta;

    eq->w[n_taps / 2] = cplx(1.0, 0.0);
    return 0;
}

void eq_rls_free(RlsEqualiser *eq)
{
    free(eq->w);   eq->w = NULL;
    free(eq->buf); eq->buf = NULL;
    free(eq->P);   eq->P = NULL;
}

Cplx eq_rls_step(RlsEqualiser *eq, Cplx input, Cplx desired, Cplx *error)
{
    int N = eq->n_taps;
    eq->buf[eq->idx] = input;

    /* Output: y = w^H · x */
    Cplx y = cplx(0, 0);
    for (int k = 0; k < N; k++) {
        int idx = (eq->idx - k + N) % N;
        y = cplx_add(y, cplx_mul(cplx_conj(eq->w[k]), eq->buf[idx]));
    }

    Cplx e = cplx_sub(desired, y);
    if (error) *error = e;

    /* Simplified RLS update using scalar gain (for real-valued P) */
    /* k = P·x / (lambda + x^H·P·x) */
    double *Px = (double *)calloc(N, sizeof(double));
    double xPx = 0;
    for (int i = 0; i < N; i++) {
        Px[i] = 0;
        for (int j = 0; j < N; j++) {
            int jdx = (eq->idx - j + N) % N;
            Px[i] += eq->P[i * N + j] * cplx_mag(eq->buf[jdx]);
        }
        int idx = (eq->idx - i + N) % N;
        xPx += cplx_mag(eq->buf[idx]) * Px[i];
    }

    double denom = eq->lambda + xPx;
    if (fabs(denom) < 1e-12) denom = 1e-12;

    /* Update weights */
    for (int i = 0; i < N; i++) {
        double gain = Px[i] / denom;
        eq->w[i] = cplx_add(eq->w[i], cplx_scale(e, gain));
    }

    /* Update P = (1/lambda) * (P - k·x^H·P) — simplified */
    double inv_lambda = 1.0 / eq->lambda;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            double ki = Px[i] / denom;
            eq->P[i * N + j] = inv_lambda * (eq->P[i * N + j] - ki * Px[j]);
        }
    }

    free(Px);
    eq->idx = (eq->idx + 1) % N;
    return y;
}

/* ════════════════════════════════════════════════════════════════════
 *  Decision-Feedback Equaliser (DFE)
 * ════════════════════════════════════════════════════════════════════ */

int eq_dfe_init(DfeEqualiser *eq, int ff_taps, int fb_taps, double mu)
{
    int r1 = eq_lms_init(&eq->ff, ff_taps, mu);
    int r2 = eq_lms_init(&eq->fb, fb_taps, mu);
    return (r1 || r2) ? -1 : 0;
}

void eq_dfe_free(DfeEqualiser *eq)
{
    eq_lms_free(&eq->ff);
    eq_lms_free(&eq->fb);
}

Cplx eq_dfe_step(DfeEqualiser *eq, Cplx input, Cplx desired, Cplx *error)
{
    /* Feedforward output */
    eq->ff.buf[eq->ff.idx] = input;
    Cplx y_ff = cplx(0, 0);
    for (int k = 0; k < eq->ff.n_taps; k++) {
        int idx = (eq->ff.idx - k + eq->ff.n_taps) % eq->ff.n_taps;
        y_ff = cplx_add(y_ff, cplx_mul(cplx_conj(eq->ff.w[k]),
                                        eq->ff.buf[idx]));
    }

    /* Feedback output */
    Cplx y_fb = cplx(0, 0);
    for (int k = 0; k < eq->fb.n_taps; k++) {
        int idx = (eq->fb.idx - k + eq->fb.n_taps) % eq->fb.n_taps;
        y_fb = cplx_add(y_fb, cplx_mul(cplx_conj(eq->fb.w[k]),
                                        eq->fb.buf[idx]));
    }

    Cplx y = cplx_add(y_ff, y_fb);
    Cplx e = cplx_sub(desired, y);
    if (error) *error = e;

    /* Update feedforward weights */
    for (int k = 0; k < eq->ff.n_taps; k++) {
        int idx = (eq->ff.idx - k + eq->ff.n_taps) % eq->ff.n_taps;
        Cplx upd = cplx_scale(cplx_mul(e, cplx_conj(eq->ff.buf[idx])),
                               eq->ff.mu);
        eq->ff.w[k] = cplx_add(eq->ff.w[k], upd);
    }

    /* Insert decision into feedback buffer */
    Cplx decision = cplx(y.re > 0 ? 1 : -1, y.im > 0 ? 1 : -1);
    eq->fb.buf[eq->fb.idx] = decision;

    /* Update feedback weights */
    for (int k = 0; k < eq->fb.n_taps; k++) {
        int idx = (eq->fb.idx - k + eq->fb.n_taps) % eq->fb.n_taps;
        Cplx upd = cplx_scale(cplx_mul(e, cplx_conj(eq->fb.buf[idx])),
                               eq->fb.mu);
        eq->fb.w[k] = cplx_add(eq->fb.w[k], upd);
    }

    eq->ff.idx = (eq->ff.idx + 1) % eq->ff.n_taps;
    eq->fb.idx = (eq->fb.idx + 1) % eq->fb.n_taps;
    return y;
}
