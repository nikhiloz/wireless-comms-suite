/**
 * @file demo.c
 * @brief Chapter 02 — Source Coding (Huffman, RLE, Entropy)
 *
 * Demonstrates lossless data compression: entropy calculation,
 * Huffman coding, and run-length encoding.
 *
 * Build:  make build/bin/02-source-coding
 * Run:    ./build/bin/02-source-coding
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/coding.h"

int main(void)
{
    rng_seed(2);
    print_separator("Chapter 02: Source Coding");

    /* ── Entropy demonstration ───────────────────────────────── */
    printf("1. Entropy\n");
    double probs[] = {0.5, 0.25, 0.125, 0.125};
    double H = entropy(probs, 4);
    printf("   P = {0.5, 0.25, 0.125, 0.125}\n");
    printf("   H = %.4f bits/symbol (max = %.4f)\n\n", H, log2(4));

    /* ── RLE demonstration ───────────────────────────────────── */
    printf("2. Run-Length Encoding\n");
    uint8_t rle_data[] = {0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,0};
    int rle_datalen = sizeof(rle_data);
    printf("   Input:  ");
    for (int i = 0; i < rle_datalen; i++) printf("%d", rle_data[i]);
    printf(" (%d values)\n", rle_datalen);

    uint8_t encoded[64];
    int enc_len = rle_encode(rle_data, rle_datalen, encoded, 64);
    printf("   RLE:    %d encoded values ", enc_len);
    printf("(compression: %.1f%%)\n", 100.0 * (1.0 - (double)enc_len / rle_datalen));

    uint8_t decoded[64];
    int dec_len = rle_decode(encoded, enc_len, decoded, 64);
    printf("   Decoded: ");
    for (int i = 0; i < dec_len; i++) printf("%d", decoded[i]);
    printf(" (%d values)\n\n", dec_len);

    /* ── Huffman demonstration ───────────────────────────────── */
    printf("3. Huffman Coding\n");
    double freqs[] = {0.4, 0.3, 0.15, 0.1, 0.05};
    int n_symbols = 5;
    HuffmanTable ht;
    huffman_build(freqs, n_symbols, &ht);
    printf("   Symbol frequencies: {0.4, 0.3, 0.15, 0.1, 0.05}\n");
    printf("   Huffman table built for %d symbols\n", n_symbols);
    printf("   Entropy H = %.4f bits/symbol\n", entropy(freqs, n_symbols));

    print_separator("End of Chapter 02");
    return 0;
}
