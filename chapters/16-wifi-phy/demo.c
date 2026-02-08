/**
 * @file demo.c
 * @brief Chapter 16 — 802.11a/g Wi-Fi OFDM PHY
 *
 * Build:  make build/bin/16-wifi-phy
 * Run:    ./build/bin/16-wifi-phy
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"
#include "../../include/channel.h"

int main(void)
{
    rng_seed(16);
    print_separator("Chapter 16: 802.11a/g Wi-Fi OFDM PHY");

    printf("Wi-Fi PPDU Structure:\n");
    printf("  [STS 160] [LTS 160] [SIGNAL] [DATA ...]\n\n");

    /* Short training sequence */
    Cplx sts[160];
    int sts_len;
    wifi_short_training(sts, &sts_len);
    printf("  Short Training Seq: %d samples\n", sts_len);

    double sts_power = signal_power(sts, sts_len);
    printf("  STS avg power: %.4f\n\n", sts_power);

    /* Long training sequence */
    Cplx lts[160];
    int lts_len;
    wifi_long_training(lts, &lts_len);
    printf("  Long Training Seq: %d samples\n", lts_len);

    double lts_power = signal_power(lts, lts_len);
    printf("  LTS avg power: %.4f\n\n", lts_power);

    /* Scrambler */
    printf("  Wi-Fi Scrambler (x^7 + x^4 + 1):\n");
    uint8_t data[16];
    random_bits(data, 16);
    printf("    Before: ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    wifi_scramble(0x5D, data, 16);
    printf("\n    After:  ");
    for (int i = 0; i < 16; i++) printf("%d", data[i]);
    printf("\n\n");

    /* Build full PPDU */
    uint8_t payload[10] = {0x48, 0x65, 0x6C, 0x6C, 0x6F,
                           0x57, 0x69, 0x46, 0x69, 0x21};
    Cplx ppdu[4096];
    int ppdu_len = wifi_build_ppdu(payload, 10, WIFI_RATE_6, ppdu);
    printf("  Full PPDU: %d samples (%.1f µs at 20 MHz)\n",
           ppdu_len, (double)ppdu_len / 20.0);

    print_separator("End of Chapter 16");
    return 0;
}
