/**
 * @file phy.c
 * @brief Protocol PHY implementations — Wi-Fi, Bluetooth, Zigbee, LoRa, ADS-B, MIMO, link budget.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Wi-Fi PHY        → chapters/16-wifi-phy/tutorial.md
 *   Bluetooth        → chapters/17-bluetooth-baseband/tutorial.md
 *   Zigbee           → chapters/18-zigbee-phy/tutorial.md
 *   LoRa             → chapters/19-lora-phy/tutorial.md
 *   ADS-B            → chapters/20-adsb-phy/tutorial.md
 *   Link budget      → chapters/21-link-budget/tutorial.md
 *   MIMO             → chapters/23-mimo/tutorial.md
 *
 * References:
 *   IEEE 802.11-2020, §17 (OFDM PHY)
 *   Bluetooth Core Spec v5.4, Vol. 6, Part B
 *   IEEE 802.15.4-2020, §12
 *   Semtech AN1200.22 (LoRa modulation basics)
 *   ICAO Annex 10, Vol. IV (Mode S / ADS-B)
 */

#include "../include/phy.h"
#include "../include/ofdm.h"
#include "../include/modulation.h"
#include "../include/coding.h"
#include "../include/spread_spectrum.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════
 * 802.11a/g OFDM PHY
 * ═══════════════════════════════════════════════════════════════════ */

void wifi_short_training(Cplx *sts, int *len)
{
    /* 802.11a STS: 12 non-zero subcarriers in 64-pt FFT, repeated 10× */
    /* Simplified: generate one 64-sample period */
    static const double S_re[12] = {
        0, 0, 1, 0, 0, 0, -1, 0, 0, 0, 1, 0
    };
    Cplx freq[64];
    memset(freq, 0, sizeof(freq));
    /* Place on known subcarrier positions: ±4, ±8, ±12, ±16, ±20, ±24 */
    int positions[12] = {4, 8, 12, 16, 20, 24, 40, 44, 48, 52, 56, 60};
    double scale = sqrt(13.0 / 6.0);
    for (int i = 0; i < 12; i++)
        freq[positions[i]] = cplx(S_re[i] * scale, 0);

    ifft(freq, 64);
    /* 10 repetitions of 16-sample short symbol = 160 samples */
    *len = 160;
    for (int r = 0; r < 10; r++)
        for (int i = 0; i < 16; i++)
            sts[r * 16 + i] = freq[i % 64];
}

void wifi_long_training(Cplx *lts, int *len)
{
    /* 802.11a LTS: 52 non-zero subcarriers, 2 × 64 + 32 CP = 160 samples */
    Cplx freq[64];
    memset(freq, 0, sizeof(freq));
    /* All data + pilot subcarriers = ±1 BPSK (simplified) */
    for (int k = 1; k <= 26; k++) {
        freq[k] = cplx(1.0, 0.0);
        freq[64 - k] = cplx(1.0, 0.0);
    }
    freq[0] = cplx(0, 0);  /* DC null */
    ifft(freq, 64);

    *len = 160;
    /* Guard interval: last 32 of symbol */
    for (int i = 0; i < 32; i++)
        lts[i] = freq[32 + i];
    /* Two copies of 64-sample symbol */
    for (int r = 0; r < 2; r++)
        for (int i = 0; i < 64; i++)
            lts[32 + r * 64 + i] = freq[i];
}

void wifi_scramble(uint8_t init, uint8_t *data, int nbits)
{
    /* 802.11 scrambler: x^7 + x^4 + 1 */
    uint8_t lfsr = init & 0x7F;
    for (int i = 0; i < nbits; i++) {
        uint8_t fb = ((lfsr >> 6) ^ (lfsr >> 3)) & 1;
        data[i] ^= fb;
        lfsr = ((lfsr << 1) | fb) & 0x7F;
    }
}

