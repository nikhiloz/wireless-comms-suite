/**
 * @file coding.h
 * @brief Source coding & channel coding.
 *
 * Covers:
 *   Source coding  — Huffman, run-length, entropy measurement
 *   Channel coding — parity, Hamming(7,4), CRC-16/32,
 *                    convolutional + Viterbi (rate 1/2)
 *   Interleaving   — block and convolutional interleavers
 */

#ifndef CODING_H
#define CODING_H

#include <stdint.h>
#include <stddef.h>

/* ── Entropy ─────────────────────────────────────────────────────── */

/** Compute entropy of a symbol probability distribution (bits). */
double entropy(const double *probs, int n_symbols);

/* ── Huffman coding ──────────────────────────────────────────────── */

#define HUFFMAN_MAX_SYMBOLS 256
#define HUFFMAN_MAX_CODELEN  32

typedef struct {
    int    n_symbols;
    uint32_t codes[HUFFMAN_MAX_SYMBOLS];     /* bit pattern          */
    int      lengths[HUFFMAN_MAX_SYMBOLS];   /* code length in bits  */
    double   avg_length;                     /* weighted average     */
} HuffmanTable;

int huffman_build(const double *probs, int n_symbols, HuffmanTable *ht);
int huffman_encode(const HuffmanTable *ht, const uint8_t *syms, int n,
                   uint8_t *bits, int max_bits);
int huffman_decode(const HuffmanTable *ht, const uint8_t *bits, int nbits,
                   uint8_t *syms, int max_syms);

/* ── Run-length encoding ─────────────────────────────────────────── */

int rle_encode(const uint8_t *data, int n, uint8_t *out, int max_out);
int rle_decode(const uint8_t *data, int n, uint8_t *out, int max_out);

/* ── CRC ─────────────────────────────────────────────────────────── */

uint16_t crc16_ccitt(const uint8_t *data, int n);
uint32_t crc32(const uint8_t *data, int n);
uint32_t crc24_adsb(const uint8_t *data, int n);  /* Mode-S CRC-24 */

/* ── Parity ──────────────────────────────────────────────────────── */

uint8_t parity_even(const uint8_t *bits, int n);
void    parity_encode(const uint8_t *in, int n, uint8_t *out); /* n+1 out */
int     parity_check(const uint8_t *codeword, int n_plus_1);   /* 0=ok */

/* ── Hamming(7,4) ────────────────────────────────────────────────── */

void hamming74_encode(const uint8_t *data4, uint8_t *code7);
int  hamming74_decode(const uint8_t *code7, uint8_t *data4); /* returns 0 or corrected pos */

/* ── Convolutional code (rate 1/2, K=7, g=[133,171] octal) ─────── */

#define CONV_K          7
#define CONV_STATES     (1 << (CONV_K - 1))  /* 64 */
#define CONV_G0         0133                  /* octal */
#define CONV_G1         0171

/** Encode `n` input bits → `2*n` output bits. */
void conv_encode(const uint8_t *in, int n, uint8_t *out);

/** Viterbi hard-decision decode. `n_coded` = length of coded bits (must be even).
 *  Returns decoded length = n_coded/2.  */
int  viterbi_decode(const uint8_t *coded, int n_coded, uint8_t *decoded);

/** Viterbi soft-decision decode (LLR input, positive = more likely 0). */
int  viterbi_decode_soft(const double *llr, int n_coded, uint8_t *decoded);

/* ── Interleaver ─────────────────────────────────────────────────── */

typedef struct {
    int  rows;
    int  cols;
    int *perm;    /* permutation table, size = rows*cols */
    int *inv;     /* inverse permutation                 */
} Interleaver;

int  interleaver_init(Interleaver *itl, int rows, int cols);
void interleaver_free(Interleaver *itl);
void interleaver_apply(const Interleaver *itl,
                       const uint8_t *in, uint8_t *out, int n);
void interleaver_deapply(const Interleaver *itl,
                         const uint8_t *in, uint8_t *out, int n);

#endif /* CODING_H */
