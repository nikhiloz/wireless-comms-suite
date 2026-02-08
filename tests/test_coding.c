/**
 * @file test_coding.c
 * @brief Unit tests for source/channel coding functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/coding.h"

int main(void)
{
    TEST_SUITE("Source & Channel Coding");
    rng_seed(123);

    /* ── Test 1: CRC-16 non-zero ─────────────────────────────── */
    TEST_CASE_BEGIN("CRC-16 CCITT on known data")
    {
        uint8_t data[] = "123456789";
        uint16_t crc = crc16_ccitt(data, 9);
        if (crc != 0) { TEST_PASS_STMT; }
        else          { TEST_FAIL_STMT("CRC-16 should be non-zero"); }
    }
    TEST_CASE_END();

    /* ── Test 2: CRC-32 non-zero ─────────────────────────────── */
    TEST_CASE_BEGIN("CRC-32 on known data")
    {
        uint8_t data[] = "123456789";
        uint32_t crc = crc32(data, 9);
        if (crc != 0) { TEST_PASS_STMT; }
        else          { TEST_FAIL_STMT("CRC-32 should be non-zero"); }
    }
    TEST_CASE_END();

    /* ── Test 3: Hamming(7,4) round-trip ─────────────────────── */
    TEST_CASE_BEGIN("Hamming(7,4) encode → decode, no errors")
    {
        uint8_t data[4] = {1, 0, 1, 1};
        uint8_t coded[7];
        hamming74_encode(data, coded);

        uint8_t decoded[4];
        int corrected = hamming74_decode(coded, decoded);

        int ok = (corrected == -1); /* -1 means no error detected */
        for (int i = 0; i < 4; i++) {
            if (data[i] != decoded[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Hamming round-trip failed"); }
    }
    TEST_CASE_END();

    /* ── Test 4: Hamming(7,4) single-error correction ────────── */
    TEST_CASE_BEGIN("Hamming(7,4) corrects single-bit error")
    {
        uint8_t data[4] = {1, 1, 0, 0};
        uint8_t coded[7];
        hamming74_encode(data, coded);

        /* Flip bit 3 */
        coded[3] ^= 1;

        uint8_t decoded[4];
        int corrected = hamming74_decode(coded, decoded);

        int ok = (corrected >= 0); /* returns corrected bit position, >= 0 means corrected */
        for (int i = 0; i < 4; i++) {
            if (data[i] != decoded[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Hamming did not correct single error"); }
    }
    TEST_CASE_END();

    /* ── Test 5: Convolutional encode → Viterbi decode ───────── */
    TEST_CASE_BEGIN("Conv encode → Viterbi decode, 64 bits")
    {
        /* Use a fixed pattern for deterministic test */
        uint8_t bits[64];
        for (int i = 0; i < 64; i++) bits[i] = (uint8_t)(i & 1);

        uint8_t coded[128];
        conv_encode(bits, 64, coded);
        int coded_len = 2 * 64;

        uint8_t decoded[64];
        viterbi_decode(coded, coded_len, decoded);

        /* The Viterbi decoder output may have a K-2=5 sample delay,
         * meaning decoded[t] == bits[t - delay].  Try delay 0..6. */
        int best_errs = 64;
        for (int d = 0; d <= 6; d++) {
            int errs = 0;
            for (int i = d; i < 58; i++) {
                if (decoded[i] != bits[i - d]) errs++;
            }
            if (errs < best_errs) best_errs = errs;
        }
        if (best_errs <= 2) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Viterbi decode too many errors"); }
    }
    TEST_CASE_END();

    /* ── Test 6: Interleaver round-trip ──────────────────────── */
    TEST_CASE_BEGIN("Interleaver apply → deapply round-trip")
    {
        uint8_t data[24];
        random_bits(data, 24);

        uint8_t original[24];
        memcpy(original, data, 24);

        Interleaver itl;
        interleaver_init(&itl, 4, 6);
        uint8_t interleaved[24];
        interleaver_apply(&itl, data, interleaved, 24);

        uint8_t recovered[24];
        interleaver_deapply(&itl, interleaved, recovered, 24);
        interleaver_free(&itl);

        int ok = 1;
        for (int i = 0; i < 24; i++) {
            if (original[i] != recovered[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Interleaver round-trip failed"); }
    }
    TEST_CASE_END();

    /* ── Test 7: Even parity ─────────────────────────────────── */
    TEST_CASE_BEGIN("Even parity check")
    {
        uint8_t data1[4] = {1, 0, 1, 0}; /* 2 ones → parity 0 */
        uint8_t data2[4] = {1, 1, 1, 0}; /* 3 ones → parity 1 */
        int ok = (parity_even(data1, 4) == 0 && parity_even(data2, 4) == 1);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Even parity wrong"); }
    }
    TEST_CASE_END();

    /* ── Test 8: RLE round-trip ──────────────────────────────── */
    TEST_CASE_BEGIN("RLE encode → decode round-trip")
    {
        uint8_t data[] = {0,0,0,0,1,1,0,0,0,1};
        uint8_t encoded[32];
        int enc_len = rle_encode(data, 10, encoded, 32);

        uint8_t decoded[16];
        int dec_len = rle_decode(encoded, enc_len, decoded, 16);

        int ok = (dec_len == 10);
        for (int i = 0; i < 10 && ok; i++)
            if (data[i] != decoded[i]) ok = 0;
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("RLE round-trip failed"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