int wifi_build_ppdu(const uint8_t *payload, int n_bytes, WifiRate rate,
                    Cplx *out)
{
    (void)rate; /* TODO: use rate to select modulation/coding */
    int pos = 0;

    /* 1. Short training sequence (160 samples) */
    Cplx sts[160];
    int sts_len;
    wifi_short_training(sts, &sts_len);
    memcpy(out + pos, sts, sts_len * sizeof(Cplx));
    pos += sts_len;

    /* 2. Long training sequence (160 samples) */
    Cplx lts[160];
    int lts_len;
    wifi_long_training(lts, &lts_len);
    memcpy(out + pos, lts, lts_len * sizeof(Cplx));
    pos += lts_len;

    /* 3. SIGNAL + DATA using OFDM (simplified) */
    OfdmParams ofdm;
    ofdm_init(&ofdm, 64, 16, 4);

    /* Convert payload to bits */
    int nbits = n_bytes * 8;
    uint8_t *bits = (uint8_t *)calloc(nbits + 128, sizeof(uint8_t));
    bits_from_bytes(payload, n_bytes, bits);

    /* Map to BPSK symbols (rate 6 Mbps baseline) */
    int nsyms = (nbits + ofdm.n_data - 1) / ofdm.n_data;
    Cplx *data_syms = (Cplx *)calloc(nsyms * ofdm.n_data, sizeof(Cplx));
    for (int i = 0; i < nsyms * ofdm.n_data; i++) {
        int bit = (i < nbits) ? bits[i] : 0;
        data_syms[i] = bit ? cplx(-1, 0) : cplx(1, 0);
    }

    /* OFDM modulate */
    int ofdm_samples = ofdm_modulate_block(&ofdm, nsyms, data_syms, out + pos);
    pos += ofdm_samples;

    free(bits);
    free(data_syms);
    return pos;
}

/* ═══════════════════════════════════════════════════════════════════
 * Bluetooth Baseband
 * ═══════════════════════════════════════════════════════════════════ */

void bt_gen_access_code(uint32_t lap, uint8_t *ac)
{
    /* Simplified: preamble (4 bits) + sync word (64 bits) + trailer (4 bits)
     * Sync word derived from LAP via BCH encoding (simplified here) */
    memset(ac, 0, BT_ACCESS_CODE_LEN);

    /* Preamble: alternating bits based on MSB of LAP */
    uint8_t msb = (lap >> 23) & 1;
    ac[0] = msb ? 0 : 1;
    ac[1] = msb ? 1 : 0;
    ac[2] = msb ? 0 : 1;
    ac[3] = msb ? 1 : 0;

    /* LAP into middle of sync word (bits 4–27) */
    for (int i = 0; i < 24; i++)
        ac[4 + i] = (lap >> (23 - i)) & 1;

    /* Fill remainder with Barker-like pattern for correlation */
    for (int i = 28; i < 68; i++)
        ac[i] = (i % 2 == 0) ? 1 : 0;

    /* Trailer */
    ac[68] = ac[67] ? 0 : 1;
    ac[69] = ac[67] ? 1 : 0;
    ac[70] = ac[67] ? 0 : 1;
    ac[71] = ac[67] ? 1 : 0;
}

void bt_whiten(uint8_t clock6, uint8_t *data, int nbits)
{
    /* Bluetooth data whitening: LFSR x^7 + x^4 + 1 */
    uint8_t lfsr = clock6 | 0x40; /* bit 6 always set */
    for (int i = 0; i < nbits; i++) {
        uint8_t fb = ((lfsr >> 6) ^ (lfsr >> 3)) & 1;
        data[i] ^= fb;
        lfsr = ((lfsr << 1) | fb) & 0x7F;
    }
}

int bt_build_packet(const BtPacketConfig *cfg,
                    const uint8_t *payload, int n_bytes,
                    int sps, Cplx *out)
{
    /* Build bit sequence: access code + header + payload */
    int header_bits = 18; /* simplified: 3-bit type + 15-bit header */
    int payload_bits = n_bytes * 8;
    int total_bits = BT_ACCESS_CODE_LEN + header_bits + payload_bits;

    uint8_t *bits = (uint8_t *)calloc(total_bits, sizeof(uint8_t));

    /* Access code */
    memcpy(bits, cfg->access_code, BT_ACCESS_CODE_LEN);

    /* Header (simplified) and payload */
    bits_from_bytes(payload, n_bytes, bits + BT_ACCESS_CODE_LEN + header_bits);

    /* Whiten */
    bt_whiten(0x3F, bits + BT_ACCESS_CODE_LEN, header_bits + payload_bits);

    /* GFSK modulate */
    double bt = (cfg->mode == BT_CLASSIC) ? 0.5 : 0.5;
    double h_mod = (cfg->mode == BT_CLASSIC) ? 0.32 : 0.5;
    int nsamples = gfsk_modulate(bits, total_bits, sps, bt, h_mod, out);

    free(bits);
    return nsamples;
}

