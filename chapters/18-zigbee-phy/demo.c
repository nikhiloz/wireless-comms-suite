/**
 * @file demo.c
 * @brief Chapter 18 — Zigbee / IEEE 802.15.4 PHY
 *
 * Build:  make build/bin/18-zigbee-phy
 * Run:    ./build/bin/18-zigbee-phy
 */

#include <stdio.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"
#include "../../include/channel.h"
#include "../../include/spread_spectrum.h"

int main(void)
{
    rng_seed(18);
    print_separator("Chapter 18: Zigbee / IEEE 802.15.4 PHY");

    printf("2.4 GHz O-QPSK with DSSS (32 chips/symbol)\n\n");

    /* ── Chip mapping demonstration ──────────────────────────── */
    printf("1. Chip Mapping (4-bit symbol → 32 chips)\n");
    for (int sym = 0; sym < 4; sym++) {
        int chips[32];
        zigbee_chip_map(sym, chips);
        printf("   Symbol %d: ", sym);
        for (int c = 0; c < 32; c++)
            printf("%c", chips[c] > 0 ? '+' : '-');
        printf("\n");
    }
    printf("\n");

    /* ── Build PPDU ──────────────────────────────────────────── */
    printf("2. Zigbee PPDU Construction\n");
    uint8_t psdu[10] = {0x01, 0x88, 0x12, 0x34, 0xAB, 0xCD,
                        0xDE, 0xAD, 0xBE, 0xEF};
    Cplx ppdu[16384];
    int n_samples = zigbee_build_ppdu(psdu, 10, 8, ppdu);
    printf("   PSDU: 10 bytes\n");
    printf("   SHR:  4×0x00 preamble + 0xA7 SFD\n");
    printf("   PHR:  frame length = %d\n", 10);
    printf("   Total OQ-PSK samples: %d (8 sps)\n", n_samples);

    /* Power check */
    double pwr = signal_power(ppdu, (n_samples < 1000) ? n_samples : 1000);
    printf("   Average power: %.4f\n", pwr);

    print_separator("End of Chapter 18");
    return 0;
}
