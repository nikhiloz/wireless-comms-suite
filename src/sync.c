/**
 * @file sync.c
 * @brief Synchronisation — timing recovery, carrier sync, frame sync.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Symbol timing   → chapters/08-timing-recovery/tutorial.md
 *   Carrier sync    → chapters/09-carrier-sync/tutorial.md
 *   Frame sync      → chapters/10-frame-sync/tutorial.md
 *
 * References:
 *   Gardner, "A BPSK/QPSK Timing Error Detector," IEEE Trans. 1986.
 *   Mueller & Muller, "Timing Recovery in Digital Synchronous Data Receivers," 1976.
 *   Costas, "Synchronous Communications," Proc. IRE, 1956.
 */

#include "../include/sync.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════════
 *  Barker codes
 * ════════════════════════════════════════════════════════════════════ */

const int BARKER_13[13] = {+1,+1,+1,+1,+1,-1,-1,+1,+1,-1,+1,-1,+1};
const int BARKER_11[11] = {+1,+1,+1,-1,-1,-1,+1,-1,-1,+1,-1};
const int BARKER_7[7]   = {+1,+1,+1,-1,-1,+1,-1};

/* ════════════════════════════════════════════════════════════════════
 *  Symbol timing recovery
 * ════════════════════════════════════════════════════════════════════ */

static void compute_loop_gains(double loop_bw, double damping,
                               double *ki, double *kp)
{
    /* Second-order loop filter design (based on normalised bandwidth) */
    double bw_n = loop_bw;
    double denom = 1.0 + 2.0 * damping * bw_n + bw_n * bw_n;
    *kp = 4.0 * damping * bw_n / denom;
    *ki = 4.0 * bw_n * bw_n / denom;
}

void timing_init(TimingRecovery *tr, int sps, double loop_bw, double damping)
{
    tr->sps = sps;
    tr->mu = 0;
    tr->mu_step = 1.0 / sps;
    tr->loop_bw = loop_bw;
    tr->damping = damping;
    tr->integrator = 0;
    compute_loop_gains(loop_bw, damping, &tr->ki, &tr->kp);
}

/* Simple linear interpolation */
static Cplx interp_linear(const Cplx *x, int idx, double mu)
{
    Cplx a = x[idx];
    Cplx b = x[idx + 1];
    return cplx_add(cplx_scale(a, 1.0 - mu), cplx_scale(b, mu));
}

int timing_recover_gardner(TimingRecovery *tr, const Cplx *in, int n, Cplx *out)
{
    int nsyms = 0;
    int sps = tr->sps;
    int half = sps / 2;

    /* Strobe-based: step through input at estimated symbol rate */
    int idx = 0;

    while (idx + sps + 1 < n) {
        /* Get on-time, early, and late samples */
        int on_idx = idx;
        int mid_idx = idx - half;
        if (mid_idx < 0) mid_idx = 0;

        Cplx on_time = (on_idx + 1 < n) ?
            interp_linear(in, on_idx, tr->mu) : in[on_idx];

        out[nsyms] = on_time;

        /* Gardner TED: e = Re{ (x[n-1/2] - x[n+1/2]) · conj(x[n]) }
         * Simplified: use real-part error for BPSK/QPSK */
        if (nsyms > 0 && mid_idx + 1 < n) {
            Cplx mid = interp_linear(in, mid_idx, tr->mu);
            double e = mid.re * (out[nsyms - 1].re - on_time.re) +
                       mid.im * (out[nsyms - 1].im - on_time.im);

            /* Loop filter */
            tr->integrator += tr->ki * e;
            double w = e * tr->kp + tr->integrator;
            tr->mu += w;
        }

        /* Advance by one symbol period */
        idx += sps;
        nsyms++;

        /* Keep mu in [0, 1) */
        while (tr->mu >= 1.0) { tr->mu -= 1.0; idx++; }
        while (tr->mu < 0.0)  { tr->mu += 1.0; idx--; }
    }

    return nsyms;
}

int timing_recover_mm(TimingRecovery *tr, const Cplx *in, int n, Cplx *out)
{
    int nsyms = 0;
    int sps = tr->sps;
    int idx = 0;
    Cplx prev_sym = cplx(0, 0);

    while (idx + 1 < n) {
        Cplx sym = interp_linear(in, idx, tr->mu);
        out[nsyms] = sym;

        if (nsyms > 0) {
            /* Mueller-Muller TED:
             * e = Re{ a[n-1]·conj(x[n]) - a[n]·conj(x[n-1]) }
             * where a[n] = hard decision on sym */
            Cplx a_curr = cplx(sym.re > 0 ? 1 : -1, sym.im > 0 ? 1 : -1);
            Cplx a_prev = cplx(prev_sym.re > 0 ? 1 : -1,
                               prev_sym.im > 0 ? 1 : -1);

            double e = a_prev.re * sym.re - a_curr.re * prev_sym.re +
                       a_prev.im * sym.im - a_curr.im * prev_sym.im;

            tr->integrator += tr->ki * e;
            double w = e * tr->kp + tr->integrator;
            tr->mu += w;
        }

        prev_sym = sym;
        idx += sps;
        nsyms++;

        while (tr->mu >= 1.0) { tr->mu -= 1.0; idx++; }
        while (tr->mu < 0.0)  { tr->mu += 1.0; idx--; }
    }

    return nsyms;
}

