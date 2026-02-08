/**
 * @file test_channel.c
 * @brief Unit tests for channel model functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/channel.h"

int main(void)
{
    TEST_SUITE("Channel Models");
    rng_seed(77);

    /* ── Test 1: AWGN adds noise ─────────────────────────────── */
    TEST_CASE_BEGIN("AWGN channel adds noise at specified SNR")
    {
        int N = 1024;
        Cplx tx[1024], rx[1024];
        for (int i = 0; i < N; i++) tx[i] = cplx(1.0, 0.0);

        channel_awgn(tx, N, 20.0, rx);

        double pwr = signal_power(rx, N);
        if (fabs(pwr - 1.0) < 0.2) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("AWGN power not as expected"); }
    }
    TEST_CASE_END();

    /* ── Test 2: Eb/N0 → SNR conversion ─────────────────────── */
    TEST_CASE_BEGIN("Eb/N0 to SNR round-trip")
    {
        double ebn0 = 10.0;
        double snr = ebn0_to_snr(ebn0, 2, 1.0, 1);
        double ebn0_back = snr_to_ebn0(snr, 2, 1.0, 1);
        if (fabs(ebn0 - ebn0_back) < 0.001) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Eb/N0 <-> SNR conversion mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 3: Signal power of known signal ────────────────── */
    TEST_CASE_BEGIN("Signal power of unit complex signal = 1.0")
    {
        Cplx s[4] = {{1,0},{0,1},{-1,0},{0,-1}};
        double p = signal_power(s, 4);
        if (fabs(p - 1.0) < 0.001) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Expected power = 1.0"); }
    }
    TEST_CASE_END();

    /* ── Test 4: AWGN real channel ───────────────────────────── */
    TEST_CASE_BEGIN("AWGN real channel adds noise")
    {
        int N = 512;
        double tx[512], rx[512];
        for (int i = 0; i < N; i++) tx[i] = 1.0;

        channel_awgn_real(tx, N, 30.0, rx);

        double mean = 0;
        for (int i = 0; i < N; i++) mean += rx[i];
        mean /= N;
        if (fabs(mean - 1.0) < 0.1) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Mean should be near 1.0 at high SNR"); }
    }
    TEST_CASE_END();

    /* ── Test 5: Rayleigh fading changes signal ──────────────── */
    TEST_CASE_BEGIN("Rayleigh flat fading alters signal")
    {
        int N = 256;
        Cplx tx[256], rx[256], h_est[256];
        for (int i = 0; i < N; i++) tx[i] = cplx(1.0, 0.0);

        RayleighChannel rch = {.sigma = 1.0, .last_coeff = cplx(0,0)};
        channel_rayleigh_flat(&rch, tx, N, rx, h_est);

        int changed = 0;
        for (int i = 0; i < N; i++) {
            if (fabs(rx[i].re - 1.0) > 0.01 || fabs(rx[i].im) > 0.01)
                changed++;
        }
        if (changed > N / 2) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Rayleigh should change most samples"); }
    }
    TEST_CASE_END();

    /* ── Test 6: Multipath channel init ──────────────────────── */
    TEST_CASE_BEGIN("Multipath channel init and apply")
    {
        MultipathChannel mch;
        int delays[3] = {0, 1, 3};
        double gains_db[3] = {0.0, -6.0, -10.5};
        channel_multipath_init(&mch, 3, delays, gains_db);

        Cplx tx[32], rx[32];
        for (int i = 0; i < 32; i++) tx[i] = cplx(i == 0 ? 1.0 : 0.0, 0.0);
        int out_len;
        channel_multipath_apply(&mch, tx, 16, rx, &out_len);

        /* First sample should be non-zero (gain at delay 0) */
        int ok = (fabs(rx[0].re) > 0.1);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Multipath impulse response wrong"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
