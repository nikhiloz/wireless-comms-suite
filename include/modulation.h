/**
 * @file modulation.h
 * @brief Digital modulation — BPSK, QPSK, 8PSK, 16/64-QAM, GFSK, O-QPSK.
 *
 * Provides:
 *   - Constellation generation & gray-coding
 *   - Bit-to-symbol mapping (modulate)
 *   - Symbol-to-bit  mapping (demodulate, hard & soft)
 *   - BER theoretical curves
 *   - Gaussian frequency-shift keying (GFSK) for Bluetooth
 *   - O-QPSK with half-sine pulse for Zigbee
 */

#ifndef MODULATION_H
#define MODULATION_H

#include "comms_utils.h"
#include <stdint.h>

/* ── Modulation scheme enum ──────────────────────────────────────── */

typedef enum {
    MOD_BPSK,
    MOD_QPSK,
    MOD_8PSK,
    MOD_16QAM,
    MOD_64QAM,
    MOD_GFSK,       /* Gaussian FSK (BT=0.5 default) */
    MOD_OQPSK       /* Offset QPSK (half-sine)       */
} ModScheme;

/** Number of bits per symbol for a given scheme. */
int mod_bits_per_symbol(ModScheme scheme);

/* ── PSK / QAM constellation ─────────────────────────────────────── */

/**
 * @brief Generate the reference constellation points.
 * @param scheme  Modulation scheme (BPSK..64QAM)
 * @param pts     Output array (must hold 2^bps entries)
 * @return Number of constellation points (M)
 */
int mod_constellation(ModScheme scheme, Cplx *pts);

/* ── Modulate ────────────────────────────────────────────────────── */

/**
 * @brief Map bit stream → complex symbols.
 * @param scheme  Modulation type
 * @param bits    Input bit stream
 * @param nbits   Number of input bits (must be multiple of bps)
 * @param syms    Output symbol array
 * @return Number of symbols produced
 */
int mod_modulate(ModScheme scheme, const uint8_t *bits, int nbits, Cplx *syms);

/* ── Demodulate ──────────────────────────────────────────────────── */

/**
 * @brief Hard-decision demodulation (minimum Euclidean distance).
 * @return Number of bits produced
 */
int mod_demodulate(ModScheme scheme, const Cplx *syms, int nsyms, uint8_t *bits);

/**
 * @brief Soft-output demodulation — produces log-likelihood ratios.
 * @param sigma   Noise standard deviation (sqrt(N0/2))
 * @param llr     Output LLR array (nsyms * bps values)
 * @return Number of LLR values produced
 */
int mod_demodulate_soft(ModScheme scheme, const Cplx *syms, int nsyms,
                        double sigma, double *llr);

/* ── BER theoretical ─────────────────────────────────────────────── */

/** BPSK BER = Q(sqrt(2*Eb/N0)). Input is Eb/N0 in linear scale. */
double ber_bpsk_theory(double ebn0_lin);

/** QPSK BER ≈ BPSK BER (same Eb/N0 performance). */
double ber_qpsk_theory(double ebn0_lin);

/** 16-QAM approximate BER. */
double ber_16qam_theory(double ebn0_lin);

/** Generic Q-function: Q(x) = 0.5 * erfc(x / sqrt(2)). */
double q_function(double x);

/* ── GFSK (Bluetooth) ───────────────────────────────────────────── */

/**
 * @brief GFSK modulate: bits → baseband I/Q samples.
 * @param bits     Input bits
 * @param nbits    Number of bits
 * @param sps      Samples per symbol (e.g. 8)
 * @param bt       Bandwidth-time product (0.5 for BT Classic, 0.3 for BLE)
 * @param h        Modulation index (0.32 typical)
 * @param out      Output complex samples (nbits * sps)
 * @return Number of samples produced
 */
int gfsk_modulate(const uint8_t *bits, int nbits, int sps,
                  double bt, double h, Cplx *out);

/**
 * @brief GFSK demodulate: I/Q → bits via FM discriminator.
 * @return Number of bits recovered
 */
int gfsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits);

/* ── O-QPSK with half-sine shaping (802.15.4 / Zigbee) ──────────── */

int oqpsk_modulate(const uint8_t *bits, int nbits, int sps, Cplx *out);
int oqpsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits);

/* ── Pulse shaping ───────────────────────────────────────────────── */

/**
 * @brief Generate raised-cosine pulse shape.
 * @param alpha   Roll-off factor [0, 1]
 * @param sps     Samples per symbol
 * @param span    Number of symbol periods for the filter
 * @param h       Output coefficients (span * sps + 1)
 * @return Filter length
 */
int raised_cosine(double alpha, int sps, int span, double *h);

/**
 * @brief Generate root-raised-cosine pulse shape.
 */
int root_raised_cosine(double alpha, int sps, int span, double *h);

/**
 * @brief Apply pulse shaping to symbols (upsample + filter).
 * @param syms    Input symbols (real, e.g. NRZ ±1)
 * @param nsyms   Number of symbols
 * @param h       Filter taps
 * @param hlen    Filter length
 * @param sps     Samples per symbol
 * @param out     Output (nsyms * sps + hlen - 1)
 * @return Number of output samples
 */
int pulse_shape(const double *syms, int nsyms, const double *h, int hlen,
                int sps, double *out);

/**
 * @brief NRZ line coding: bit 0 → -1.0, bit 1 → +1.0 */
void nrz_encode(const uint8_t *bits, int n, double *out);

/**
 * @brief Manchester line coding: each bit → two half-symbol values */
void manchester_encode(const uint8_t *bits, int n, double *out);

#endif /* MODULATION_H */
