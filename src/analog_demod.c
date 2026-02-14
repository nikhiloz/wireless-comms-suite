/**
 * @file analog_demod.c
 * @brief Analog demodulation — FM, AM, SSB implementations.
 */

#include "analog_demod.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ── FM Modulate ─────────────────────────────────────────────────── */

int fm_modulate(const double *audio, int n, double freq_dev, Cplx *out)
{
    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        phase += 2.0 * M_PI * freq_dev * audio[i];
        out[i] = cplx_exp_j(phase);
    }
    return n;
}

/* ── FM Demodulate (discriminator) ───────────────────────────────── */

int fm_demodulate(const Cplx *iq, int n, double *out)
{
    /* Instantaneous freq via d/dt[arg(iq)] = arg(iq[i] * conj(iq[i-1])) */
    for (int i = 1; i < n; i++) {
        Cplx prod = cplx_mul(iq[i], cplx_conj(iq[i - 1]));
        out[i - 1] = atan2(prod.im, prod.re) / M_PI;  /* normalised ±1 */
    }
    return n - 1;
}

/* ── De-emphasis ─────────────────────────────────────────────────── */

void fm_deemphasis(const double *in, int n, double tau_us, double fs,
                   double *out)
{
    double tau = tau_us * 1.0e-6;
    double alpha = exp(-1.0 / (tau * fs));
    double gain = 1.0 - alpha;

    out[0] = gain * in[0];
    for (int i = 1; i < n; i++) {
        out[i] = gain * in[i] + alpha * out[i - 1];
    }
}

/* ── Pre-emphasis ────────────────────────────────────────────────── */

void fm_preemphasis(const double *in, int n, double tau_us, double fs,
                    double *out)
{
    double tau = tau_us * 1.0e-6;
    double alpha = exp(-1.0 / (tau * fs));
    double inv_gain = 1.0 / (1.0 - alpha);

    out[0] = inv_gain * in[0];
    for (int i = 1; i < n; i++) {
        out[i] = inv_gain * (in[i] - alpha * in[i - 1]);
    }
}

/* ── Stereo pilot detection ──────────────────────────────────────── */

double fm_stereo_pilot_detect(const double *in, int n, double fs)
{
    /* Goertzel algorithm tuned to 19 kHz */
    double f_pilot = 19000.0;
    double k = (double)((int)(0.5 + (double)n * f_pilot / fs));
    double w = 2.0 * M_PI * k / (double)n;
    double coeff = 2.0 * cos(w);
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < n; i++) {
        double s0 = in[i] + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }

    double power = (s1 * s1 + s2 * s2 - coeff * s1 * s2) / ((double)n * n);
    double total_pwr = 0.0;
    for (int i = 0; i < n; i++) total_pwr += in[i] * in[i];
    total_pwr /= (double)n;

    return (total_pwr > 1.0e-12) ? sqrt(fabs(power) / total_pwr) : 0.0;
}

/* ── Stereo decode ───────────────────────────────────────────────── */

int fm_stereo_decode(const double *composite, int n, double fs,
                     double *left, double *right)
{
    double pilot_str = fm_stereo_pilot_detect(composite, n, fs);
    if (pilot_str < 0.05) return -1;   /* no pilot */

    double f_pilot = 19000.0;
    double norm = 2.0 * M_PI * f_pilot / fs;

    /* Recover phase of 19 kHz pilot via correlation */
    double sin_sum = 0.0, cos_sum = 0.0;
    for (int i = 0; i < n; i++) {
        sin_sum += composite[i] * sin(norm * i);
        cos_sum += composite[i] * cos(norm * i);
    }
    double pilot_phase = atan2(sin_sum, cos_sum);

    /* L+R is baseband, L-R is at 38 kHz sub-carrier */
    for (int i = 0; i < n; i++) {
        double mono = composite[i];
        double diff = composite[i] * 2.0 *
                      cos(2.0 * norm * i + 2.0 * pilot_phase);
        left[i]  = (mono + diff) * 0.5;
        right[i] = (mono - diff) * 0.5;
    }
    return 0;
}

/* ── AM Modulate ─────────────────────────────────────────────────── */

int am_modulate(const double *audio, int n, double mod_idx,
                double fc_norm, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        double env = 1.0 + mod_idx * audio[i];
        double theta = 2.0 * M_PI * fc_norm * i;
        out[i] = cplx_scale(cplx_exp_j(theta), env);
    }
    return n;
}

