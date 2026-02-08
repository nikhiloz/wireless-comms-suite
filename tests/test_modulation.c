/**
 * @file test_modulation.c
 * @brief Unit tests for modulation / demodulation functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/modulation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void)
{
    TEST_SUITE("Modulation / Demodulation");
    rng_seed(42);

    /* ── Test 1: BPSK round-trip ─────────────────────────────── */
    TEST_CASE_BEGIN("BPSK modulate → demodulate round-trip")
    {
        uint8_t bits[16];
        random_bits(bits, 16);

        Cplx syms[16];
        mod_modulate(MOD_BPSK, bits, 16, syms);

        uint8_t recovered[16];
        mod_demodulate(MOD_BPSK, syms, 16, recovered);

        int ok = 1;
        for (int i = 0; i < 16; i++) {
            if (bits[i] != recovered[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("BPSK round-trip mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 2: QPSK round-trip ─────────────────────────────── */
    TEST_CASE_BEGIN("QPSK modulate → demodulate round-trip")
    {
        uint8_t bits[32];
        random_bits(bits, 32);

        int nsyms = 32 / 2;
        Cplx syms[16];
        mod_modulate(MOD_QPSK, bits, 32, syms);

        uint8_t recovered[32];
        mod_demodulate(MOD_QPSK, syms, nsyms, recovered);

        int ok = 1;
        for (int i = 0; i < 32; i++) {
            if (bits[i] != recovered[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("QPSK round-trip mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 3: 16-QAM round-trip ───────────────────────────── */
    TEST_CASE_BEGIN("16-QAM modulate → demodulate round-trip")
    {
        uint8_t bits[64];
        random_bits(bits, 64);

        int nsyms = 64 / 4;
        Cplx syms[16];
        mod_modulate(MOD_16QAM, bits, 64, syms);

        uint8_t recovered[64];
        mod_demodulate(MOD_16QAM, syms, nsyms, recovered);

        int ok = 1;
        for (int i = 0; i < 64; i++) {
            if (bits[i] != recovered[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("16-QAM round-trip mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 4: BPSK BER theory at high SNR ≈ 0 ───────────── */
    TEST_CASE_BEGIN("BPSK theoretical BER at 10 dB ≈ 3.87e-6")
    {
        double ber = ber_bpsk_theory(10.0);
        if (ber < 1e-4 && ber > 1e-8) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Unexpected BPSK BER"); }
    }
    TEST_CASE_END();

    /* ── Test 5: Raised cosine symmetry ──────────────────────── */
    TEST_CASE_BEGIN("Raised cosine filter is symmetric")
    {
        double h[41];
        int flen = raised_cosine(0.35, 4, 10, h); /* alpha=0.35, sps=4, span=10 → 41 taps */
        int ok = (flen == 41);
        for (int i = 0; i < flen / 2 && ok; i++) {
            if (fabs(h[i] - h[flen - 1 - i]) > 1e-10) { ok = 0; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("RC filter should be symmetric"); }
    }
    TEST_CASE_END();

    /* ── Test 6: BPSK constellation values ───────────────────── */
    TEST_CASE_BEGIN("BPSK constellation: ±1 on real axis")
    {
        Cplx pts[2];
        int n = mod_constellation(MOD_BPSK, pts);
        int ok = (n == 2);
        ok = ok && (fabs(pts[0].re - (-1.0)) < 0.01 && fabs(pts[0].im) < 0.01);
        ok = ok && (fabs(pts[1].re - 1.0) < 0.01 && fabs(pts[1].im) < 0.01);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("BPSK should be +1 and -1"); }
    }
    TEST_CASE_END();

    /* ── Test 7: NRZ encoding ────────────────────────────────── */
    TEST_CASE_BEGIN("NRZ encode: 0→-1, 1→+1")
    {
        uint8_t bits[4] = {0, 1, 0, 1};
        double nrz[4];
        nrz_encode(bits, 4, nrz);
        int ok = (nrz[0] == -1.0 && nrz[1] == 1.0 &&
                  nrz[2] == -1.0 && nrz[3] == 1.0);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("NRZ encoding incorrect"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