/* ═══════════════════════════════════════════════════════════════════
 * Zigbee / 802.15.4 PHY
 * ═══════════════════════════════════════════════════════════════════ */

int zigbee_build_ppdu(const uint8_t *psdu, int n_bytes, int sps, Cplx *out)
{
    /* SHR: 4 × 0x00 preamble + 0xA7 SFD = 5 bytes = 40 bits → 10 symbols */
    uint8_t shr[5] = {0x00, 0x00, 0x00, 0x00, 0xA7};

    /* PHR: frame length */
    uint8_t phr = (uint8_t)n_bytes;

    /* Total bytes: SHR + PHR + PSDU */
    int total_bytes = 5 + 1 + n_bytes;
    uint8_t *frame = (uint8_t *)calloc(total_bytes, sizeof(uint8_t));
    memcpy(frame, shr, 5);
    frame[5] = phr;
    memcpy(frame + 6, psdu, n_bytes);

    /* Convert bytes to 4-bit symbols → chip sequences → O-QPSK */
    int n_symbols = total_bytes * 2; /* 2 nibbles per byte */
    int total_chips = n_symbols * 32;
    uint8_t *chip_bits = (uint8_t *)calloc(total_chips, sizeof(uint8_t));

    int chip_pos = 0;
    for (int b = 0; b < total_bytes; b++) {
        /* Low nibble first (802.15.4 convention) */
        uint8_t lo = frame[b] & 0x0F;
        uint8_t hi = (frame[b] >> 4) & 0x0F;

        int chips32[32];
        zigbee_chip_map(lo, chips32);
        for (int c = 0; c < 32; c++)
            chip_bits[chip_pos++] = (chips32[c] > 0) ? 1 : 0;

        zigbee_chip_map(hi, chips32);
        for (int c = 0; c < 32; c++)
            chip_bits[chip_pos++] = (chips32[c] > 0) ? 1 : 0;
    }

    /* O-QPSK modulate chips */
    int nsamples = oqpsk_modulate(chip_bits, total_chips, sps, out);

    free(frame);
    free(chip_bits);
    return nsamples;
}

/* ═══════════════════════════════════════════════════════════════════
 * LoRa PHY (CSS — Chirp Spread Spectrum)
 * ═══════════════════════════════════════════════════════════════════ */

void lora_init(LoraParams *lp, int sf, int bw, int cr)
{
    lp->sf = sf;
    lp->bw = bw;
    lp->cr = cr;
    lp->n_fft = 1 << sf;
    lp->fs = bw; /* baseband sample rate = BW */
}

void lora_modulate_symbol(const LoraParams *lp, int symbol, Cplx *out)
{
    int N = lp->n_fft;

    /* CSS upchirp: frequency ramp from -BW/2 to +BW/2 with cyclic shift
     * f(t) = ((symbol + t*BW/N) mod N) - N/2  normalised by N */
    for (int i = 0; i < N; i++) {
        (void)((double)(symbol + i) / N); /* f_inst for reference */
        /* Accumulate phase properly with chirp rate */
        double chirp_phase = M_PI * (double)i * i / (double)(N);
        double total_phase = 2.0 * M_PI * (double)symbol * i / N + chirp_phase;
        out[i] = cplx_exp_j(total_phase);
    }
}

int lora_preamble(const LoraParams *lp, int n_pre, Cplx *out)
{
    int N = lp->n_fft;
    int pos = 0;

    /* Upchirps (preamble symbols — all value 0) */
    for (int i = 0; i < n_pre; i++) {
        lora_modulate_symbol(lp, 0, out + pos);
        pos += N;
    }

    /* 2 downchirps (sync) — conjugate of upchirp */
    Cplx *upchirp = (Cplx *)malloc(N * sizeof(Cplx));
    lora_modulate_symbol(lp, 0, upchirp);
    for (int d = 0; d < 2; d++) {
        for (int i = 0; i < N; i++)
            out[pos + i] = cplx_conj(upchirp[i]);
        pos += N;
    }

    free(upchirp);
    return pos;
}

