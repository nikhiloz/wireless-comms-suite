/**
 * @file demo.c
 * @brief Chapter 20 — ADS-B / Mode S PHY
 *
 * Build:  make build/bin/20-adsb-phy
 * Run:    ./build/bin/20-adsb-phy
 */

#include <stdio.h>
#include <string.h>
#include "../../include/comms_utils.h"
#include "../../include/phy.h"

int main(void)
{
    rng_seed(20);
    print_separator("Chapter 20: ADS-B / Mode S PHY");

    printf("1090 MHz, 1 Mbit/s PPM, 112-bit message\n\n");

    /* ── Encode ADS-B message ────────────────────────────────── */
    printf("1. ADS-B Message Encoding\n");
    AdsbMessage msg;
    msg.df = 17;           /* DF17 = ADS-B */
    msg.ca = 5;            /* Capability */
    msg.icao = 0x4840D6;   /* Example ICAO */
    memset(msg.msg, 0, sizeof(msg.msg));
    msg.msg[0] = 0x58;     /* Type code 11: airborne position */
    msg.msg[1] = 0xB9;
    msg.msg[2] = 0x86;
    msg.msg[3] = 0xD0;
    msg.msg[4] = 0xA3;
    msg.msg[5] = 0x21;
    msg.msg[6] = 0x09;

    printf("   DF=%d, CA=%d, ICAO=0x%06X\n", msg.df, msg.ca, msg.icao);

    uint8_t bits[112];
    adsb_encode(&msg, bits);
    printf("   Encoded 112 bits (incl. CRC-24)\n");

    /* ── PPM modulation ──────────────────────────────────────── */
    printf("\n2. PPM Modulation (2 MHz sample rate)\n");
    double samples[240];
    int nsamp = adsb_modulate(bits, samples);
    printf("   Preamble: 8 µs (16 samples)\n");
    printf("   Data: 112 µs (224 samples)\n");
    printf("   Total: %d samples = 120 µs\n", nsamp);

    /* ── Demodulation round-trip ─────────────────────────────── */
    printf("\n3. Demodulation & CRC Verification\n");
    AdsbMessage rx_msg;
    int rc = adsb_demodulate(samples, nsamp, &rx_msg);
    printf("   Decoded ICAO: 0x%06X\n", rx_msg.icao);
    printf("   CRC check: %s\n", (rc == 0) ? "PASS" : "FAIL");
    printf("   DF=%d, CA=%d\n", rx_msg.df, rx_msg.ca);

    /* ── CRC-24 standalone ───────────────────────────────────── */
    printf("\n4. CRC-24 (ADS-B polynomial)\n");
    uint32_t crc = adsb_crc24(bits, 88);
    printf("   CRC-24 of first 88 bits: 0x%06X\n", crc);
    printf("   Message CRC: 0x%06X\n", rx_msg.crc);

    print_separator("End of Chapter 20");
    return 0;
}
