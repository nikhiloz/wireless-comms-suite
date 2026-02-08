/**
 * @file phy.h
 * @brief Protocol PHY helpers — Wi-Fi, Bluetooth, Zigbee, LoRa, ADS-B.
 *
 * High-level structures and frame builders for real protocol layers.
 * Each protocol chapter (16–20) uses these plus the lower-level modules.
 */

#ifndef PHY_H
#define PHY_H

#include "comms_utils.h"
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════════
 * 802.11a/g OFDM PHY
 * ═══════════════════════════════════════════════════════════════════ */

typedef enum {
    WIFI_RATE_6   = 0xD,   /* BPSK  1/2 */
    WIFI_RATE_9   = 0xF,   /* BPSK  3/4 */
    WIFI_RATE_12  = 0x5,   /* QPSK  1/2 */
    WIFI_RATE_18  = 0x7,   /* QPSK  3/4 */
    WIFI_RATE_24  = 0x9,   /* 16QAM 1/2 */
    WIFI_RATE_36  = 0xB,   /* 16QAM 3/4 */
    WIFI_RATE_48  = 0x1,   /* 64QAM 2/3 */
    WIFI_RATE_54  = 0x3    /* 64QAM 3/4 */
} WifiRate;

typedef struct {
    WifiRate rate;
    int      length;       /* PSDU length in bytes                   */
} WifiSignalField;

/**
 * @brief Build 802.11a PPDU: short preamble + long preamble + SIGNAL + DATA.
 * @param payload   PSDU bytes
 * @param n_bytes   PSDU length
 * @param rate      Data rate enum
 * @param out       Output baseband I/Q (caller allocates)
 * @return Number of samples produced
 */
int wifi_build_ppdu(const uint8_t *payload, int n_bytes, WifiRate rate,
                    Cplx *out);

/** 802.11a short training symbol (STS) generator. */
void wifi_short_training(Cplx *sts, int *len);

/** 802.11a long training symbol (LTS) generator. */
void wifi_long_training(Cplx *lts, int *len);

/** 802.11a scrambler (polynomial x^7 + x^4 + 1). */
void wifi_scramble(uint8_t init, uint8_t *data, int nbits);

/* ═══════════════════════════════════════════════════════════════════
 * Bluetooth Baseband (Classic + BLE)
 * ═══════════════════════════════════════════════════════════════════ */

typedef enum {
    BT_CLASSIC,    /* GFSK, BT=0.5, h=0.32, 1 Msym/s */
    BT_LE_1M,      /* GFSK, BT=0.5, h=0.5,  1 Msym/s */
    BT_LE_2M       /* GFSK, BT=0.5, h=0.5,  2 Msym/s */
} BtMode;

#define BT_ACCESS_CODE_LEN 72  /* bits: preamble(4) + sync(64) + trailer(4) */
#define BT_PREAMBLE_LEN     4

typedef struct {
    BtMode   mode;
    uint32_t lap;          /* Lower Address Part (for access code) */
    uint8_t  access_code[BT_ACCESS_CODE_LEN];
} BtPacketConfig;

/**
 * @brief Generate Bluetooth access code from LAP.
 * @param lap       24-bit Lower Address Part
 * @param ac        Output access code bits (72 bits)
 */
void bt_gen_access_code(uint32_t lap, uint8_t *ac);

/**
 * @brief Bluetooth data whitening (LFSR: x^7 + x^4 + 1).
 * @param clock6  Lower 6 bits of master clock
 */
void bt_whiten(uint8_t clock6, uint8_t *data, int nbits);

/**
 * @brief Build complete Bluetooth baseband packet → I/Q.
 * @return Number of I/Q samples
 */
int bt_build_packet(const BtPacketConfig *cfg,
                    const uint8_t *payload, int n_bytes,
                    int sps, Cplx *out);

/* ═══════════════════════════════════════════════════════════════════
 * Zigbee / 802.15.4 PHY (2.4 GHz O-QPSK)
 * ═══════════════════════════════════════════════════════════════════ */

#define ZIGBEE_SHR_LEN     40  /* preamble(32) + SFD(8) bits */
#define ZIGBEE_CHIPS_PER_SYM 32

typedef struct {
    uint8_t  frame_len;       /* PHR: frame length field             */
} ZigbeePhyConfig;

/**
 * @brief Build 802.15.4 PPDU: SHR + PHR + PSDU → chip stream → I/Q.
 * @return Number of I/Q samples
 */
int zigbee_build_ppdu(const uint8_t *psdu, int n_bytes, int sps, Cplx *out);

/* ═══════════════════════════════════════════════════════════════════
 * LoRa PHY (CSS — Chirp Spread Spectrum)
 * ═══════════════════════════════════════════════════════════════════ */

typedef struct {
    int  sf;              /* Spreading factor (7–12)               */
    int  bw;              /* Bandwidth in Hz (125000, 250000, 500000) */
    int  cr;              /* Coding rate (1=4/5, 2=4/6, 3=4/7, 4=4/8) */
    int  n_fft;           /* 2^SF                                  */
    int  fs;              /* Sample rate (= BW for baseband)       */
} LoraParams;

