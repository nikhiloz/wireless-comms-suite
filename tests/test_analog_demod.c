/**
 * @file test_analog_demod.c
 * @brief Unit tests for analog_demod module (FM, AM, SSB).
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "../include/comms_utils.h"
#include "../include/analog_demod.h"
#include "test_framework.h"

int main(void)
{
    rng_seed(250);
    TEST_SUITE("Analog Demod");

    /* ── Test 1: FM modulate → demodulate round-trip ─────────── */
    TEST_CASE_BEGIN("FM mod/demod round-trip") {
        int n = 512;
        double *audio = malloc(n * sizeof(double));
        Cplx   *iq    = malloc(n * sizeof(Cplx));
        double *demod  = malloc(n * sizeof(double));

        /* 1 kHz tone, fs=48 kHz, deviation=0.25 */
        double freq_dev = 0.25;
        for (int i = 0; i < n; i++)
            audio[i] = 0.8 * sin(2.0 * M_PI * 1000.0 / 48000.0 * i);

        fm_modulate(audio, n, freq_dev, iq);
        int nd = fm_demodulate(iq, n, demod);
        TEST_ASSERT(nd == n - 1);

        /* Discriminator output should track audio scaled by freq_dev */
        double max_err = 0.0;
        for (int i = 10; i < nd; i++) {
            double expected = audio[i + 1] * (2.0 * freq_dev);
            double err = fabs(demod[i] - expected);
            if (err > max_err) max_err = err;
        }
        /* FM round-trip should be very accurate without noise */
        TEST_ASSERT(max_err < 0.05);

        free(audio); free(iq); free(demod);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 2: AM modulate → envelope detect ───────────────── */
    TEST_CASE_BEGIN("AM mod/envelope-detect round-trip") {
        int n = 1024;
        double *audio  = malloc(n * sizeof(double));
        Cplx   *am_iq  = malloc(n * sizeof(Cplx));
        double *env    = malloc(n * sizeof(double));

        /* Low-freq tone so envelope is clean */
        for (int i = 0; i < n; i++)
            audio[i] = 0.5 * sin(2.0 * M_PI * 100.0 / 48000.0 * i);

        am_modulate(audio, n, 0.8, 0.15, am_iq);
        am_envelope_detect(am_iq, n, env);

        /* Correlation should be high (> 0.9) */
        double c = 0.0, pa = 0.0, pb = 0.0;
        for (int i = 0; i < n; i++) {
            c  += audio[i] * env[i];
            pa += audio[i] * audio[i];
            pb += env[i] * env[i];
        }
        double rho = c / sqrt(pa * pb);
        TEST_ASSERT(rho > 0.85);

        free(audio); free(am_iq); free(env);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 3: SSB modulate → demodulate round-trip ────────── */
    TEST_CASE_BEGIN("SSB USB mod/demod round-trip") {
        int n = 1024;
        double *audio = malloc(n * sizeof(double));
        Cplx   *ssb   = malloc(n * sizeof(Cplx));
        double *demod  = malloc(n * sizeof(double));

        for (int i = 0; i < n; i++)
            audio[i] = sin(2.0 * M_PI * 2000.0 / 48000.0 * i);

        ssb_modulate(audio, n, 1, 0.1, ssb);  /* USB */
        ssb_demodulate(ssb, n, 0.1, demod);

        /* Correlation in center portion (avoid edge effects from Hilbert) */
        double c = 0.0, pa = 0.0, pb = 0.0;
        int start = 50, end = n - 50;
        for (int i = start; i < end; i++) {
            c  += audio[i] * demod[i];
            pa += audio[i] * audio[i];
            pb += demod[i] * demod[i];
        }
        double rho = (pa > 0 && pb > 0) ? c / sqrt(pa * pb) : 0.0;
        TEST_ASSERT(rho > 0.80);

        free(audio); free(ssb); free(demod);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 4: Pre/de-emphasis inverse ─────────────────────── */
    TEST_CASE_BEGIN("Pre-emphasis / de-emphasis inverse") {
        int n = 512;
        double *audio = malloc(n * sizeof(double));
        double *pre   = malloc(n * sizeof(double));
        double *post  = malloc(n * sizeof(double));

        for (int i = 0; i < n; i++)
            audio[i] = 0.6 * sin(2.0 * M_PI * 3000.0 / 48000.0 * i);

        fm_preemphasis(audio, n, 75.0, 48000.0, pre);
        fm_deemphasis(pre, n, 75.0, 48000.0, post);

        double max_err = 0.0;
        for (int i = 20; i < n; i++) {   /* skip transient */
            double e = fabs(post[i] - audio[i]);
            if (e > max_err) max_err = e;
        }
        TEST_ASSERT(max_err < 0.01);

        free(audio); free(pre); free(post);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 5: Stereo pilot detection ──────────────────────── */
    TEST_CASE_BEGIN("Stereo pilot detection (19 kHz)") {
        int fs = 240000;
        int n = 4800;   /* 20 ms at 240 kHz */
        double *sig = calloc(n, sizeof(double));

        /* Insert 19 kHz pilot at moderate power */
        for (int i = 0; i < n; i++)
            sig[i] = 0.1 * sin(2.0 * M_PI * 19000.0 / fs * i)
                   + 0.5 * sin(2.0 * M_PI * 1000.0 / fs * i);

        double strength = fm_stereo_pilot_detect(sig, n, (double)fs);
        TEST_ASSERT(strength > 0.05);   /* pilot present */

        /* No pilot: only 1 kHz */
        for (int i = 0; i < n; i++)
            sig[i] = 0.5 * sin(2.0 * M_PI * 1000.0 / fs * i);
        double no_pilot = fm_stereo_pilot_detect(sig, n, (double)fs);
        TEST_ASSERT(no_pilot < strength);

        free(sig);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 6: Lowpass FIR filter ──────────────────────────── */
    TEST_CASE_BEGIN("Lowpass FIR removes high-freq noise") {
        int n = 512;
        double *sig = malloc(n * sizeof(double));
        double *out = malloc(n * sizeof(double));

        /* 100 Hz signal + 10 kHz noise, fs = 48 kHz */
        for (int i = 0; i < n; i++) {
            sig[i] = sin(2.0 * M_PI * 100.0 / 48000.0 * i)
                   + 0.5 * sin(2.0 * M_PI * 10000.0 / 48000.0 * i);
        }

        lowpass_fir(sig, n, 0.01, 31, out);  /* cutoff at ~480 Hz */

        /* Measure high-freq energy reduction */
        double hf_in = 0.0, hf_out = 0.0;
        for (int i = 50; i < n - 50; i++) {
            double lf = sin(2.0 * M_PI * 100.0 / 48000.0 * i);
            hf_in  += (sig[i] - lf) * (sig[i] - lf);
            hf_out += (out[i] - lf) * (out[i] - lf);
        }
        /* HF energy should be significantly reduced */
        TEST_ASSERT(hf_out < hf_in * 0.5);

        free(sig); free(out);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    /* ── Test 7: AM coherent demod ───────────────────────────── */
    TEST_CASE_BEGIN("AM coherent demod") {
        int n = 1024;
        double *audio  = malloc(n * sizeof(double));
        Cplx   *am_sig = malloc(n * sizeof(Cplx));
        double *demod  = malloc(n * sizeof(double));

        for (int i = 0; i < n; i++)
            audio[i] = 0.5 * sin(2.0 * M_PI * 500.0 / 48000.0 * i);

        double fc_norm = 0.15;
        am_modulate(audio, n, 0.8, fc_norm, am_sig);
        am_coherent_demod(am_sig, n, fc_norm, demod);

        /* Correlation should be high */
        double c = 0.0, pa = 0.0, pb = 0.0;
        for (int i = 0; i < n; i++) {
            c  += audio[i] * demod[i];
            pa += audio[i] * audio[i];
            pb += demod[i] * demod[i];
        }
        double rho = (pa > 0 && pb > 0) ? c / sqrt(pa * pb) : 0.0;
        TEST_ASSERT(rho > 0.90);

        free(audio); free(am_sig); free(demod);
        TEST_PASS_STMT;
    } TEST_CASE_END();

    TEST_SUMMARY();
}