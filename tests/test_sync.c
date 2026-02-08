/**
 * @file test_sync.c
 * @brief Unit tests for synchronisation functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/sync.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int main(void)
{
    TEST_SUITE("Synchronisation");
    rng_seed(99);

    /* ── Test 1: Barker-13 autocorrelation peak ──────────────── */
    TEST_CASE_BEGIN("Barker-13 frame sync detection")
    {
        double signal[100];
        for (int i = 0; i < 100; i++) signal[i] = 0.0;
        for (int i = 0; i < 13; i++)
            signal[20 + i] = BARKER_13[i];

        int pos = frame_sync_detect(signal, 100, BARKER_13, 13, 0.8);
        if (pos >= 19 && pos <= 21) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Barker-13 not detected at correct position"); }
    }
    TEST_CASE_END();

    /* ── Test 2: Scrambler is own inverse ────────────────────── */
    TEST_CASE_BEGIN("Scrambler is self-inverse")
    {
        uint8_t data[32];
        random_bits(data, 32);

        uint8_t original[32];
        for (int i = 0; i < 32; i++) original[i] = data[i];

        scrambler(0x48, 0x7F, data, 32);
        scrambler(0x48, 0x7F, data, 32);

        int ok = 1;
        for (int i = 0; i < 32; i++) {
            if (data[i] != original[i]) { ok = 0; break; }
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Scrambler not self-inverse"); }
    }
    TEST_CASE_END();

    /* ── Test 3: Carrier sync init ───────────────────────────── */
    TEST_CASE_BEGIN("Carrier sync struct initialises cleanly")
    {
        CarrierSync cs;
        carrier_init(&cs, 0.01, 0.707);
        int ok = (fabs(cs.phase) < 0.001 && fabs(cs.freq) < 0.001);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("CarrierSync init not zeroed"); }
    }
    TEST_CASE_END();

    /* ── Test 4: Timing recovery init ────────────────────────── */
    TEST_CASE_BEGIN("Timing recovery struct initialises")
    {
        TimingRecovery tr;
        timing_init(&tr, 4, 0.01, 0.707);
        int ok = (tr.sps == 4);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("TimingRecovery sps not set"); }
    }
    TEST_CASE_END();

    /* ── Test 5: Frame sync correlate at known position ──────── */
    TEST_CASE_BEGIN("Frame sync correlate returns correct pattern")
    {
        double sig[32];
        for (int i = 0; i < 32; i++) sig[i] = 0.0;

        for (int i = 0; i < 7; i++) sig[10 + i] = BARKER_7[i];

        double corr[32];
        frame_sync_correlate(sig, 32, BARKER_7, 7, corr);

        double max_val = 0;
        int max_pos = 0;
        for (int i = 0; i < 32; i++) {
            if (corr[i] > max_val) { max_val = corr[i]; max_pos = i; }
        }
        if (max_pos == 10 && max_val > 6.5) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Correlation peak not at position 10"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