/**
 * @brief Initialise LoRa parameters.
 * @param sf  Spreading factor
 * @param bw  Bandwidth
 * @param cr  Coding rate
 */
void lora_init(LoraParams *lp, int sf, int bw, int cr);

/**
 * @brief Generate one LoRa upchirp symbol.
 * @param lp      LoRa params
 * @param symbol  Symbol value [0, 2^SF - 1]
 * @param out     Output samples (2^SF samples)
 */
void lora_modulate_symbol(const LoraParams *lp, int symbol, Cplx *out);

/**
 * @brief Generate LoRa preamble (n_pre upchirps + 2 downchirps).
 */
int lora_preamble(const LoraParams *lp, int n_pre, Cplx *out);

/**
 * @brief Demodulate one symbol (dechirp + FFT peak).
 * @return Detected symbol value [0, 2^SF - 1]
 */
int lora_demodulate_symbol(const LoraParams *lp, const Cplx *in);

/**
 * @brief Modulate payload → full LoRa frame (preamble + symbols).
 * @return Total samples produced
 */
int lora_build_frame(const LoraParams *lp, const uint8_t *payload,
                     int n_bytes, Cplx *out);

/* ═══════════════════════════════════════════════════════════════════
 * ADS-B / Mode S (1090 MHz Extended Squitter)
 * ═══════════════════════════════════════════════════════════════════ */

#define ADSB_MSG_BITS    112   /* DF17 long message */
#define ADSB_PREAMBLE_US  8.0  /* 8 µs preamble     */

typedef struct {
    uint8_t  df;            /* Downlink format (17 for ADS-B)       */
    uint8_t  ca;            /* Capability                           */
    uint32_t icao;          /* 24-bit ICAO address                  */
    uint8_t  msg[7];        /* 56-bit ME (message, extended)        */
    uint32_t crc;           /* 24-bit CRC (PI field)                */
} AdsbMessage;

/**
 * @brief Encode ADS-B message → 112-bit stream.
 */
void adsb_encode(const AdsbMessage *msg, uint8_t *bits112);

/**
 * @brief Generate ADS-B baseband waveform (PPM at 2 MHz sample rate).
 * @param bits112  112-bit encoded message
 * @param out      Output samples (preamble + data = 240 samples @2MHz)
 * @return Number of samples
 */
int adsb_modulate(const uint8_t *bits112, double *out);

/**
 * @brief Detect preamble and decode in received signal.
 * @return 0 on success, -1 on CRC failure
 */
int adsb_demodulate(const double *samples, int n_samples,
                    AdsbMessage *msg);

/**
 * @brief Compute CRC-24 for ADS-B/Mode-S.
 */
uint32_t adsb_crc24(const uint8_t *bits, int nbits);

/* ═══════════════════════════════════════════════════════════════════
 * MIMO helpers
 * ═══════════════════════════════════════════════════════════════════ */

/**
 * @brief Alamouti 2×1 STBC encode: 2 symbols → 2 time slots.
 * @param s       Input symbols (2)
 * @param tx0     Output for antenna 0 (2 time slots)
 * @param tx1     Output for antenna 1 (2 time slots)
 */
void mimo_alamouti_encode(const Cplx *s, Cplx *tx0, Cplx *tx1);

/**
 * @brief Alamouti decode with known channel.
 * @param rx      Received signal (2 time slots)
 * @param h0, h1  Channel coefficients (antenna 0, 1)
 * @param s_hat   Decoded symbols (2)
 */
void mimo_alamouti_decode(const Cplx *rx, Cplx h0, Cplx h1, Cplx *s_hat);

/**
 * @brief Maximum Ratio Combining (MRC) for N receive antennas.
 * @param rx   Received signals [n_rx]
 * @param h    Channel coefficients [n_rx]
 * @param n_rx Number of receive antennas
 * @return Combined symbol
 */
Cplx mimo_mrc(const Cplx *rx, const Cplx *h, int n_rx);

/**
 * @brief ZF MIMO detection for N_tx × N_rx.
 * @param rx      Received vector [n_rx]
 * @param H       Channel matrix [n_rx × n_tx], row-major
 * @param n_rx    Number of RX antennas
 * @param n_tx    Number of TX antennas
 * @param s_hat   Detected symbols [n_tx]
 */
void mimo_zf_detect(const Cplx *rx, const Cplx *H,
                    int n_rx, int n_tx, Cplx *s_hat);

/* ── Link budget ─────────────────────────────────────────────────── */

/** Free-space path loss in dB: FSPL = 20*log10(d) + 20*log10(f) + 20*log10(4π/c). */
double link_fspl_db(double distance_m, double freq_hz);

/** Friis equation: Pr(dBm) = Pt(dBm) + Gt(dBi) + Gr(dBi) - FSPL(dB). */
double link_friis_dbm(double pt_dbm, double gt_dbi, double gr_dbi,
                      double distance_m, double freq_hz);

/** Noise floor: kTB in dBm. */
double link_noise_floor_dbm(double bandwidth_hz, double noise_figure_db);

/** Required Eb/N0 margin for target BER (BPSK approximation). */
double link_required_ebn0(double target_ber);

#endif /* PHY_H */
