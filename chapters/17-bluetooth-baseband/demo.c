/**
 * @file demo.c
 * @brief Chapter 17 — Bluetooth Baseband (GFSK, Whitening, Packets)
 *
 * Build:  make build/bin/17-bluetooth-baseband
 * Run:    ./build/bin/17-bluetooth-baseband
 */

#include <stdio.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"
#include "../../include/channel.h"
#include "../../include/modulation.h"

int main(void)
{
    rng_seed(17);
    print_separator("Chapter 17: Bluetooth Baseband");

    /* ── Access code generation ───────────────────────────────── */
    printf("1. Access Code (LAP = 0x9E8B33)\n");
    uint8_t ac[BT_ACCESS_CODE_LEN];
    bt_gen_access_code(0x9E8B33, ac);
    printf("   AC (72 bits): ");
    for (int i = 0; i < 20; i++) printf("%d", ac[i]);
    printf("... (first 20)\n\n");

    /* ── Data whitening ──────────────────────────────────────── */
    printf("2. Data Whitening (clock6 = 0x3F)\n");
    uint8_t data[16];
    random_bits(data, 16);
    printf("   Before: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    bt_whiten(0x3F, data, 16);
    printf("\n   After:  ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    bt_whiten(0x3F, data, 16);
    printf("\n   Dewhit: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    printf("\n\n");

    /* ── GFSK modulation ─────────────────────────────────────── */
    printf("3. BT Classic GFSK (BT=0.5, h=0.32)\n");
    uint8_t tx_data[5] = {0xA5, 0x3C, 0x7E, 0x01, 0xFF};
    BtPacketConfig cfg;
    cfg.mode = BT_CLASSIC;
    cfg.lap = 0x9E8B33;
    memcpy(cfg.access_code, ac, BT_ACCESS_CODE_LEN);

    Cplx iq[8192];
    int n_samples = bt_build_packet(&cfg, tx_data, 5, 8, iq);
    printf("   Payload: 5 bytes → %d IQ samples (8 sps)\n", n_samples);

    /* Power profile */
    double pwr = signal_power(iq, (n_samples > 0) ? n_samples : 1);
    printf("   Average power: %.4f\n", pwr);

    print_separator("End of Chapter 17");
    return 0;
}
