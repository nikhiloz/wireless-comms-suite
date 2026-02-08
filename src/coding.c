/**
 * @file coding.c
 * @brief Source coding & channel coding — Huffman, CRC, Hamming, Viterbi.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Source coding        → chapters/02-source-coding/tutorial.md
 *   Channel coding       → chapters/03-channel-coding/tutorial.md
 *   Convolutional codes  → chapters/11-convolutional-viterbi/tutorial.md
 *   Interleaving         → chapters/12-interleaving/tutorial.md
 *
 * References:
 *   Proakis & Salehi, Digital Communications (5th ed.), Ch. 8.
 *   Lin & Costello, Error Control Coding (2nd ed.).
 */

#include "../include/coding.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ════════════════════════════════════════════════════════════════════
 *  Entropy
 * ════════════════════════════════════════════════════════════════════ */

double entropy(const double *probs, int n_symbols)
{
    double H = 0;
    for (int i = 0; i < n_symbols; i++) {
        if (probs[i] > 1e-15)
            H -= probs[i] * log2(probs[i]);
    }
    return H;
}

/* ════════════════════════════════════════════════════════════════════
 *  Huffman coding (min-heap based)
 * ════════════════════════════════════════════════════════════════════ */

typedef struct HuffNode {
    double prob;
    int    symbol;       /* -1 for internal nodes */
    struct HuffNode *left, *right;
} HuffNode;

static HuffNode *huff_new_node(double prob, int sym)
{
    HuffNode *n = (HuffNode *)calloc(1, sizeof(HuffNode));
    n->prob = prob;
    n->symbol = sym;
    return n;
}

static void huff_free_tree(HuffNode *n)
{
    if (!n) return;
    huff_free_tree(n->left);
    huff_free_tree(n->right);
    free(n);
}

/* Simple min-heap for Huffman tree construction */
typedef struct {
    HuffNode *nodes[HUFFMAN_MAX_SYMBOLS];
    int size;
} MinHeap;

static void heap_push(MinHeap *h, HuffNode *n)
{
    int i = h->size++;
    h->nodes[i] = n;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->nodes[parent]->prob > h->nodes[i]->prob) {
            HuffNode *tmp = h->nodes[parent];
            h->nodes[parent] = h->nodes[i];
            h->nodes[i] = tmp;
            i = parent;
        } else break;
    }
}

static HuffNode *heap_pop(MinHeap *h)
{
    HuffNode *min = h->nodes[0];
    h->nodes[0] = h->nodes[--h->size];
    int i = 0;
    while (1) {
        int left = 2 * i + 1, right = 2 * i + 2, smallest = i;
        if (left < h->size && h->nodes[left]->prob < h->nodes[smallest]->prob)
            smallest = left;
        if (right < h->size && h->nodes[right]->prob < h->nodes[smallest]->prob)
            smallest = right;
        if (smallest == i) break;
        HuffNode *tmp = h->nodes[i];
        h->nodes[i] = h->nodes[smallest];
        h->nodes[smallest] = tmp;
        i = smallest;
    }
    return min;
}

static void huff_build_codes(HuffNode *n, uint32_t code, int len,
                             HuffmanTable *ht)
{
    if (!n) return;
    if (n->symbol >= 0) {
        ht->codes[n->symbol] = code;
        ht->lengths[n->symbol] = len;
        return;
    }
    huff_build_codes(n->left,  (code << 1) | 0, len + 1, ht);
    huff_build_codes(n->right, (code << 1) | 1, len + 1, ht);
}

int huffman_build(const double *probs, int n_symbols, HuffmanTable *ht)
{
    if (n_symbols < 2 || n_symbols > HUFFMAN_MAX_SYMBOLS) return -1;

    memset(ht, 0, sizeof(*ht));
    ht->n_symbols = n_symbols;

    MinHeap heap = { .size = 0 };
    for (int i = 0; i < n_symbols; i++) {
        if (probs[i] > 0)
            heap_push(&heap, huff_new_node(probs[i], i));
    }
    if (heap.size < 2) return -1;

    while (heap.size > 1) {
        HuffNode *a = heap_pop(&heap);
        HuffNode *b = heap_pop(&heap);
        HuffNode *parent = huff_new_node(a->prob + b->prob, -1);
        parent->left = a;
        parent->right = b;
        heap_push(&heap, parent);
    }

    HuffNode *root = heap_pop(&heap);
    huff_build_codes(root, 0, 0, ht);
    huff_free_tree(root);

    /* Compute average code length */
    ht->avg_length = 0;
    for (int i = 0; i < n_symbols; i++) {
        if (probs[i] > 0 && ht->lengths[i] > 0)
            ht->avg_length += probs[i] * ht->lengths[i];
    }
    return 0;
}

