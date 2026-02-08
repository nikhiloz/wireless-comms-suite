/**
 * @file test_phy.c
 * @brief Unit tests for protocol PHY implementations.
 *
 * Run with: make test
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "test_framework.h"
#include "../include/comms_utils.h"
#include "../include/phy.h"

int main(void)
{
    TEST_SUITE("Protocol PHY");
    rng_seed(444);

    /* ── Test 1: Wi-Fi short training sequence length ────────── */
    TEST_CASE_BEGIN("Wi-Fi STS generates 160 samples")
    {
        Cplx sts[160];
        int len;
        wifi_short_training(sts, &len);
        if (len == 160) { TEST_PASS_STMT; }
        else            { TEST_FAIL_STMT("STS should be 160 samples"); }
    }
    TEST_CASE_END();

    /* ── Test 2: Wi-Fi long training sequence length ─────────── */
    TEST_CASE_BEGIN("Wi-Fi LTS generates 160 samples")
    {
        Cplx lts[160];
        int len;
        wifi_long_training(lts, &len);
        if (len == 160) { TEST_PASS_STMT; }
        else            { TEST_FAIL_STMT("LTS should be 160 samples"); }
    }
    TEST_CASE_END();

    /* ── Test 3: Wi-Fi scrambler is self-inverse ─────────────── */
    TEST_CASE_BEGIN("Wi-Fi scrambler is self-inverse")
    {
        uint8_t data[40];
        random_bits(data, 40);
        uint8_t original[40];
        memcpy(original, data, 40);

        wifi_scramble(0x5D, data, 40);
        wifi_scramble(0x5D, data, 40);

        int ok = (memcmp(data, original, 40) == 0);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Scrambler not self-inverse"); }
    }
    TEST_CASE_END();

    /* ── Test 4: Bluetooth whitening is self-inverse ─────────── */
    TEST_CASE_BEGIN("BT whitening is self-inverse")
    {
        uint8_t data[32];
        random_bits(data, 32);
        uint8_t original[32];
        memcpy(original, data, 32);

        bt_whiten(0x3F, data, 32);
        bt_whiten(0x3F, data, 32);

        int ok = (memcmp(data, original, 32) == 0);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("BT whitening not self-inverse"); }
    }
    TEST_CASE_END();

    /* ── Test 5: ADS-B encode → modulate → demodulate ────────── */
    TEST_CASE_BEGIN("ADS-B encode -> modulate -> demodulate round-trip")
    {
        AdsbMessage tx_msg;
        tx_msg.df = 17;
        tx_msg.ca = 5;
        tx_msg.icao = 0xABCDEF;
        memset(tx_msg.msg, 0, sizeof(tx_msg.msg));
        tx_msg.msg[0] = 0x58; tx_msg.msg[1] = 0xB9;

        uint8_t bits[112];
        adsb_encode(&tx_msg, bits);

        double samples[240];
        int n_samp = adsb_modulate(bits, samples);

        AdsbMessage rx_msg;
        int rc = adsb_demodulate(samples, n_samp, &rx_msg);

        int ok = (rc == 0 && rx_msg.icao == 0xABCDEF && rx_msg.df == 17);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("ADS-B round-trip failed"); }
    }
    TEST_CASE_END();

    /* ── Test 6: LoRa modulate → demodulate ──────────────────── */
    TEST_CASE_BEGIN("LoRa mod -> demod recovers symbol")
    {
        LoraParams lp;
        lora_init(&lp, 7, 125000, 1);

        int symbol_in = 42;
        Cplx chirp[128];
        lora_modulate_symbol(&lp, symbol_in, chirp);

        int symbol_out = lora_demodulate_symbol(&lp, chirp);
        if (symbol_out == symbol_in) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("LoRa symbol mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 7: MIMO Alamouti encode → decode ───────────────── */
    TEST_CASE_BEGIN("Alamouti STBC encode -> decode")
    {
        Cplx s[2] = {{1.0, 0.5}, {-0.5, 1.0}};
        Cplx tx0[2], tx1[2];
        mimo_alamouti_encode(s, tx0, tx1);

        Cplx h0 = cplx(1,0), h1 = cplx(1,0);
        Cplx rx[2];
        rx[0] = cplx_add(cplx_mul(h0, tx0[0]), cplx_mul(h1, tx1[0]));
        rx[1] = cplx_add(cplx_mul(h0, tx0[1]), cplx_mul(h1, tx1[1]));

        Cplx s_hat[2];
        mimo_alamouti_decode(rx, h0, h1, s_hat);

        int ok = (fabs(s_hat[0].re - s[0].re) < 0.01 &&
                  fabs(s_hat[0].im - s[0].im) < 0.01 &&
                  fabs(s_hat[1].re - s[1].re) < 0.01 &&
                  fabs(s_hat[1].im - s[1].im) < 0.01);
        if (ok) { TEST_PASS_STMT; }
        else    { TEST_FAIL_STMT("Alamouti decode mismatch"); }
    }
    TEST_CASE_END();

    /* ── Test 8: Link budget FSPL ────────────────────────────── */
    TEST_CASE_BEGIN("FSPL at 1km, 2.4 GHz ~ 100.0 dB")
    {
        double fspl = link_fspl_db(1000.0, 2.4e9);
        if (fabs(fspl - 100.0) < 1.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("FSPL should be ~100 dB"); }
    }
    TEST_CASE_END();

    /* ── Test 9: Noise floor at 1 MHz BW ────────────────────── */
    TEST_CASE_BEGIN("Noise floor at 1MHz BW, NF=3 dB ~ -111 dBm")
    {
        double nf = link_noise_floor_dbm(1e6, 3.0);
        if (fabs(nf - (-111.0)) < 1.0) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("Noise floor should be ~-111 dBm"); }
    }
    TEST_CASE_END();

    /* ── Test 10: MRC combining ──────────────────────────────── */
    TEST_CASE_BEGIN("MRC combines 2-antenna signal correctly")
    {
        Cplx h[2] = {{1,0},{0,1}};
        Cplx rx[2] = {{2,0},{0,2}};
        Cplx y = mimo_mrc(rx, h, 2);
        if (fabs(y.re - 2.0) < 0.01 && fabs(y.im) < 0.01) { TEST_PASS_STMT; }
        else { TEST_FAIL_STMT("MRC output should be (2,0)"); }
    }
    TEST_CASE_END();

    TEST_SUMMARY();
}
