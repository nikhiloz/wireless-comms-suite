/**
 * @file modulation.c
 * @brief Digital modulation — BPSK, QPSK, 8PSK, QAM, GFSK, O-QPSK.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Modulation theory     → chapters/05-digital-modulation/tutorial.md
 *   Pulse shaping         → chapters/04-pulse-shaping/tutorial.md
 *   GFSK (Bluetooth)      → chapters/17-bluetooth-baseband/tutorial.md
 *   O-QPSK (Zigbee)       → chapters/18-zigbee-phy/tutorial.md
 *
 * References:
 *   Proakis & Salehi, Digital Communications (5th ed.), Ch. 4–5.
 *   Haykin, Communication Systems (4th ed.), Ch. 7.
 */

#include "../include/modulation.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ════════════════════════════════════════════════════════════════════
 *  Bits per symbol
 * ════════════════════════════════════════════════════════════════════ */

int mod_bits_per_symbol(ModScheme scheme)
{
    switch (scheme) {
        case MOD_BPSK:   return 1;
        case MOD_QPSK:   return 2;
        case MOD_8PSK:   return 3;
        case MOD_16QAM:  return 4;
        case MOD_64QAM:  return 6;
        case MOD_GFSK:   return 1;
        case MOD_OQPSK:  return 2;
        default:         return 1;
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Constellation generation (Gray-coded)
 * ════════════════════════════════════════════════════════════════════ */

int mod_constellation(ModScheme scheme, Cplx *pts)
{
    int M;
    switch (scheme) {
        case MOD_BPSK:
            pts[0] = cplx(-1.0, 0.0);
            pts[1] = cplx( 1.0, 0.0);
            return 2;

        case MOD_QPSK:
        case MOD_OQPSK:
            /* Gray code: 00→(+1,+1), 01→(-1,+1), 11→(-1,-1), 10→(+1,-1) */
            {
                double s = 1.0 / sqrt(2.0);
                pts[0] = cplx( s,  s);  /* 00 */
                pts[1] = cplx(-s,  s);  /* 01 */
                pts[2] = cplx( s, -s);  /* 10 */
                pts[3] = cplx(-s, -s);  /* 11 */
            }
            return 4;

        case MOD_8PSK:
            M = 8;
            for (int i = 0; i < M; i++) {
                /* Gray ordering: i XOR (i>>1) */
                int gray = i ^ (i >> 1);
                double angle = 2.0 * M_PI * gray / M;
                pts[i] = cplx(cos(angle), sin(angle));
            }
            return M;

        case MOD_16QAM:
            M = 16;
            {
                /* 4×4 grid, Gray-coded on each axis */
                int gray4[4] = {0, 1, 3, 2};
                double norm = 1.0 / sqrt(10.0); /* average power = 1 */
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        int idx = gray4[i] * 4 + gray4[j];
                        pts[idx] = cplx((-3 + 2 * j) * norm,
                                        ( 3 - 2 * i) * norm);
                    }
                }
            }
            return M;

        case MOD_64QAM:
            M = 64;
            {
                int gray8[8] = {0, 1, 3, 2, 7, 6, 4, 5};
                double norm = 1.0 / sqrt(42.0);
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        int idx = gray8[i] * 8 + gray8[j];
                        pts[idx] = cplx((-7 + 2 * j) * norm,
                                        ( 7 - 2 * i) * norm);
                    }
                }
            }
            return M;

        default:
            pts[0] = cplx(-1.0, 0.0);
            pts[1] = cplx( 1.0, 0.0);
            return 2;
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Modulate: bits → complex symbols
 * ════════════════════════════════════════════════════════════════════ */

int mod_modulate(ModScheme scheme, const uint8_t *bits, int nbits, Cplx *syms)
{
    int bps = mod_bits_per_symbol(scheme);
    int M = 1 << bps;
    Cplx constellation[64];
    mod_constellation(scheme, constellation);

    int nsyms = nbits / bps;
    for (int i = 0; i < nsyms; i++) {
        int idx = 0;
        for (int b = 0; b < bps; b++) {
            idx = (idx << 1) | (bits[i * bps + b] & 1);
        }
        if (idx >= M) idx = M - 1;
        syms[i] = constellation[idx];
    }
    return nsyms;
}

/* ════════════════════════════════════════════════════════════════════
 *  Demodulate: complex symbols → bits (hard decision)
 * ════════════════════════════════════════════════════════════════════ */

int mod_demodulate(ModScheme scheme, const Cplx *syms, int nsyms, uint8_t *bits)
{
    int bps = mod_bits_per_symbol(scheme);
    int M = 1 << bps;
    Cplx constellation[64];
    mod_constellation(scheme, constellation);

    int nbits = 0;
    for (int i = 0; i < nsyms; i++) {
        /* Find nearest constellation point */
        double min_dist = 1e30;
        int min_idx = 0;
        for (int j = 0; j < M; j++) {
            double d = cplx_mag2(cplx_sub(syms[i], constellation[j]));
            if (d < min_dist) {
                min_dist = d;
                min_idx = j;
            }
        }
        /* Extract bits from index */
        for (int b = bps - 1; b >= 0; b--) {
            bits[nbits++] = (min_idx >> b) & 1;
        }
    }
    return nbits;
}

