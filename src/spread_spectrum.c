/**
 * @file spread_spectrum.c
 * @brief Spread spectrum — DSSS, FHSS, PN sequence generation.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Spread spectrum    → chapters/15-spread-spectrum/tutorial.md
 *   Zigbee PHY         → chapters/18-zigbee-phy/tutorial.md
 *
 * References:
 *   Haykin, Communication Systems (4th ed.), Ch. 9.
 *   IEEE 802.15.4-2020, Table 22 (chip sequences).
 */

#include "../include/spread_spectrum.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* ════════════════════════════════════════════════════════════════════
 *  PN sequences (LFSR-based)
 * ════════════════════════════════════════════════════════════════════ */

int pn_msequence(uint32_t poly, int n_bits, int *seq)
{
    int len = (1 << n_bits) - 1;
    if (len > PN_MAX_LEN) len = PN_MAX_LEN;

    uint32_t lfsr = 1; /* non-zero initial state */
    for (int i = 0; i < len; i++) {
        /* Output = LSB of LFSR → map to ±1 */
        seq[i] = (lfsr & 1) ? +1 : -1;

        /* Feedback: XOR of tapped bits */
        uint32_t fb = 0;
        uint32_t tmp = lfsr & poly;
        while (tmp) { fb ^= tmp & 1; tmp >>= 1; }

        lfsr = (lfsr >> 1) | (fb << (n_bits - 1));
    }
    return len;
}

int pn_gold(uint32_t poly1, uint32_t poly2, int n_bits, int shift, int *seq)
{
    int len = (1 << n_bits) - 1;
    if (len > PN_MAX_LEN) len = PN_MAX_LEN;

    int *m1 = (int *)malloc(len * sizeof(int));
    int *m2 = (int *)malloc(len * sizeof(int));

    pn_msequence(poly1, n_bits, m1);
    pn_msequence(poly2, n_bits, m2);

    /* Gold code = m1 XOR shifted m2 (in ±1: multiply) */
    for (int i = 0; i < len; i++) {
        int j = (i + shift) % len;
        seq[i] = m1[i] * m2[j];
    }

    free(m1);
    free(m2);
    return len;
}

void pn_autocorr(const int *seq, int n, double *corr)
{
    for (int lag = 0; lag < n; lag++) {
        double sum = 0;
        for (int i = 0; i < n; i++)
            sum += seq[i] * seq[(i + lag) % n];
        corr[lag] = sum;
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  DSSS spreading / despreading
 * ════════════════════════════════════════════════════════════════════ */

void dsss_spread(const uint8_t *bits, int nbits,
                 const int *pn_code, int chip_len, double *chips)
{
    for (int i = 0; i < nbits; i++) {
        double bit_val = bits[i] ? +1.0 : -1.0;
        for (int c = 0; c < chip_len; c++)
            chips[i * chip_len + c] = bit_val * pn_code[c];
    }
}

int dsss_despread(const double *chips, int nchips,
                  const int *pn_code, int chip_len, uint8_t *bits)
{
    int nbits = nchips / chip_len;
    for (int i = 0; i < nbits; i++) {
        double corr = 0;
        for (int c = 0; c < chip_len; c++)
            corr += chips[i * chip_len + c] * pn_code[c];
        bits[i] = (corr > 0) ? 1 : 0;
    }
    return nbits;
}

double dsss_processing_gain_db(int chip_len)
{
    return 10.0 * log10((double)chip_len);
}

/* ════════════════════════════════════════════════════════════════════
 *  FHSS
 * ════════════════════════════════════════════════════════════════════ */

void fhss_init(FhssParams *fp, int n_channels, int n_hops,
               uint32_t seed, double dwell)
{
    fp->n_channels = n_channels;
    fp->hop_len = n_hops;
    fp->dwell_time = dwell;

    /* Simple LCG-based hop pattern */
    uint32_t state = seed;
    for (int i = 0; i < n_hops; i++) {
        state = state * 1103515245 + 12345;
        fp->hop_seq[i] = (int)((state >> 16) % n_channels);
    }
}

int fhss_get_channel(const FhssParams *fp, int hop_idx)
{
    return fp->hop_seq[hop_idx % fp->hop_len];
}

/* ════════════════════════════════════════════════════════════════════
 *  802.15.4 chip mapping (16 PN codes for 4-bit symbols)
 * ════════════════════════════════════════════════════════════════════ */

/* Table 22 from IEEE 802.15.4-2020 (2.4 GHz DSSS) */
static const uint32_t zigbee_chip_table[16] = {
    0x744AC39B, 0x44AC39B7, 0x4AC39B74, 0xAC39B744,
    0xC39B744A, 0x39B744AC, 0x9B744AC3, 0xB744AC39,
    0xDEE06931, 0xEE06931D, 0xE06931DE, 0x06931DEE,
    0x6931DEE0, 0x931DEE06, 0x31DEE069, 0x1DEE0693
};

void zigbee_chip_map(uint8_t symbol, int *chips32)
{
    uint32_t code = zigbee_chip_table[symbol & 0x0F];
    for (int i = 0; i < 32; i++) {
        chips32[i] = (code & (1u << (31 - i))) ? +1 : -1;
    }
}