int lora_demodulate_symbol(const LoraParams *lp, const Cplx *in)
{
    int N = lp->n_fft;

    /* Dechirp: multiply by conjugate of base upchirp */
    Cplx *dechirped = (Cplx *)calloc(N, sizeof(Cplx));
    Cplx *base = (Cplx *)calloc(N, sizeof(Cplx));
    lora_modulate_symbol(lp, 0, base);

    for (int i = 0; i < N; i++)
        dechirped[i] = cplx_mul(in[i], cplx_conj(base[i]));

    /* FFT and find peak */
    fft(dechirped, N);

    double max_mag = 0;
    int max_idx = 0;
    for (int i = 0; i < N; i++) {
        double mag = cplx_mag2(dechirped[i]);
        if (mag > max_mag) {
            max_mag = mag;
            max_idx = i;
        }
    }

    free(dechirped);
    free(base);
    return max_idx;
}

int lora_build_frame(const LoraParams *lp, const uint8_t *payload,
                     int n_bytes, Cplx *out)
{
    int N = lp->n_fft;
    int pos = 0;

    /* Preamble (8 upchirps + 2 downchirps) */
    pos = lora_preamble(lp, 8, out);

    /* Encode payload bytes as symbols */
    int mask = N - 1;
    for (int i = 0; i < n_bytes; i++) {
        int sym = payload[i] & mask;
        lora_modulate_symbol(lp, sym, out + pos);
        pos += N;
    }

    return pos;
}

/* ═══════════════════════════════════════════════════════════════════
 * ADS-B / Mode S
 * ═══════════════════════════════════════════════════════════════════ */

void adsb_encode(const AdsbMessage *msg, uint8_t *bits112)
{
    memset(bits112, 0, 112);

    /* DF (5 bits) */
    for (int i = 0; i < 5; i++)
        bits112[i] = (msg->df >> (4 - i)) & 1;

    /* CA (3 bits) */
    for (int i = 0; i < 3; i++)
        bits112[5 + i] = (msg->ca >> (2 - i)) & 1;

    /* ICAO (24 bits) */
    for (int i = 0; i < 24; i++)
        bits112[8 + i] = (msg->icao >> (23 - i)) & 1;

    /* ME (56 bits) */
    for (int byte = 0; byte < 7; byte++) {
        for (int bit = 0; bit < 8; bit++)
            bits112[32 + byte * 8 + bit] = (msg->msg[byte] >> (7 - bit)) & 1;
    }

    /* CRC-24 over first 88 bits → bits 88–111 */
    uint8_t data_bytes[11];
    bytes_from_bits(bits112, 88, data_bytes);
    uint32_t crc = crc24_adsb(data_bytes, 11);
    for (int i = 0; i < 24; i++)
        bits112[88 + i] = (crc >> (23 - i)) & 1;
}

uint32_t adsb_crc24(const uint8_t *bits, int nbits)
{
    int nbytes = (nbits + 7) / 8;
    uint8_t *bytes = (uint8_t *)calloc(nbytes, sizeof(uint8_t));
    bytes_from_bits(bits, nbits, bytes);
    uint32_t crc = crc24_adsb(bytes, nbytes);
    free(bytes);
    return crc;
}

int adsb_modulate(const uint8_t *bits112, double *out)
{
    /* 2 MHz sample rate → 0.5 µs per sample
     * Preamble: 8 µs = 16 samples (specific pattern)
     * Data: 112 bits × 1 µs/bit = 112 µs = 224 samples
     * Total: 240 samples */
    int pos = 0;

    /* Preamble: pulses at 0, 1, 3.5, 4.5 µs (positions 0,2, 7,9 in samples) */
    memset(out, 0, 240 * sizeof(double));
    int preamble_high[4] = {0, 2, 7, 9};
    for (int i = 0; i < 4; i++) {
        out[preamble_high[i]] = 1.0;
        out[preamble_high[i] + 1] = 1.0;
    }
    pos = 16; /* 8 µs preamble */

    /* PPM: each bit = 1 µs (2 samples). Bit 1 = [1,0], Bit 0 = [0,1] */
    for (int i = 0; i < 112; i++) {
        if (bits112[i]) {
            out[pos]     = 1.0;
            out[pos + 1] = 0.0;
        } else {
            out[pos]     = 0.0;
            out[pos + 1] = 1.0;
        }
        pos += 2;
    }

    return pos; /* 240 */
}