/* ════════════════════════════════════════════════════════════════════
 *  Soft-output demodulation (approximate LLR)
 * ════════════════════════════════════════════════════════════════════ */

int mod_demodulate_soft(ModScheme scheme, const Cplx *syms, int nsyms,
                        double sigma, double *llr)
{
    int bps = mod_bits_per_symbol(scheme);
    int M = 1 << bps;
    Cplx constellation[64];
    mod_constellation(scheme, constellation);

    double sigma2 = sigma * sigma;
    if (sigma2 < 1e-30) sigma2 = 1e-30;

    int nllr = 0;
    for (int i = 0; i < nsyms; i++) {
        for (int b = 0; b < bps; b++) {
            double max0 = -1e30, max1 = -1e30;
            for (int j = 0; j < M; j++) {
                double d2 = cplx_mag2(cplx_sub(syms[i], constellation[j]));
                double metric = -d2 / (2.0 * sigma2);
                if ((j >> (bps - 1 - b)) & 1)
                    max1 = (metric > max1) ? metric : max1;
                else
                    max0 = (metric > max0) ? metric : max0;
            }
            llr[nllr++] = max0 - max1; /* positive → bit 0 more likely */
        }
    }
    return nllr;
}

/* ════════════════════════════════════════════════════════════════════
 *  BER theoretical curves
 * ════════════════════════════════════════════════════════════════════ */

double q_function(double x)
{
    return 0.5 * erfc(x / sqrt(2.0));
}

double ber_bpsk_theory(double ebn0_lin)
{
    return q_function(sqrt(2.0 * ebn0_lin));
}

double ber_qpsk_theory(double ebn0_lin)
{
    return ber_bpsk_theory(ebn0_lin); /* Same Eb/N0 performance */
}

double ber_16qam_theory(double ebn0_lin)
{
    return (3.0 / 8.0) * erfc(sqrt(2.0 * ebn0_lin / 5.0));
}

/* ════════════════════════════════════════════════════════════════════
 *  GFSK modulation (Bluetooth)
 * ════════════════════════════════════════════════════════════════════ */

static void gaussian_filter(double bt, int sps, int span, double *h, int *hlen)
{
    *hlen = span * sps + 1;
    double sigma = sqrt(log(2.0)) / (2.0 * M_PI * bt);
    double sum = 0;
    for (int i = 0; i < *hlen; i++) {
        double t = (i - *hlen / 2.0) / sps;
        h[i] = exp(-t * t / (2.0 * sigma * sigma));
        sum += h[i];
    }
    for (int i = 0; i < *hlen; i++) h[i] /= sum;
}

int gfsk_modulate(const uint8_t *bits, int nbits, int sps,
                  double bt, double h_mod, Cplx *out)
{
    int nsamples = nbits * sps;

    /* Generate NRZ impulses */
    double *nrz = (double *)calloc(nsamples, sizeof(double));
    for (int i = 0; i < nbits; i++) {
        double val = bits[i] ? 1.0 : -1.0;
        for (int j = 0; j < sps; j++)
            nrz[i * sps + j] = val;
    }

    /* Gaussian filter */
    double gf[128];
    int gf_len;
    gaussian_filter(bt, sps, 3, gf, &gf_len);

    /* Filter to get frequency deviation */
    double *freq = (double *)calloc(nsamples, sizeof(double));
    for (int i = 0; i < nsamples; i++) {
        freq[i] = 0;
        for (int k = 0; k < gf_len; k++) {
            int idx = i - k + gf_len / 2;
            if (idx >= 0 && idx < nsamples)
                freq[i] += nrz[idx] * gf[k];
        }
    }

    /* Integrate phase and generate I/Q */
    double phase = 0;
    double dev = h_mod * M_PI / sps;
    for (int i = 0; i < nsamples; i++) {
        phase += dev * freq[i];
        out[i] = cplx_exp_j(phase);
    }

    free(nrz);
    free(freq);
    return nsamples;
}

int gfsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits)
{
    /* FM discriminator: angle difference between consecutive samples */
    int nbits = nsamples / sps;
    for (int i = 0; i < nbits; i++) {
        /* Sample at symbol midpoint */
        int idx = i * sps + sps / 2;
        if (idx < 1 || idx >= nsamples) {
            bits[i] = 0;
            continue;
        }
        Cplx prod = cplx_mul(in[idx], cplx_conj(in[idx - 1]));
        double freq = atan2(prod.im, prod.re);
        bits[i] = (freq > 0) ? 1 : 0;
    }
    return nbits;
}

/* ════════════════════════════════════════════════════════════════════
 *  O-QPSK with half-sine shaping (802.15.4 / Zigbee)
 * ════════════════════════════════════════════════════════════════════ */