int huffman_encode(const HuffmanTable *ht, const uint8_t *syms, int n,
                   uint8_t *bits, int max_bits)
{
    int pos = 0;
    for (int i = 0; i < n; i++) {
        int sym = syms[i];
        if (sym >= ht->n_symbols) return -1;
        int len = ht->lengths[sym];
        uint32_t code = ht->codes[sym];
        for (int b = len - 1; b >= 0; b--) {
            if (pos >= max_bits) return -1;
            bits[pos++] = (code >> b) & 1;
        }
    }
    return pos;
}

int huffman_decode(const HuffmanTable *ht, const uint8_t *bits, int nbits,
                   uint8_t *syms, int max_syms)
{
    /* Brute-force table lookup (simple, not performance-critical) */
    int pos = 0, nsyms = 0;
    while (pos < nbits && nsyms < max_syms) {
        int found = 0;
        for (int s = 0; s < ht->n_symbols && !found; s++) {
            int len = ht->lengths[s];
            if (len == 0 || pos + len > nbits) continue;
            uint32_t code = ht->codes[s];
            int match = 1;
            for (int b = 0; b < len && match; b++) {
                if (((code >> (len - 1 - b)) & 1) != (bits[pos + b] & 1))
                    match = 0;
            }
            if (match) {
                syms[nsyms++] = (uint8_t)s;
                pos += len;
                found = 1;
            }
        }
        if (!found) break; /* invalid bitstream */
    }
    return nsyms;
}

/* ════════════════════════════════════════════════════════════════════
 *  Run-length encoding
 * ════════════════════════════════════════════════════════════════════ */

int rle_encode(const uint8_t *data, int n, uint8_t *out, int max_out)
{
    int pos = 0;
    int i = 0;
    while (i < n) {
        uint8_t val = data[i];
        int count = 1;
        while (i + count < n && data[i + count] == val && count < 255)
            count++;
        if (pos + 2 > max_out) return -1;
        out[pos++] = (uint8_t)count;
        out[pos++] = val;
        i += count;
    }
    return pos;
}

int rle_decode(const uint8_t *data, int n, uint8_t *out, int max_out)
{
    int pos = 0;
    for (int i = 0; i + 1 < n; i += 2) {
        int count = data[i];
        uint8_t val = data[i + 1];
        for (int j = 0; j < count; j++) {
            if (pos >= max_out) return -1;
            out[pos++] = val;
        }
    }
    return pos;
}

/* ════════════════════════════════════════════════════════════════════
 *  CRC — CRC-16-CCITT, CRC-32, CRC-24 (Mode-S)
 * ════════════════════════════════════════════════════════════════════ */

