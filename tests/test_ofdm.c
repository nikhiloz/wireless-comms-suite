/**
 * @file test_ofdm.c
 * @brief Unit tests for OFDM and FFT functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/ofdm.h"
#include "../include/modulation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void)
{
    TEST_SUITE("OFDM & FFT");
    rng_seed(55);

    /* ── Test 1: FFT of DC signal ────────────────────────────── */
    TEST_CASE_BEGIN("FFT: DC signal -> all energy in bin 0")
    {
        Cplx x[8];
        for (int i = 0; i < 8; i++) x[i] = cplx(1.0, 0.0);
        fft(x, 8);

        int ok = (fabs(x[0].re - 8.0) < 0.001);
        for (int i = 1; i < 8; i++)
            if (cplx_mag2(x[i]) > 0.001) ok = 0;

        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("DC signal FFT failed"); }
    }
    TEST_CASE_END();

    /* ── Test 2: FFT→IFFT round-trip ─────────────────────────── */
    TEST_CASE_BEGIN("FFT -> IFFT recovers original signal")
    {
        Cplx x[16], orig[16];
        for (int i = 0; i < 16; i++) {
            x[i] = cplx(rng_uniform() * 2 - 1, rng_uniform() * 2 - 1);
            orig[i] = x[i];
        }
        fft(x, 16);
        ifft(x, 16);

        int ok = 1;
        for (int i = 0; i < 16; i++) {
            if (fabs(x[i].re - orig[i].re) > 0.001 ||
                fabs(x[i].im - orig[i].im) > 0.001) {
                ok = 0; break;
            }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("FFT->IFFT round-trip failed"); }
    }
    TEST_CASE_END();

    /* ── Test 3: OFDM init creates valid params ──────────────── */
    TEST_CASE_BEGIN("OFDM init: 64-pt, CP=16, 4 pilots")
    {
        OfdmParams ofdm;
        ofdm_init(&ofdm, 64, 16, 4);

        int ok = (ofdm.n_fft == 64 && ofdm.n_cp == 16 &&
                  ofdm.n_pilot == 4 && ofdm.n_data > 0);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("OFDM init parameters wrong"); }
    }
    TEST_CASE_END();

    /* ── Test 4: OFDM modulate → demodulate round-trip ───────── */
    TEST_CASE_BEGIN("OFDM single symbol round-trip (BPSK)")
    {
        OfdmParams ofdm;
        ofdm_init(&ofdm, 64, 16, 4);

        Cplx data_in[64];
        for (int i = 0; i < ofdm.n_data; i++)
            data_in[i] = (rng_uniform() > 0.5) ? cplx(1,0) : cplx(-1,0);

        Cplx time_samples[80];
        ofdm_modulate(&ofdm, data_in, time_samples);

        Cplx data_out[64];
        ofdm_demodulate(&ofdm, time_samples, data_out, NULL);

        int ok = 1;
        for (int i = 0; i < ofdm.n_data; i++) {
            if (fabs(data_out[i].re - data_in[i].re) > 0.01 ||
                fabs(data_out[i].im - data_in[i].im) > 0.01) {
                ok = 0; break;
            }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("OFDM round-trip mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 5: FFT impulse → flat spectrum ─────────────────── */
    TEST_CASE_BEGIN("FFT impulse gives flat magnitude spectrum")
    {
        Cplx x[8] = {{1,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
        fft(x, 8);

        int ok = 1;
        for (int k = 0; k < 8; k++) {
            double mag = sqrt(cplx_mag2(x[k]));
            if (fabs(mag - 1.0) > 0.001) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Impulse should give flat spectrum"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