int oqpsk_modulate(const uint8_t *bits, int nbits, int sps, Cplx *out)
{
    int nsyms = nbits / 2;
    int nsamples = (nsyms + 1) * sps; /* +1 for Q offset */

    /* Generate half-sine pulse */
    double *pulse = (double *)calloc(2 * sps, sizeof(double));
    for (int i = 0; i < 2 * sps; i++)
        pulse[i] = sin(M_PI * i / (2.0 * sps));

    memset(out, 0, nsamples * sizeof(Cplx));

    for (int i = 0; i < nsyms; i++) {
        double bi = bits[2 * i] ? 1.0 : -1.0;      /* I bit */
        double bq = bits[2 * i + 1] ? 1.0 : -1.0;  /* Q bit */

        /* I: shaped at i * sps */
        for (int j = 0; j < 2 * sps && i * sps + j < nsamples; j++)
            out[i * sps + j].re += bi * pulse[j];

        /* Q: offset by half symbol (sps/2), shaped */
        int q_off = i * sps + sps / 2;
        for (int j = 0; j < 2 * sps && q_off + j < nsamples; j++)
            out[q_off + j].im += bq * pulse[j];
    }

    free(pulse);
    return nsamples;
}

int oqpsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits)
{
    int nsyms = (nsamples - sps) / sps;
    if (nsyms < 1) return 0;

    for (int i = 0; i < nsyms; i++) {
        /* Sample I at symbol center */
        int i_idx = i * sps + sps;
        double i_val = (i_idx < nsamples) ? in[i_idx].re : 0;
        bits[2 * i] = (i_val > 0) ? 1 : 0;

        /* Sample Q offset by sps/2 */
        int q_idx = i * sps + sps + sps / 2;
        double q_val = (q_idx < nsamples) ? in[q_idx].im : 0;
        bits[2 * i + 1] = (q_val > 0) ? 1 : 0;
    }
    return nsyms * 2;
}

/* ════════════════════════════════════════════════════════════════════
 *  Pulse shaping
 * ════════════════════════════════════════════════════════════════════ */

int raised_cosine(double alpha, int sps, int span, double *h)
{
    int len = span * sps + 1;
    int half = len / 2;

    for (int i = 0; i < len; i++) {
        double t = (double)(i - half) / sps;

        if (fabs(t) < 1e-12) {
            h[i] = 1.0;
        } else if (alpha > 0 && fabs(fabs(t) - 1.0 / (2.0 * alpha)) < 1e-12) {
            h[i] = (M_PI / 4.0) * sinc(1.0 / (2.0 * alpha));
        } else {
            h[i] = sinc(t) * cos(M_PI * alpha * t) /
                   (1.0 - 4.0 * alpha * alpha * t * t);
        }
    }
    return len;
}

int root_raised_cosine(double alpha, int sps, int span, double *h)
{
    int len = span * sps + 1;
    int half = len / 2;

    for (int i = 0; i < len; i++) {
        double t = (double)(i - half) / sps;

        if (fabs(t) < 1e-12) {
            h[i] = 1.0 - alpha + 4.0 * alpha / M_PI;
        } else if (fabs(fabs(t) - 1.0 / (4.0 * alpha)) < 1e-12 && alpha > 0) {
            h[i] = (alpha / sqrt(2.0)) *
                   ((1.0 + 2.0 / M_PI) * sin(M_PI / (4.0 * alpha)) +
                    (1.0 - 2.0 / M_PI) * cos(M_PI / (4.0 * alpha)));
        } else {
            double num = sin(M_PI * t * (1.0 - alpha)) +
                         4.0 * alpha * t * cos(M_PI * t * (1.0 + alpha));
            double den = M_PI * t * (1.0 - 16.0 * alpha * alpha * t * t);
            h[i] = num / den;
        }
    }

    /* Normalise */
    double energy = 0;
    for (int i = 0; i < len; i++) energy += h[i] * h[i];
    double norm = 1.0 / sqrt(energy / sps);
    for (int i = 0; i < len; i++) h[i] *= norm;

    return len;
}

int pulse_shape(const double *syms, int nsyms, const double *h, int hlen,
                int sps, double *out)
{
    int up_len = nsyms * sps;
    int out_len = up_len + hlen - 1;

    /* Upsample */
    double *upsampled = (double *)calloc(up_len, sizeof(double));
    for (int i = 0; i < nsyms; i++)
        upsampled[i * sps] = syms[i];

    /* Convolve */
    memset(out, 0, out_len * sizeof(double));
    for (int i = 0; i < up_len; i++) {
        if (fabs(upsampled[i]) < 1e-15) continue;
        for (int j = 0; j < hlen; j++)
            out[i + j] += upsampled[i] * h[j];
    }

    free(upsampled);
    return out_len;
}

void nrz_encode(const uint8_t *bits, int n, double *out)
{
    for (int i = 0; i < n; i++)
        out[i] = bits[i] ? 1.0 : -1.0;
}

void manchester_encode(const uint8_t *bits, int n, double *out)
{
    for (int i = 0; i < n; i++) {
        if (bits[i]) {
            out[2 * i]     =  1.0;
            out[2 * i + 1] = -1.0;
        } else {
            out[2 * i]     = -1.0;
            out[2 * i + 1] =  1.0;
        }
    }
}
