/**
 * @file demo.c
 * @brief Chapter 21 — Link Budget (Friis, Noise Figure, Fade Margin)
 *
 * Build:  make build/bin/21-link-budget
 * Run:    ./build/bin/21-link-budget
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"

int main(void)
{
    rng_seed(21);
    print_separator("Chapter 21: Link Budget Analysis");

    /* ── FSPL table ──────────────────────────────────────────── */
    printf("1. Free Space Path Loss\n\n");
    printf("              100m     1km      10km     100km\n");
    printf("  ────────    ─────    ─────    ─────    ─────\n");
    double freqs[] = {433e6, 868e6, 2.4e9, 5.8e9};
    const char *fnames[] = {"433 MHz ", "868 MHz ", "2.4 GHz ", "5.8 GHz "};
    double dists[] = {100, 1000, 10000, 100000};

    for (int f = 0; f < 4; f++) {
        printf("  %s  ", fnames[f]);
        for (int d = 0; d < 4; d++)
            printf(" %5.1f    ", link_fspl_db(dists[d], freqs[f]));
        printf("\n");
    }

    /* ── Friis equation ──────────────────────────────────────── */
    printf("\n2. Friis Link Budget (Wi-Fi Example)\n");
    double pt = 20.0;          /* 20 dBm TX power */
    double gt = 3.0, gr = 3.0; /* 3 dBi antennas */
    double freq = 2.4e9;
    printf("   TX Power: %.0f dBm, Antennas: %.0f dBi each, f=2.4 GHz\n\n", pt, gt);
    printf("   Distance    Rx Power    FSPL\n");
    printf("   ────────    ────────    ────\n");
    double test_dists[] = {1, 10, 50, 100, 500};
    for (int i = 0; i < 5; i++) {
        double rx_pwr = link_friis_dbm(pt, gt, gr, test_dists[i], freq);
        double fspl = link_fspl_db(test_dists[i], freq);
        printf("   %5.0f m     %+7.1f dBm  %.1f dB\n",
               test_dists[i], rx_pwr, fspl);
    }

    /* ── Noise floor ─────────────────────────────────────────── */
    printf("\n3. Noise Floor Analysis\n");
    struct { const char* name; double bw; double nf; } systems[] = {
        {"LoRa SF12",    7812.5,  6.0},
        {"Zigbee",      2e6,     6.0},
        {"Bluetooth",   1e6,     8.0},
        {"Wi-Fi (20M)", 20e6,    5.0},
    };
    printf("   System          BW         NF    Noise Floor\n");
    printf("   ──────          ──         ──    ──────────\n");
    for (int i = 0; i < 4; i++) {
        double nf_dbm = link_noise_floor_dbm(systems[i].bw, systems[i].nf);
        printf("   %-14s  %8.0f Hz  %.0f dB  %+.1f dBm\n",
               systems[i].name, systems[i].bw, systems[i].nf, nf_dbm);
    }

    /* ── Required Eb/N0 ──────────────────────────────────────── */
    printf("\n4. Required Eb/N0 for Target BER\n");
    double bers[] = {1e-3, 1e-4, 1e-5, 1e-6};
    for (int i = 0; i < 4; i++) {
        double ebn0 = link_required_ebn0(bers[i]);
        printf("   BER = %.0e → Eb/N0 ≈ %.1f dB\n", bers[i], ebn0);
    }

    print_separator("End of Chapter 21");
    return 0;
}