int adsb_demodulate(const double *samples, int n_samples, AdsbMessage *msg)
{
    /* Detect preamble via correlation */
    double preamble_template[16] = {0};
    preamble_template[0] = 1; preamble_template[1] = 1;
    preamble_template[2] = 1; preamble_template[3] = 1;
    preamble_template[7] = 1; preamble_template[8] = 1;
    preamble_template[9] = 1; preamble_template[10] = 1;

    int start = -1;
    double max_corr = 0;

    for (int i = 0; i <= n_samples - 240; i++) {
        double corr = 0;
        for (int j = 0; j < 16; j++)
            corr += samples[i + j] * preamble_template[j];
        if (corr > max_corr) {
            max_corr = corr;
            start = i;
        }
    }

    if (start < 0) return -1;

    /* Decode bits via PPM: compare first and second half of each bit period */
    uint8_t bits[112];
    int data_start = start + 16;
    for (int i = 0; i < 112; i++) {
        double high = samples[data_start + 2 * i];
        double low  = samples[data_start + 2 * i + 1];
        bits[i] = (high > low) ? 1 : 0;
    }

    /* Extract fields */
    msg->df = 0;
    for (int i = 0; i < 5; i++) msg->df = (msg->df << 1) | bits[i];

    msg->ca = 0;
    for (int i = 0; i < 3; i++) msg->ca = (msg->ca << 1) | bits[5 + i];

    msg->icao = 0;
    for (int i = 0; i < 24; i++) msg->icao = (msg->icao << 1) | bits[8 + i];

    for (int byte = 0; byte < 7; byte++) {
        msg->msg[byte] = 0;
        for (int bit = 0; bit < 8; bit++)
            msg->msg[byte] = (msg->msg[byte] << 1) | bits[32 + byte * 8 + bit];
    }

    msg->crc = 0;
    for (int i = 0; i < 24; i++) msg->crc = (msg->crc << 1) | bits[88 + i];

    /* Verify CRC */
    uint8_t data_bytes[11];
    bytes_from_bits(bits, 88, data_bytes);
    uint32_t computed_crc = crc24_adsb(data_bytes, 11);
    return (computed_crc == msg->crc) ? 0 : -1;
}

/* ═══════════════════════════════════════════════════════════════════
 * MIMO helpers
 * ═══════════════════════════════════════════════════════════════════ */

void mimo_alamouti_encode(const Cplx *s, Cplx *tx0, Cplx *tx1)
{
    /* Time slot 0: TX0 sends s[0], TX1 sends s[1]
     * Time slot 1: TX0 sends -s[1]*, TX1 sends s[0]* */
    tx0[0] = s[0];
    tx0[1] = cplx_scale(cplx_conj(s[1]), -1.0);
    tx1[0] = s[1];
    tx1[1] = cplx_conj(s[0]);
}

void mimo_alamouti_decode(const Cplx *rx, Cplx h0, Cplx h1, Cplx *s_hat)
{
    /* Alamouti combining:
     * s0_hat = h0* · r0 + h1 · r1*
     * s1_hat = h1* · r0 - h0 · r1* */
    double norm2 = cplx_mag2(h0) + cplx_mag2(h1);
    if (norm2 < 1e-12) norm2 = 1e-12;

    Cplx r0 = rx[0], r1_conj = cplx_conj(rx[1]);
    s_hat[0] = cplx_scale(cplx_add(cplx_mul(cplx_conj(h0), r0),
                                    cplx_mul(h1, r1_conj)),
                           1.0 / norm2);
    s_hat[1] = cplx_scale(cplx_sub(cplx_mul(cplx_conj(h1), r0),
                                    cplx_mul(h0, r1_conj)),
                           1.0 / norm2);
}

Cplx mimo_mrc(const Cplx *rx, const Cplx *h, int n_rx)
{
    /* MRC: y = Σ h_i* · r_i / Σ |h_i|^2 */
    Cplx num = cplx(0, 0);
    double den = 0;
    for (int i = 0; i < n_rx; i++) {
        num = cplx_add(num, cplx_mul(cplx_conj(h[i]), rx[i]));
        den += cplx_mag2(h[i]);
    }
    if (den < 1e-12) den = 1e-12;
    return cplx_scale(num, 1.0 / den);
}

