/**
 * @file spread_spectrum.h
 * @brief Spread spectrum — DSSS, FHSS, PN sequence generation.
 *
 * Covers:
 *   - PN sequence generators (LFSR, Gold, Kasami)
 *   - DSSS spreading / despreading
 *   - FHSS hop pattern generation
 *   - Processing gain calculation
 *   - Chip-rate correlation receiver
 */

#ifndef SPREAD_SPECTRUM_H
#define SPREAD_SPECTRUM_H

#include "comms_utils.h"
#include <stdint.h>

/* ── PN Sequences ────────────────────────────────────────────────── */

#define PN_MAX_LEN 4096

/**
 * @brief Generate maximal-length sequence (m-sequence) via LFSR.
 * @param poly    LFSR polynomial (binary, MSB = highest order)
 * @param n_bits  Number of LFSR bits (sequence length = 2^n_bits - 1)
 * @param seq     Output sequence (±1 values), length = 2^n_bits - 1
 * @return Sequence length
 */
int pn_msequence(uint32_t poly, int n_bits, int *seq);

/**
 * @brief Generate Gold code from two m-sequences.
 * @param poly1, poly2  LFSR polynomials for the two base sequences
 * @param n_bits         LFSR order
 * @param shift          Phase shift of second sequence [0, 2^n-2]
 * @param seq            Output Gold code (±1)
 * @return Code length
 */
int pn_gold(uint32_t poly1, uint32_t poly2, int n_bits, int shift, int *seq);

/** Auto-correlation of PN sequence (should be ~N for zero lag, ~-1 else). */
void pn_autocorr(const int *seq, int n, double *corr);

/* ── DSSS ────────────────────────────────────────────────────────── */

/**
 * @brief DSSS spread: each bit → chip_len chips (bit XOR PN).
 * @param bits     Input data bits (0/1)
 * @param nbits    Number of data bits
 * @param pn_code  PN spreading code (±1)
 * @param chip_len Chips per bit (spreading factor)
 * @param chips    Output chip stream (±1), length = nbits * chip_len
 */
void dsss_spread(const uint8_t *bits, int nbits,
                 const int *pn_code, int chip_len, double *chips);

/**
 * @brief DSSS despread: correlate chips with PN code to recover bits.
 * @param chips    Received chip stream (may contain noise)
 * @param nchips   Number of chips (must be multiple of chip_len)
 * @param pn_code  PN spreading code (±1)
 * @param chip_len Chips per bit
 * @param bits     Recovered bits (output)
 * @return Number of recovered bits
 */
int dsss_despread(const double *chips, int nchips,
                  const int *pn_code, int chip_len, uint8_t *bits);

/**
 * @brief Processing gain in dB = 10*log10(chip_len).
 */
double dsss_processing_gain_db(int chip_len);

/* ── FHSS ────────────────────────────────────────────────────────── */

#define FHSS_MAX_CHANNELS 128

typedef struct {
    int    n_channels;           /* number of hop channels           */
    int    hop_seq[FHSS_MAX_CHANNELS * 4]; /* hop pattern            */
    int    hop_len;              /* hop sequence length               */
    double dwell_time;           /* time on each channel (seconds)    */
} FhssParams;

/**
 * @brief Generate pseudo-random hop sequence.
 * @param fp          Params struct to fill
 * @param n_channels  Number of available channels
 * @param n_hops      Length of hop sequence
 * @param seed        Seed for deterministic generation
 * @param dwell       Dwell time per hop (seconds)
 */
void fhss_init(FhssParams *fp, int n_channels, int n_hops,
               uint32_t seed, double dwell);

/** Get the channel index for hop number `hop_idx`. */
int fhss_get_channel(const FhssParams *fp, int hop_idx);

/* ── Zigbee chip mapping (802.15.4 DSSS) ─────────────────────────── */

/** Map 4-bit symbol to 32-chip PN sequence (802.15.4 table). */
void zigbee_chip_map(uint8_t symbol, int *chips32);

#endif /* SPREAD_SPECTRUM_H */
