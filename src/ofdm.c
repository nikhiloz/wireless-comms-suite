/**
 * @file ofdm.c
 * @brief OFDM system — FFT/IFFT TX/RX, cyclic prefix, pilot equalisation.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   OFDM system       → chapters/14-ofdm-system/tutorial.md
 *   Wi-Fi PHY         → chapters/16-wifi-phy/tutorial.md
 *
 * References:
 *   Proakis & Salehi, Digital Communications (5th ed.), Ch. 12.
 *   IEEE 802.11-2020.
 */

#include "../include/ofdm.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════════
 *  FFT (radix-2 DIT, in-place)
 * ════════════════════════════════════════════════════════════════════ */

static void bit_reverse(Cplx *x, int n)
{
    int bits = 0;
    for (int tmp = n; tmp > 1; tmp >>= 1) bits++;

    for (int i = 0; i < n; i++) {
        int j = 0;
        for (int b = 0; b < bits; b++)
            if (i & (1 << b)) j |= 1 << (bits - 1 - b);
        if (j > i) {
            Cplx tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
    }
}

void fft(Cplx *x, int n)
{
    bit_reverse(x, n);
    for (int size = 2; size <= n; size *= 2) {
        int half = size / 2;
        double angle = -2.0 * M_PI / size;
        Cplx w_step = cplx_exp_j(angle);
        for (int start = 0; start < n; start += size) {
            Cplx w = cplx(1.0, 0.0);
            for (int k = 0; k < half; k++) {
                Cplx even = x[start + k];
                Cplx odd  = cplx_mul(w, x[start + k + half]);
                x[start + k]        = cplx_add(even, odd);
                x[start + k + half] = cplx_sub(even, odd);
                w = cplx_mul(w, w_step);
            }
        }
    }
}

void ifft(Cplx *x, int n)
{
    /* Conjugate, FFT, conjugate, scale by 1/N */
    for (int i = 0; i < n; i++) x[i] = cplx_conj(x[i]);
    fft(x, n);
    for (int i = 0; i < n; i++) {
        x[i] = cplx_conj(x[i]);
        x[i] = cplx_scale(x[i], 1.0 / n);
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  OFDM parameter initialisation
 * ════════════════════════════════════════════════════════════════════ */

void ofdm_init(OfdmParams *p, int n_fft, int n_cp, int n_pilot)
{
    memset(p, 0, sizeof(*p));
    p->n_fft = n_fft;
    p->n_cp = n_cp;
    p->n_pilot = n_pilot;
    p->pilot_val = cplx(1.0, 0.0);

    /* Guard bands: DC null + edge guards */
    p->n_guard_lo = n_fft / 8;
    p->n_guard_hi = n_fft / 8;

    int usable = n_fft - p->n_guard_lo - p->n_guard_hi - 1; /* -1 for DC */

    /* Place pilots evenly among usable subcarriers */
    if (n_pilot > 0 && n_pilot < usable) {
        int spacing = usable / (n_pilot + 1);
        for (int i = 0; i < n_pilot; i++) {
            p->pilot_idx[i] = p->n_guard_lo + 1 + (i + 1) * spacing;
        }
    }
    p->n_pilot = n_pilot;

    /* Data subcarriers = everything else in usable range */
    int d = 0;
    for (int k = p->n_guard_lo + 1; k < n_fft - p->n_guard_hi; k++) {
        if (k == n_fft / 2) continue; /* DC null */
        int is_pilot = 0;
        for (int pi = 0; pi < n_pilot; pi++) {
            if (k == p->pilot_idx[pi]) { is_pilot = 1; break; }
        }
        if (!is_pilot)
            p->data_idx[d++] = k;
    }
    p->n_data = d;
}

/* ════════════════════════════════════════════════════════════════════
 *  OFDM TX: symbol mapping → IFFT → CP insertion
 * ════════════════════════════════════════════════════════════════════ */

int ofdm_modulate(const OfdmParams *p, const Cplx *data_syms, Cplx *out)
{
    int n = p->n_fft;
    Cplx *freq = (Cplx *)calloc(n, sizeof(Cplx));

    /* Map data to subcarriers */
    for (int i = 0; i < p->n_data; i++)
        freq[p->data_idx[i]] = data_syms[i];

    /* Insert pilots */
    for (int i = 0; i < p->n_pilot; i++)
        freq[p->pilot_idx[i]] = p->pilot_val;

    /* IFFT → time domain */
    ifft(freq, n);

    /* Copy with cyclic prefix */
    memcpy(out, freq + n - p->n_cp, p->n_cp * sizeof(Cplx));
    memcpy(out + p->n_cp, freq, n * sizeof(Cplx));

    free(freq);
    return n + p->n_cp;
}

int ofdm_modulate_block(const OfdmParams *p, int n_symbols,
                        const Cplx *data_syms, Cplx *out)
{
    int total = 0;
    int sym_size = p->n_fft + p->n_cp;
    for (int s = 0; s < n_symbols; s++) {
        ofdm_modulate(p, data_syms + s * p->n_data, out + s * sym_size);
        total += sym_size;
    }
    return total;
}

/* ════════════════════════════════════════════════════════════════════
 *  OFDM RX: CP removal → FFT → extract data
 * ════════════════════════════════════════════════════════════════════ */

int ofdm_demodulate(const OfdmParams *p, const Cplx *in,
                    Cplx *data_syms, Cplx *h_est)
{
    int n = p->n_fft;

    /* Remove CP, copy to work buffer */
    Cplx *freq = (Cplx *)calloc(n, sizeof(Cplx));
    memcpy(freq, in + p->n_cp, n * sizeof(Cplx));

    /* FFT → frequency domain */
    fft(freq, n);

    /* Channel estimation via pilots */
    Cplx h_interp[OFDM_MAX_CARRIERS];
    if (p->n_pilot > 0) {
        ofdm_channel_estimate(p, freq, h_interp);
    } else {
        for (int i = 0; i < p->n_data; i++)
            h_interp[i] = cplx(1.0, 0.0);
    }

    /* Extract and equalise data */
    for (int i = 0; i < p->n_data; i++) {
        Cplx h = h_interp[i];
        double h_mag2 = cplx_mag2(h);
        if (h_mag2 < 1e-12) h_mag2 = 1e-12;
        data_syms[i] = cplx_scale(cplx_mul(freq[p->data_idx[i]], cplx_conj(h)),
                                  1.0 / h_mag2);
        if (h_est) h_est[i] = h;
    }

    free(freq);
    return p->n_data;
}

int ofdm_demodulate_block(const OfdmParams *p, int n_symbols,
                          const Cplx *in, Cplx *data_syms)
{
    int sym_size = p->n_fft + p->n_cp;
    int total = 0;
    for (int s = 0; s < n_symbols; s++) {
        ofdm_demodulate(p, in + s * sym_size,
                        data_syms + s * p->n_data, NULL);
        total += p->n_data;
    }
    return total;
}

/* ════════════════════════════════════════════════════════════════════
 *  Channel estimation (linear interpolation between pilots)
 * ════════════════════════════════════════════════════════════════════ */

void ofdm_channel_estimate(const OfdmParams *p, const Cplx *rx_freq,
                           Cplx *h_interp)
{
    /* Estimate H at pilot positions: H[k] = Rx[k] / Tx_pilot */
    Cplx h_pilot[OFDM_MAX_PILOTS];
    for (int i = 0; i < p->n_pilot; i++) {
        Cplx rx_pilot = rx_freq[p->pilot_idx[i]];
        double p_mag2 = cplx_mag2(p->pilot_val);
        if (p_mag2 < 1e-12) p_mag2 = 1e-12;
        h_pilot[i] = cplx_scale(cplx_mul(rx_pilot, cplx_conj(p->pilot_val)),
                                1.0 / p_mag2);
    }

    /* Linear interpolation to data positions */
    for (int d = 0; d < p->n_data; d++) {
        int dk = p->data_idx[d];

        /* Find surrounding pilots */
        int lo = -1, hi = -1;
        for (int pi = 0; pi < p->n_pilot; pi++) {
            if (p->pilot_idx[pi] <= dk) lo = pi;
            if (p->pilot_idx[pi] >= dk && hi < 0) hi = pi;
        }

        if (lo < 0 && hi >= 0) {
            h_interp[d] = h_pilot[hi];
        } else if (hi < 0 && lo >= 0) {
            h_interp[d] = h_pilot[lo];
        } else if (lo >= 0 && hi >= 0 && lo != hi) {
            double alpha = (double)(dk - p->pilot_idx[lo]) /
                           (double)(p->pilot_idx[hi] - p->pilot_idx[lo]);
            h_interp[d] = cplx_add(
                cplx_scale(h_pilot[lo], 1.0 - alpha),
                cplx_scale(h_pilot[hi], alpha));
        } else {
            h_interp[d] = (lo >= 0) ? h_pilot[lo] : cplx(1.0, 0.0);
        }
    }
}

void ofdm_equalise_zf(const Cplx *data, const Cplx *h, int n, Cplx *out)
{
    for (int i = 0; i < n; i++) {
        double h_mag2 = cplx_mag2(h[i]);
        if (h_mag2 < 1e-12) h_mag2 = 1e-12;
        out[i] = cplx_scale(cplx_mul(data[i], cplx_conj(h[i])), 1.0 / h_mag2);
    }
}