/* ── AM Envelope Detection ───────────────────────────────────────── */

int am_envelope_detect(const Cplx *iq, int n, double *out)
{
    for (int i = 0; i < n; i++) {
        out[i] = cplx_mag(iq[i]);
    }
    /* Remove DC (average carrier level) */
    double dc = 0.0;
    for (int i = 0; i < n; i++) dc += out[i];
    dc /= (double)n;
    for (int i = 0; i < n; i++) out[i] -= dc;
    return n;
}

/* ── AM Coherent Demod ───────────────────────────────────────────── */

int am_coherent_demod(const Cplx *iq, int n, double fc_norm, double *out)
{
    for (int i = 0; i < n; i++) {
        double theta = 2.0 * M_PI * fc_norm * i;
        Cplx lo = cplx_exp_j(-theta);
        Cplx prod = cplx_mul(iq[i], lo);
        out[i] = prod.re;
    }
    /* Remove DC */
    double dc = 0.0;
    for (int i = 0; i < n; i++) dc += out[i];
    dc /= (double)n;
    for (int i = 0; i < n; i++) out[i] -= dc;
    return n;
}

/* ── SSB Modulate (Hilbert method) ───────────────────────────────── */

int ssb_modulate(const double *audio, int n, int upper,
                 double fc_norm, Cplx *out)
{
    /* FIR Hilbert transform (31-tap) */
    int htaps = 31;
    int half = htaps / 2;
    double *hilbert = (double *)calloc((size_t)n, sizeof(double));
    if (!hilbert) return 0;

    for (int i = half; i < n - half; i++) {
        double sum = 0.0;
        for (int k = -half; k <= half; k++) {
            if (k == 0) continue;
            if (k % 2 != 0) {  /* odd taps only for Hilbert */
                double coeff = 2.0 / (M_PI * k);
                /* Hamming window */
                double w = 0.54 - 0.46 *
                           cos(2.0 * M_PI * (k + half) / (htaps - 1));
                sum += audio[i - k] * coeff * w;
            }
        }
        hilbert[i] = sum;
    }

    double sign = upper ? 1.0 : -1.0;
    for (int i = 0; i < n; i++) {
        double theta = 2.0 * M_PI * fc_norm * i;
        Cplx bb = cplx(audio[i], sign * hilbert[i]);
        Cplx carrier = cplx_exp_j(theta);
        out[i] = cplx_mul(bb, carrier);
    }

    free(hilbert);
    return n;
}

/* ── SSB Demodulate ──────────────────────────────────────────────── */

int ssb_demodulate(const Cplx *iq, int n, double fc_norm, double *out)
{
    /* Product detector: multiply by carrier, take real part */
    for (int i = 0; i < n; i++) {
        double theta = 2.0 * M_PI * fc_norm * i;
        Cplx lo = cplx_exp_j(-theta);
        Cplx prod = cplx_mul(iq[i], lo);
        out[i] = prod.re;
    }
    return n;
}

/* ── Low-pass FIR filter ─────────────────────────────────────────── */

void lowpass_fir(const double *in, int n, double fc, int taps, double *out)
{
    if (taps < 1) taps = 1;
    if (taps % 2 == 0) taps++;     /* ensure odd */
    int half = taps / 2;

    /* Generate windowed-sinc coefficients */
    double *h = (double *)calloc((size_t)taps, sizeof(double));
    if (!h) { memcpy(out, in, (size_t)n * sizeof(double)); return; }

    double sum = 0.0;
    for (int k = 0; k < taps; k++) {
        int m = k - half;
        if (m == 0) {
            h[k] = 2.0 * fc;
        } else {
            h[k] = sin(2.0 * M_PI * fc * m) / (M_PI * m);
        }
        /* Hamming window */
        double w = 0.54 - 0.46 * cos(2.0 * M_PI * k / (taps - 1));
        h[k] *= w;
        sum += h[k];
    }
    /* Normalise */
    for (int k = 0; k < taps; k++) h[k] /= sum;

    /* Apply convolution, delay-compensated */
    for (int i = 0; i < n; i++) {
        double acc = 0.0;
        for (int k = 0; k < taps; k++) {
            int idx = i - k + half;
            if (idx >= 0 && idx < n) {
                acc += in[idx] * h[k];
            }
        }
        out[i] = acc;
    }
    free(h);
}