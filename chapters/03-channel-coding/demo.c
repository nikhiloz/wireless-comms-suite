/**
 * @file demo.c
 * @brief Chapter 03 — Channel Coding (Parity, Hamming, CRC)
 *
 * Demonstrates error detection and correction codes.
 *
 * Build:  make build/bin/03-channel-coding
 * Run:    ./build/bin/03-channel-coding
 */

#include <stdio.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/coding.h"

int main(void)
{
    rng_seed(3);
    print_separator("Chapter 03: Channel Coding — Parity, Hamming, CRC");

    /* ── Parity bit ──────────────────────────────────────────── */
    printf("1. Even Parity\n");
    uint8_t data1[8] = {1,0,1,1,0,0,1,0};
    printf("   Data: ");
    for (int i = 0; i < 8; i++) printf("%d", data1[i]);
    uint8_t p = parity_even(data1, 8);
    printf("  → parity bit = %d\n\n", p);

    /* ── Hamming(7,4) ────────────────────────────────────────── */
    printf("2. Hamming(7,4)\n");
    uint8_t msg[4] = {1, 0, 1, 1};
    uint8_t encoded[7];
    hamming74_encode(msg, encoded);
    printf("   Message: ");
    for (int i = 0; i < 4; i++) printf("%d", msg[i]);
    printf("  → Codeword: ");
    for (int i = 0; i < 7; i++) printf("%d", encoded[i]);
    printf("\n");

    /* Introduce single error */
    encoded[2] ^= 1;
    printf("   Corrupted bit 2: ");
    for (int i = 0; i < 7; i++) printf("%d", encoded[i]);
    printf("\n");

    uint8_t decoded[4];
    int corrected = hamming74_decode(encoded, decoded);
    printf("   Decoded: ");
    for (int i = 0; i < 4; i++) printf("%d", decoded[i]);
    printf("  (corrected %d error)\n\n", corrected);

    /* ── CRC ─────────────────────────────────────────────────── */
    printf("3. CRC\n");
    uint8_t crc_data[] = "Hello, CRC!";
    uint16_t crc16 = crc16_ccitt(crc_data, 11);
    uint32_t crc32v = crc32(crc_data, 11);
    printf("   Data: \"%s\"\n", crc_data);
    printf("   CRC-16 = 0x%04X\n", crc16);
    printf("   CRC-32 = 0x%08X\n", crc32v);

    print_separator("End of Chapter 03");
    return 0;
}
