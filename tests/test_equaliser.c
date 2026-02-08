/**
 * @file test_equaliser.c
 * @brief Unit tests for channel equalisation functions.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/equaliser.h"

int main(void)
{
    TEST_SUITE("Channel Equalisation");
    rng_seed(333);

    /* ── Test 1: ZF flat channel recovers signal ─────────────── */
    TEST_CASE_BEGIN("ZF flat channel: h=(2,0) -> recovers amplitude")
    {
        Cplx h = cplx(2.0, 0.0);
        Cplx rx[4] = {{2,0},{-2,0},{2,0},{-2,0}};
        Cplx eq[4];
        eq_zf_flat(rx, h, 4, eq);

        int ok = 1;
        for (int i = 0; i < 4; i++) {
            double expected = (i % 2 == 0) ? 1.0 : -1.0;
            if (fabs(eq[i].re - expected) > 0.001) ok = 0;
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("ZF flat equalisation wrong"); }
    }
    TEST_CASE_END();

    /* ── Test 2: ZF frequency domain ─────────────────────────── */
    TEST_CASE_BEGIN("ZF frequency domain equalisation")
    {
        Cplx H[4] = {{2,0},{1,1},{0.5,0},{1,-1}};
        Cplx rx[4] = {{4,0},{2,2},{1,0},{2,-2}};
        Cplx eq[4];
        eq_zf_freq(rx, H, 4, eq);

        int ok = 1;
        for (int i = 0; i < 4; i++) {
            if (fabs(eq[i].re - 2.0) > 0.01 || fabs(eq[i].im) > 0.01)
                ok = 0;
        }
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("ZF freq eq should give (2,0)"); }
    }
    TEST_CASE_END();

    /* ── Test 3: LMS equaliser converges ─────────────────────── */
    TEST_CASE_BEGIN("LMS equaliser converges on known channel")
    {
        LmsEqualiser lms;
        eq_lms_init(&lms, 5, 0.01);

        double mse = 0;
        for (int i = 0; i < 200; i++) {
            double x = (i % 2 == 0) ? 1.0 : -1.0;
            double desired = (i >= 2) ? ((i - 2) % 2 == 0 ? 1.0 : -1.0) : 0.0;
            Cplx sample = cplx(x, 0);
            Cplx ref = cplx(desired, 0);
            Cplx err;
            Cplx out = eq_lms_step(&lms, sample, ref, &err);
            if (i > 150) {
                double e = fabs(out.re - desired);
                mse += e * e;
            }
        }
        mse /= 50.0;
        eq_lms_free(&lms);

        if (mse < 0.1) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("LMS MSE too high after convergence"); }
    }
    TEST_CASE_END();

    /* ── Test 4: RLS equaliser init ──────────────────────────── */
    TEST_CASE_BEGIN("RLS equaliser initialises and runs")
    {
        RlsEqualiser rls;
        eq_rls_init(&rls, 5, 0.99, 1.0);

        Cplx err;
        Cplx out = eq_rls_step(&rls, cplx(1,0), cplx(1,0), &err);
        eq_rls_free(&rls);

        int ok = isfinite(out.re) && isfinite(out.im);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("RLS output not finite"); }
    }
    TEST_CASE_END();

    /* ── Test 5: DFE init and step ───────────────────────────── */
    TEST_CASE_BEGIN("DFE initialises and produces output")
    {
        DfeEqualiser dfe;
        eq_dfe_init(&dfe, 5, 3, 0.01);

        Cplx err;
        Cplx out = eq_dfe_step(&dfe, cplx(1,0), cplx(1,0), &err);
        eq_dfe_free(&dfe);

        int ok = isfinite(out.re) && isfinite(out.im);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("DFE output not finite"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