/* ════════════════════════════════════════════════════════════════════
 *  Carrier synchronisation
 * ════════════════════════════════════════════════════════════════════ */

void carrier_init(CarrierSync *cs, double loop_bw, double damping)
{
    cs->freq = 0;
    cs->phase = 0;
    cs->loop_bw = loop_bw;
    cs->damping = damping;

    double denom = 1.0 + 2.0 * damping * loop_bw + loop_bw * loop_bw;
    cs->alpha = 4.0 * damping * loop_bw / denom;
    cs->beta  = 4.0 * loop_bw * loop_bw / denom;
}

double carrier_costas_bpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        /* Derotate by current phase estimate */
        Cplx derot = cplx_mul(in[i], cplx_exp_j(-cs->phase));
        out[i] = derot;

        /* BPSK phase error: Im(x) * sign(Re(x)) */
        double error = derot.im * (derot.re > 0 ? 1.0 : -1.0);

        /* Update loop */
        cs->freq  += cs->beta * error;
        cs->phase += cs->freq + cs->alpha * error;

        /* Wrap phase */
        while (cs->phase >  M_PI) cs->phase -= 2.0 * M_PI;
        while (cs->phase < -M_PI) cs->phase += 2.0 * M_PI;
    }
    return cs->freq;
}

double carrier_costas_qpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        Cplx derot = cplx_mul(in[i], cplx_exp_j(-cs->phase));
        out[i] = derot;

        /* QPSK phase error: Im(x·conj(decision)) — 4th-power approach */
        double error = derot.re * (derot.im > 0 ? 1.0 : -1.0) -
                       derot.im * (derot.re > 0 ? 1.0 : -1.0);

        cs->freq  += cs->beta * error;
        cs->phase += cs->freq + cs->alpha * error;

        while (cs->phase >  M_PI) cs->phase -= 2.0 * M_PI;
        while (cs->phase < -M_PI) cs->phase += 2.0 * M_PI;
    }
    return cs->freq;
}

double carrier_pll(CarrierSync *cs, const Cplx *in, int n,
                   PhaseDetFunc pdet, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        Cplx derot = cplx_mul(in[i], cplx_exp_j(-cs->phase));
        out[i] = derot;

        double error = pdet ? pdet(derot, cs->phase) : atan2(derot.im, derot.re);

        cs->freq  += cs->beta * error;
        cs->phase += cs->freq + cs->alpha * error;

        while (cs->phase >  M_PI) cs->phase -= 2.0 * M_PI;
        while (cs->phase < -M_PI) cs->phase += 2.0 * M_PI;
    }
    return cs->freq;
}

/* ════════════════════════════════════════════════════════════════════
 *  Frame synchronisation
 * ════════════════════════════════════════════════════════════════════ */

int frame_sync_correlate(const double *signal, int sig_len,
                         const int *preamble, int pre_len,
                         double *corr)
{
    int n_corr = sig_len - pre_len + 1;
    double peak = 0;
    int peak_idx = 0;

    for (int i = 0; i < n_corr; i++) {
        double sum = 0;
        for (int j = 0; j < pre_len; j++)
            sum += signal[i + j] * preamble[j];
        corr[i] = sum;
        if (fabs(sum) > peak) {
            peak = fabs(sum);
            peak_idx = i;
        }
    }
    return peak_idx;
}

int frame_sync_detect(const double *signal, int sig_len,
                      const int *preamble, int pre_len,
                      double threshold)
{
    /* Normalised correlation */
    double pre_energy = 0;
    for (int j = 0; j < pre_len; j++)
        pre_energy += preamble[j] * preamble[j];

    for (int i = 0; i <= sig_len - pre_len; i++) {
        double sum = 0, sig_energy = 0;
        for (int j = 0; j < pre_len; j++) {
            sum += signal[i + j] * preamble[j];
            sig_energy += signal[i + j] * signal[i + j];
        }
        double norm = sqrt(sig_energy * pre_energy);
        if (norm > 1e-12 && fabs(sum) / norm >= threshold)
            return i;
    }
    return -1;
}

/* ════════════════════════════════════════════════════════════════════
 *  Scrambler (additive, LFSR-based)
 * ════════════════════════════════════════════════════════════════════ */

void scrambler(uint16_t poly, uint16_t init, uint8_t *bits, int n)
{
    uint16_t lfsr = init;
    for (int i = 0; i < n; i++) {
        /* Compute feedback from polynomial taps */
        uint16_t fb = 0;
        uint16_t tmp = lfsr & poly;
        while (tmp) { fb ^= tmp & 1; tmp >>= 1; }

        bits[i] ^= fb;

        lfsr = (lfsr << 1) | fb;
    }
}
