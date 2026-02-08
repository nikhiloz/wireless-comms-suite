/**
 * @file test_spread.c
 * @brief Unit tests for spread spectrum functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/spread_spectrum.h"

int main(void)
{
    TEST_SUITE("Spread Spectrum");
    rng_seed(200);

    /* ── Test 1: M-sequence length = 2^n - 1 ────────────────── */
    TEST_CASE_BEGIN("M-sequence length for 5-bit LFSR = 31")
    {
        int seq[31];
        int len = pn_msequence(0x12, 5, seq); /* poly, n_bits=degree */
        if (len == 31) { TEST_PASS_STMT; }
        else           { TEST_FAIL_STMT("Expected length 31"); }
    }
    TEST_CASE_END();

    /* ── Test 2: M-sequence is +/-1 valued ───────────────────── */
    TEST_CASE_BEGIN("M-sequence values are +1 or -1")
    {
        int seq[63];
        int len = pn_msequence(0x21, 6, seq); /* 6-bit LFSR → 63 chips */
        int ok = 1;
        for (int i = 0; i < len; i++) {
            if (seq[i] != 1 && seq[i] != -1) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Values should be +/-1"); }
    }
    TEST_CASE_END();

    /* ── Test 3: DSSS spread → despread round-trip ───────────── */
    TEST_CASE_BEGIN("DSSS spread -> despread recovers data")
    {
        uint8_t data[8];
        random_bits(data, 8);

        int code[7];
        pn_msequence(0x05, 3, code); /* 3-bit LFSR → 7 chips */
        int chip_len = 7;

        double spread[56];
        dsss_spread(data, 8, code, chip_len, spread);

        uint8_t despread[8];
        dsss_despread(spread, 56, code, chip_len, despread);

        int ok = 1;
        for (int i = 0; i < 8; i++) {
            if (data[i] != despread[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("DSSS round-trip failed"); }
    }
    TEST_CASE_END();

    /* ── Test 4: Processing gain calculation ─────────────────── */
    TEST_CASE_BEGIN("DSSS processing gain (chip=31) ~ 14.9 dB")
    {
        double pg = dsss_processing_gain_db(31);
        if (fabs(pg - 14.91) < 0.1) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Processing gain not ~14.9 dB"); }
    }
    TEST_CASE_END();

    /* ── Test 5: Gold code length ────────────────────────────── */
    TEST_CASE_BEGIN("Gold code length for n_bits=31 is 31")
    {
        int gold[31];
        int len = pn_gold(0x12, 0x1E, 5, 0, gold); /* 5-bit register → 31 chips */
        if (len == 31) { TEST_PASS_STMT; }
        else           { TEST_FAIL_STMT("Expected Gold length 31"); }
    }
    TEST_CASE_END();

    /* ── Test 6: FHSS channel hopping ────────────────────────── */
    TEST_CASE_BEGIN("FHSS produces valid channel indices")
    {
        FhssParams fh;
        fhss_init(&fh, 20, 15, 12345, 0.001);

        int ok = 1;
        for (int i = 0; i < 15; i++) {
            int ch = fhss_get_channel(&fh, i);
            if (ch < 0 || ch >= 20) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("FHSS channel out of range"); }
    }
    TEST_CASE_END();

    /* ── Test 7: Zigbee chip map gives 32 chips ──────────────── */
    TEST_CASE_BEGIN("Zigbee chip map: symbol 0 -> 32 chips")
    {
        int chips[32];
        zigbee_chip_map(0, chips);
        int ok = 1;
        for (int i = 0; i < 32; i++) {
            if (chips[i] != 1 && chips[i] != -1) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Chip values should be +/-1"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