uint16_t crc16_ccitt(const uint8_t *data, int n)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < n; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint32_t crc32(const uint8_t *data, int n)
{
    uint32_t crc = 0xFFFFFFFF;
    for (int i = 0; i < n; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}

uint32_t crc24_adsb(const uint8_t *data, int n)
{
    /* CRC-24 for Mode-S / ADS-B: polynomial 0xFFF409 */
    uint32_t crc = 0;
    for (int i = 0; i < n; i++) {
        crc ^= (uint32_t)data[i] << 16;
        for (int b = 0; b < 8; b++) {
            if (crc & 0x800000)
                crc = (crc << 1) ^ 0xFFF409;
            else
                crc <<= 1;
            crc &= 0xFFFFFF;
        }
    }
    return crc;
}

/* ════════════════════════════════════════════════════════════════════
 *  Parity
 * ════════════════════════════════════════════════════════════════════ */

uint8_t parity_even(const uint8_t *bits, int n)
{
    uint8_t p = 0;
    for (int i = 0; i < n; i++) p ^= (bits[i] & 1);
    return p;
}

void parity_encode(const uint8_t *in, int n, uint8_t *out)
{
    memcpy(out, in, n);
    out[n] = parity_even(in, n);
}

int parity_check(const uint8_t *codeword, int n_plus_1)
{
    return parity_even(codeword, n_plus_1); /* 0 = no error */
}

/* ════════════════════════════════════════════════════════════════════
 *  Hamming(7,4)
 * ════════════════════════════════════════════════════════════════════ */

/* Generator matrix rows (systematic: [I4 | P]) */
static const uint8_t hamming_G[4][7] = {
    {1,0,0,0, 1,1,0},
    {0,1,0,0, 0,1,1},
    {0,0,1,0, 1,1,1},
    {0,0,0,1, 1,0,1}
};

/* Parity check matrix (H) */
static const uint8_t hamming_H[3][7] = {
    {1,0,1,1,1,0,0},
    {1,1,1,0,0,1,0},
    {0,1,1,1,0,0,1}
};

void hamming74_encode(const uint8_t *data4, uint8_t *code7)
{
    for (int j = 0; j < 7; j++) {
        code7[j] = 0;
        for (int i = 0; i < 4; i++)
            code7[j] ^= (data4[i] & 1) & hamming_G[i][j];
    }
}

int hamming74_decode(const uint8_t *code7, uint8_t *data4)
{
    /* Compute syndrome */
    uint8_t syndrome[3];
    for (int i = 0; i < 3; i++) {
        syndrome[i] = 0;
        for (int j = 0; j < 7; j++)
            syndrome[i] ^= (code7[j] & 1) & hamming_H[i][j];
    }

    int syn_val = (syndrome[0] << 2) | (syndrome[1] << 1) | syndrome[2];
    int error_pos = -1;

    if (syn_val != 0) {
        /* Find matching column in H */
        for (int j = 0; j < 7; j++) {
            int col = (hamming_H[0][j] << 2) | (hamming_H[1][j] << 1) | hamming_H[2][j];
            if (col == syn_val) {
                error_pos = j;
                break;
            }
        }
    }

    /* Copy and correct */
    uint8_t corrected[7];
    memcpy(corrected, code7, 7);
    if (error_pos >= 0)
        corrected[error_pos] ^= 1;

    /* Extract data bits (systematic) */
    for (int i = 0; i < 4; i++)
        data4[i] = corrected[i];

    return error_pos; /* -1 = no error, else corrected position */
}

/* ════════════════════════════════════════════════════════════════════
 *  Convolutional encoder (rate 1/2, K=7)
 * ════════════════════════════════════════════════════════════════════ */

static int popcount(unsigned int x)
{
    int c = 0;
    while (x) { c += x & 1; x >>= 1; }
    return c;
}

void conv_encode(const uint8_t *in, int n, uint8_t *out)
{
    unsigned int state = 0;
    for (int i = 0; i < n; i++) {
        state = ((state << 1) | (in[i] & 1)) & (CONV_STATES * 2 - 1);
        out[2 * i]     = popcount(state & CONV_G0) & 1;
        out[2 * i + 1] = popcount(state & CONV_G1) & 1;
    }
}

/* ════════════════════════════════════════════════════════════════════
 *  Viterbi decoder (hard decision)
 * ════════════════════════════════════════════════════════════════════ */

#define VITERBI_INF 1e30

int viterbi_decode(const uint8_t *coded, int n_coded, uint8_t *decoded)
{
    int n_data = n_coded / 2;
    if (n_data < 1) return 0;

    /* Path metrics */
    double pm_old[CONV_STATES], pm_new[CONV_STATES];
    int path[256][CONV_STATES]; /* limit to 256 bits max for stack alloc */

    int max_len = n_data < 256 ? n_data : 256;

    for (int s = 0; s < CONV_STATES; s++) pm_old[s] = VITERBI_INF;
    pm_old[0] = 0;

    for (int t = 0; t < max_len; t++) {
        for (int s = 0; s < CONV_STATES; s++) pm_new[s] = VITERBI_INF;

        uint8_t r0 = coded[2 * t] & 1;
        uint8_t r1 = coded[2 * t + 1] & 1;

        for (int s = 0; s < CONV_STATES; s++) {
            if (pm_old[s] >= VITERBI_INF) continue;

            for (int bit = 0; bit <= 1; bit++) {
                int ns = ((s << 1) | bit) & (CONV_STATES - 1);
                unsigned int full_state = (s << 1) | bit;
                uint8_t e0 = popcount(full_state & CONV_G0) & 1;
                uint8_t e1 = popcount(full_state & CONV_G1) & 1;

                double bm = (e0 != r0) + (e1 != r1); /* Hamming distance */
                double metric = pm_old[s] + bm;

                if (metric < pm_new[ns]) {
                    pm_new[ns] = metric;
                    path[t][ns] = s;
                }
            }
        }

        memcpy(pm_old, pm_new, sizeof(pm_old));
    }

    /* Traceback from best final state */
    int best_state = 0;
    for (int s = 1; s < CONV_STATES; s++)
        if (pm_old[s] < pm_old[best_state]) best_state = s;

    int state = best_state;
    for (int t = max_len - 1; t >= 0; t--) {
        int prev = path[t][state];
        decoded[t] = (state >> (CONV_K - 2)) & 1;
        state = prev;
    }

    return max_len;
}

int viterbi_decode_soft(const double *llr, int n_coded, uint8_t *decoded)
{
    int n_data = n_coded / 2;
    if (n_data < 1) return 0;

    double pm_old[CONV_STATES], pm_new[CONV_STATES];
    int path[256][CONV_STATES];
    int max_len = n_data < 256 ? n_data : 256;

    for (int s = 0; s < CONV_STATES; s++) pm_old[s] = VITERBI_INF;
    pm_old[0] = 0;

    for (int t = 0; t < max_len; t++) {
        for (int s = 0; s < CONV_STATES; s++) pm_new[s] = VITERBI_INF;

        double l0 = llr[2 * t];
        double l1 = llr[2 * t + 1];

        for (int s = 0; s < CONV_STATES; s++) {
            if (pm_old[s] >= VITERBI_INF) continue;

            for (int bit = 0; bit <= 1; bit++) {
                int ns = ((s << 1) | bit) & (CONV_STATES - 1);
                unsigned int full_state = (s << 1) | bit;
                uint8_t e0 = popcount(full_state & CONV_G0) & 1;
                uint8_t e1 = popcount(full_state & CONV_G1) & 1;

                /* Soft metric: LLR * encoded bit (positive = agrees with 0) */
                double bm = (e0 ? -l0 : l0) + (e1 ? -l1 : l1);
                double metric = pm_old[s] - bm;

                if (metric < pm_new[ns]) {
                    pm_new[ns] = metric;
                    path[t][ns] = s;
                }
            }
        }
        memcpy(pm_old, pm_new, sizeof(pm_old));
    }

    int best_state = 0;
    for (int s = 1; s < CONV_STATES; s++)
        if (pm_old[s] < pm_old[best_state]) best_state = s;

    int state = best_state;
    for (int t = max_len - 1; t >= 0; t--) {
        int prev = path[t][state];
        decoded[t] = (state >> (CONV_K - 2)) & 1;
        state = prev;
    }

    return max_len;
}

/* ════════════════════════════════════════════════════════════════════
 *  Interleaver (block interleaver: write by rows, read by columns)
 * ════════════════════════════════════════════════════════════════════ */

int interleaver_init(Interleaver *itl, int rows, int cols)
{
    itl->rows = rows;
    itl->cols = cols;
    int size = rows * cols;
    itl->perm = (int *)malloc(size * sizeof(int));
    itl->inv  = (int *)malloc(size * sizeof(int));
    if (!itl->perm || !itl->inv) return -1;

    /* Write by rows, read by columns */
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int write_idx = r * cols + c;   /* row-major input */
            int read_idx  = c * rows + r;   /* column-major output */
            itl->perm[write_idx] = read_idx;
            itl->inv[read_idx] = write_idx;
        }
    }
    return 0;
}

void interleaver_free(Interleaver *itl)
{
    free(itl->perm); itl->perm = NULL;
    free(itl->inv);  itl->inv = NULL;
}

void interleaver_apply(const Interleaver *itl,
                       const uint8_t *in, uint8_t *out, int n)
{
    int size = itl->rows * itl->cols;
    for (int i = 0; i < n && i < size; i++)
        out[itl->perm[i]] = in[i];
}

void interleaver_deapply(const Interleaver *itl,
                         const uint8_t *in, uint8_t *out, int n)
{
    int size = itl->rows * itl->cols;
    for (int i = 0; i < n && i < size; i++)
        out[itl->inv[i]] = in[i];
}