void mimo_zf_detect(const Cplx *rx, const Cplx *H,
                    int n_rx, int n_tx, Cplx *s_hat)
{
    /* ZF: s = (H^H·H)^{-1} · H^H · r
     * Simplified for 2×2 case; general case uses pseudoinverse */
    if (n_tx == 2 && n_rx >= 2) {
        /* 2×2: H^H·H is 2×2, invert analytically */
        Cplx HH[4]; /* H^H (2×n_rx) × H (n_rx×2) = 2×2 */
        memset(HH, 0, sizeof(HH));
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < n_rx; k++) {
                    HH[i * 2 + j] = cplx_add(HH[i * 2 + j],
                        cplx_mul(cplx_conj(H[k * n_tx + i]),
                                 H[k * n_tx + j]));
                }
            }
        }

        /* Invert 2×2: inv = [d,-b;-c,a] / det */
        Cplx det = cplx_sub(cplx_mul(HH[0], HH[3]),
                             cplx_mul(HH[1], HH[2]));
        double det2 = cplx_mag2(det);
        if (det2 < 1e-12) det2 = 1e-12;
        Cplx inv_det = cplx_scale(cplx_conj(det), 1.0 / det2);

        Cplx HHi[4];
        HHi[0] = cplx_mul(HH[3], inv_det);
        HHi[1] = cplx_mul(cplx_scale(HH[1], -1), inv_det);
        HHi[2] = cplx_mul(cplx_scale(HH[2], -1), inv_det);
        HHi[3] = cplx_mul(HH[0], inv_det);

        /* H^H · r */
        Cplx HHr[2] = { cplx(0,0), cplx(0,0) };
        for (int i = 0; i < 2; i++) {
            for (int k = 0; k < n_rx; k++)
                HHr[i] = cplx_add(HHr[i],
                    cplx_mul(cplx_conj(H[k * n_tx + i]), rx[k]));
        }

        /* (H^H·H)^{-1} · H^H · r */
        for (int i = 0; i < 2; i++) {
            s_hat[i] = cplx(0, 0);
            for (int j = 0; j < 2; j++)
                s_hat[i] = cplx_add(s_hat[i],
                    cplx_mul(HHi[i * 2 + j], HHr[j]));
        }
    } else {
        /* Fallback: just use MRC per stream */
        for (int i = 0; i < n_tx; i++) {
            Cplx num = cplx(0, 0);
            for (int k = 0; k < n_rx; k++)
                num = cplx_add(num, cplx_mul(cplx_conj(H[k * n_tx + i]), rx[k]));
            s_hat[i] = num;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Link budget helpers
 * ═══════════════════════════════════════════════════════════════════ */

double link_fspl_db(double distance_m, double freq_hz)
{
    /* FSPL = 20*log10(d) + 20*log10(f) + 20*log10(4π/c) */
    double c = 299792458.0;
    return 20.0 * log10(distance_m) + 20.0 * log10(freq_hz) +
           20.0 * log10(4.0 * M_PI / c);
}

double link_friis_dbm(double pt_dbm, double gt_dbi, double gr_dbi,
                      double distance_m, double freq_hz)
{
    return pt_dbm + gt_dbi + gr_dbi - link_fspl_db(distance_m, freq_hz);
}

double link_noise_floor_dbm(double bandwidth_hz, double noise_figure_db)
{
    /* N = kTB in dBm + noise figure */
    double kT_dbm = -174.0; /* dBm/Hz at 290K */
    return kT_dbm + 10.0 * log10(bandwidth_hz) + noise_figure_db;
}

double link_required_ebn0(double target_ber)
{
    /* Approximate inversion of BPSK BER = Q(sqrt(2*Eb/N0))
     * Using Newton's method approximation */
    if (target_ber >= 0.5) return 0.0;
    if (target_ber < 1e-10) return 15.0;

    /* Rough approximation using erfcinv */
    double x = 2.0 * target_ber;
    double t = sqrt(-2.0 * log(x));
    double ebn0_db = 10.0 * log10(t * t / 2.0);
    return ebn0_db;
}
