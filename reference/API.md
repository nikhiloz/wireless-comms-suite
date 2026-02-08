# API Reference — wireless-comms-suite

Complete public API surface for all 9 modules. Functions are listed with
signatures and brief descriptions. All types and functions are declared in
the corresponding `include/*.h` header.

---

## Table of Contents

1. [comms_utils.h — Core Utilities](#1-comms_utilsh--core-utilities)
2. [modulation.h — Digital Modulation](#2-modulationh--digital-modulation)
3. [coding.h — Source & Channel Coding](#3-codingh--source--channel-coding)
4. [channel.h — Channel Models](#4-channelh--channel-models)
5. [sync.h — Synchronisation](#5-synch--synchronisation)
6. [ofdm.h — OFDM System](#6-ofdmh--ofdm-system)
7. [spread_spectrum.h — Spread Spectrum](#7-spread_spectrumh--spread-spectrum)
8. [equaliser.h — Channel Equalisation](#8-equaliserh--channel-equalisation)
9. [phy.h — Protocol PHY, MIMO & Link Budget](#9-phyh--protocol-phy-mimo--link-budget)

---

## 1. comms_utils.h — Core Utilities

### Types

```c
typedef struct { double re, im; } Cplx;
```

### Complex Arithmetic

| Function | Description |
|----------|-------------|
| `Cplx cplx(double re, double im)` | Construct complex number |
| `Cplx cplx_add(Cplx a, Cplx b)` | a + b |
| `Cplx cplx_sub(Cplx a, Cplx b)` | a − b |
| `Cplx cplx_mul(Cplx a, Cplx b)` | a × b |
| `Cplx cplx_conj(Cplx z)` | Complex conjugate |
| `Cplx cplx_scale(Cplx z, double s)` | Scalar multiply |
| `double cplx_mag(Cplx z)` | \|z\| |
| `double cplx_mag2(Cplx z)` | \|z\|² (no sqrt) |
| `double cplx_phase(Cplx z)` | arg(z) in radians |
| `Cplx cplx_from_polar(double mag, double phase)` | Polar → rectangular |
| `Cplx cplx_exp_j(double theta)` | e^{jθ} |

### PRNG

| Function | Description |
|----------|-------------|
| `void rng_seed(uint64_t seed)` | Seed the Xoshiro256** generator |
| `double rng_uniform(void)` | Uniform [0, 1) |
| `double rng_gaussian(void)` | N(0,1) via Box-Muller |
| `int rng_bernoulli(double p)` | 1 with probability p |

### Bit Helpers

| Function | Description |
|----------|-------------|
| `void bits_from_bytes(const uint8_t *bytes, int nbytes, uint8_t *bits)` | Unpack bytes → bits (MSB first) |
| `void bytes_from_bits(const uint8_t *bits, int nbits, uint8_t *bytes)` | Pack bits → bytes |
| `void random_bits(uint8_t *bits, int n)` | Fill with random 0/1 |
| `int bit_errors(const uint8_t *a, const uint8_t *b, int n)` | Count differing bits |
| `void print_bits(const uint8_t *bits, int n, const char *label)` | Print bit array |

### ASCII Plotting

| Function | Description |
|----------|-------------|
| `void print_signal_ascii(const char *title, const double *x, int n, int max)` | Time-domain signal plot |
| `void print_constellation_ascii(const Cplx *syms, int n, int grid)` | IQ constellation scatter |
| `void print_eye_diagram_ascii(const double *x, int n, int sps, int eyes)` | Eye diagram overlay |
| `void print_bar_chart(const char *title, const double *vals, const char **labels, int n)` | Horizontal bar chart |
| `void print_separator(const char *title)` | Section divider line |

### Mathematics

| Function | Description |
|----------|-------------|
| `double db_to_linear(double db)` | 10^(dB/10) |
| `double linear_to_db(double lin)` | 10·log₁₀(lin) |
| `double sinc(double x)` | sin(πx)/(πx), sinc(0)=1 |
| `double clamp(double x, double lo, double hi)` | Clamp to range |
| `int next_pow2(int n)` | Next power of 2 ≥ n |
| `double get_time_ms(void)` | Wall-clock milliseconds |

---

## 2. modulation.h — Digital Modulation

### Enums

```c
typedef enum {
    MOD_BPSK, MOD_QPSK, MOD_8PSK, MOD_16QAM, MOD_64QAM,
    MOD_GFSK, MOD_OQPSK
} ModScheme;
```

### Core Modulation

| Function | Description |
|----------|-------------|
| `int mod_bits_per_symbol(ModScheme scheme)` | Bits per symbol for scheme |
| `int mod_constellation(ModScheme scheme, Cplx *pts)` | Get constellation points (Gray-coded) |
| `int mod_modulate(ModScheme scheme, const uint8_t *bits, int nbits, Cplx *syms)` | Map bits → IQ symbols |
| `int mod_demodulate(ModScheme scheme, const Cplx *syms, int nsyms, uint8_t *bits)` | Hard decision demod |
| `int mod_demodulate_soft(ModScheme scheme, const Cplx *syms, int nsyms, double sigma2, double *llr)` | Soft LLR output |

### BER Theory

| Function | Description |
|----------|-------------|
| `double ber_bpsk_theory(double ebn0_db)` | Q(√(2·Eb/N0)) |
| `double ber_qpsk_theory(double ebn0_db)` | Same as BPSK (same Eb/N0) |
| `double ber_16qam_theory(double ebn0_db)` | Approximate 16-QAM BER |
| `double q_function(double x)` | Q(x) = 0.5·erfc(x/√2) |

### GFSK

| Function | Description |
|----------|-------------|
| `int gfsk_modulate(const uint8_t *bits, int nbits, int sps, double bt, double h, Cplx *out)` | Gaussian FSK modulation |
| `int gfsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits)` | FM discriminator demod |

### O-QPSK

| Function | Description |
|----------|-------------|
| `int oqpsk_modulate(const uint8_t *bits, int nbits, int sps, Cplx *out)` | Offset QPSK with half-sine |
| `int oqpsk_demodulate(const Cplx *in, int nsamples, int sps, uint8_t *bits)` | O-QPSK demodulator |

### Pulse Shaping

| Function | Description |
|----------|-------------|
| `int raised_cosine(double alpha, int sps, int span, double *h)` | RC filter coefficients |
| `int root_raised_cosine(double alpha, int sps, int span, double *h)` | RRC (matched pair) |
| `int pulse_shape(const double *syms, int nsyms, const double *h, int hlen, int sps, double *out)` | Upsample + convolve |

### Line Coding

| Function | Description |
|----------|-------------|
| `void nrz_encode(const uint8_t *bits, int n, double *out)` | 0→−1, 1→+1 |
| `void manchester_encode(const uint8_t *bits, int n, double *out)` | 2 samples/bit self-clocking |

---

## 3. coding.h — Source & Channel Coding

### Source Coding

| Function | Description |
|----------|-------------|
| `double entropy(const double *probs, int n)` | Shannon entropy (bits) |
| `int huffman_build(const double *probs, int n, HuffmanTable *ht)` | Build Huffman tree |
| `int huffman_encode(const HuffmanTable *ht, const uint8_t *syms, int n, uint8_t *bits, int max)` | Huffman encode symbols |
| `int huffman_decode(const HuffmanTable *ht, const uint8_t *bits, int nbits, uint8_t *syms, int max)` | Huffman decode bits |
| `int rle_encode(const uint8_t *data, int n, uint8_t *out, int max)` | Run-length encode |
| `int rle_decode(const uint8_t *data, int n, uint8_t *out, int max)` | Run-length decode |

### CRC

| Function | Description |
|----------|-------------|
| `uint16_t crc16_ccitt(const uint8_t *data, int n)` | CRC-16/CCITT (poly 0x1021) |
| `uint32_t crc32(const uint8_t *data, int n)` | CRC-32 (Ethernet) |
| `uint32_t crc24_adsb(const uint8_t *data, int n)` | CRC-24 (Mode S / ADS-B) |

### Parity & Hamming

| Function | Description |
|----------|-------------|
| `uint8_t parity_even(const uint8_t *bits, int n)` | Even parity bit |
| `void parity_encode(const uint8_t *in, int n, uint8_t *out)` | Append parity (n+1 out) |
| `int parity_check(const uint8_t *codeword, int n_plus_1)` | Check parity (0 = OK) |
| `void hamming74_encode(const uint8_t *data4, uint8_t *code7)` | Hamming(7,4) encode |
| `int hamming74_decode(const uint8_t *code7, uint8_t *data4)` | Decode + SECDED |

### Convolutional / Viterbi

| Function | Description |
|----------|-------------|
| `void conv_encode(const uint8_t *in, int n, uint8_t *out)` | Rate-1/2, K=7 (g₁=0171, g₂=0133) |
| `int viterbi_decode(const uint8_t *coded, int n_coded, uint8_t *decoded)` | Hard-decision Viterbi |
| `int viterbi_decode_soft(const double *llr, int n_coded, uint8_t *decoded)` | Soft-decision Viterbi |

### Interleaver

```c
typedef struct {
    int *perm;
    int size, rows, cols;
} Interleaver;
```

| Function | Description |
|----------|-------------|
| `int interleaver_init(Interleaver *itl, int rows, int cols)` | Block interleaver (write rows, read cols) |
| `void interleaver_free(Interleaver *itl)` | Free permutation table |
| `void interleaver_apply(const Interleaver *itl, const uint8_t *in, uint8_t *out)` | Interleave |
| `void interleaver_deapply(const Interleaver *itl, const uint8_t *in, uint8_t *out)` | De-interleave |

---

## 4. channel.h — Channel Models

### AWGN

| Function | Description |
|----------|-------------|
| `double channel_awgn(const Cplx *in, int n, double snr_db, Cplx *out)` | Add complex AWGN, returns noise σ |
| `double channel_awgn_real(const double *in, int n, double snr_db, double *out)` | Real-valued AWGN |
| `double ebn0_to_snr(double ebn0_db, int bps, double code_rate, double bw_ratio)` | Eb/N0 → SNR conversion |
| `double snr_to_ebn0(double snr_db, int bps, double code_rate, double bw_ratio)` | SNR → Eb/N0 conversion |

### Fading

```c
typedef struct { Cplx h; double power; } RayleighChannel;
typedef struct { Cplx h; double K; double power; } RicianChannel;
typedef struct { int n_taps; double delays[32]; double gains[32]; Cplx taps[32]; } MultipathChannel;
```

| Function | Description |
|----------|-------------|
| `void channel_rayleigh_flat(RayleighChannel *ch, const Cplx *in, int n, double snr_db, Cplx *out)` | Flat Rayleigh fading + AWGN |
| `void channel_rayleigh_gen(int n, Cplx *coeffs)` | Generate n Rayleigh coefficients |
| `void channel_rician_flat(RicianChannel *ch, const Cplx *in, int n, double snr_db, Cplx *out)` | Rician (K-factor) fading + AWGN |
| `void channel_multipath_init(MultipathChannel *ch, int n_taps, const double *delays, const double *gains)` | Initialise multipath profile |
| `void channel_multipath_apply(const MultipathChannel *ch, const Cplx *in, int n, double snr_db, Cplx *out)` | Apply multipath + AWGN |
| `void channel_doppler(const Cplx *in, int n, double fd, Cplx *out)` | Apply frequency shift |

### Power Measurement

| Function | Description |
|----------|-------------|
| `double signal_power(const Cplx *x, int n)` | Average complex signal power |
| `double signal_power_real(const double *x, int n)` | Average real signal power |
| `double compute_snr_db(const Cplx *signal, const Cplx *noisy, int n)` | Measured SNR in dB |

---

## 5. sync.h — Synchronisation

### Timing Recovery

```c
typedef struct {
    int sps;
    double mu, gain_p, gain_i, accum;
    Cplx prev, prev2;
} TimingRecovery;
```

| Function | Description |
|----------|-------------|
| `void timing_init(TimingRecovery *tr, int sps, double loop_bw, double damping)` | Initialise TED loop |
| `int timing_recover_gardner(TimingRecovery *tr, const Cplx *in, int n, Cplx *out)` | Gardner TED |
| `int timing_recover_mm(TimingRecovery *tr, const Cplx *in, int n, Cplx *out)` | Mueller-Muller TED |

### Carrier Synchronisation

```c
typedef struct {
    double phase, freq, gain_p, gain_i, accum;
    double loop_bw, damping;
} CarrierSync;
```

| Function | Description |
|----------|-------------|
| `void carrier_init(CarrierSync *cs, double loop_bw, double damping)` | Initialise carrier loop |
| `double carrier_costas_bpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out)` | BPSK Costas loop |
| `double carrier_costas_qpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out)` | QPSK Costas loop |
| `double carrier_pll(CarrierSync *cs, const Cplx *in, int n, PhaseDetFunc pdet, Cplx *out)` | Generic PLL |

### Frame Synchronisation

| Function | Description |
|----------|-------------|
| `int frame_sync_correlate(const double *signal, int sig_len, const int *preamble, int pre_len, double *corr)` | Cross-correlation |
| `int frame_sync_detect(const double *signal, int sig_len, const int *preamble, int pre_len, double threshold)` | Detect preamble (returns offset) |

### Scrambler

| Function | Description |
|----------|-------------|
| `void scrambler(uint16_t poly, uint16_t init, uint8_t *bits, int n)` | LFSR scrambler (self-inverse) |

### Constants

```c
extern const int BARKER_13[13];  /* {+1,+1,+1,+1,+1,−1,−1,+1,+1,−1,+1,−1,+1} */
extern const int BARKER_11[11];
extern const int BARKER_7[7];
```

---

## 6. ofdm.h — OFDM System

### FFT

| Function | Description |
|----------|-------------|
| `void fft(Cplx *x, int n)` | In-place radix-2 DIT FFT (n must be power of 2) |
| `void ifft(Cplx *x, int n)` | In-place IFFT with 1/N scaling |

### OFDM Parameters

```c
typedef struct {
    int n_fft, n_cp, n_data, n_pilot;
    int data_idx[256], pilot_idx[32];
} OfdmParams;
```

### OFDM Functions

| Function | Description |
|----------|-------------|
| `void ofdm_init(OfdmParams *p, int n_fft, int n_cp, int n_pilot)` | Set up subcarrier indices |
| `int ofdm_modulate(const OfdmParams *p, const Cplx *data, Cplx *out)` | Single OFDM symbol |
| `int ofdm_modulate_block(const OfdmParams *p, int n, const Cplx *data, Cplx *out)` | Multi-symbol block |
| `int ofdm_demodulate(const OfdmParams *p, const Cplx *in, Cplx *data, Cplx *pilots)` | Single symbol demod |
| `int ofdm_demodulate_block(const OfdmParams *p, int n, const Cplx *in, Cplx *data)` | Block demod |
| `void ofdm_channel_estimate(const OfdmParams *p, const Cplx *rx_freq, const Cplx *tx_pilots, Cplx *h_est)` | Pilot-based estimation |
| `void ofdm_equalise_zf(const Cplx *data, const Cplx *h, int n, Cplx *out)` | ZF frequency-domain EQ |

---

## 7. spread_spectrum.h — Spread Spectrum

### PN Sequences

| Function | Description |
|----------|-------------|
| `int pn_msequence(uint32_t poly, int n_bits, int *seq)` | Maximum-length LFSR (±1 output) |
| `int pn_gold(uint32_t poly1, uint32_t poly2, int n, int shift, int *seq)` | Gold code (XOR of 2 m-sequences) |
| `void pn_autocorr(const int *seq, int n, double *corr)` | Autocorrelation |

### DSSS

| Function | Description |
|----------|-------------|
| `void dsss_spread(const uint8_t *bits, int nbits, const int *code, int chip_len, double *chips)` | Spread bits with PN code |
| `int dsss_despread(const double *chips, int nchips, const int *code, int chip_len, uint8_t *bits)` | Correlate + threshold |
| `double dsss_processing_gain_db(int chip_len)` | 10·log₁₀(chip_len) |

### FHSS

```c
typedef struct {
    int n_channels, n_hops, seed;
    int *hop_seq;
} FhssParams;
```

| Function | Description |
|----------|-------------|
| `void fhss_init(FhssParams *fp, int n_channels, int n_hops, int seed)` | Generate hop sequence (LCG) |
| `int fhss_get_channel(const FhssParams *fp, int hop_idx)` | Get channel for hop index |

### 802.15.4

| Function | Description |
|----------|-------------|
| `void zigbee_chip_map(uint8_t symbol, int *chips32)` | 4-bit symbol → 32 PN chips |

---

## 8. equaliser.h — Channel Equalisation

### Frequency-Domain

| Function | Description |
|----------|-------------|
| `void eq_zf_freq(const Cplx *rx, const Cplx *h, int n, Cplx *out)` | ZF: out[k] = rx[k] / h[k] |
| `void eq_zf_flat(const Cplx *rx, Cplx h, int n, Cplx *out)` | ZF for flat channel |
| `void eq_mmse_freq(const Cplx *rx, const Cplx *h, int n, double noise_var, Cplx *out)` | MMSE: h*/(|h|² + σ²) |

### LMS Adaptive

```c
typedef struct {
    Cplx *w;          /* Tap weights */
    Cplx *buf;        /* Delay line */
    int n_taps, idx;
    double mu;        /* Step size */
} LmsEqualiser;
```

| Function | Description |
|----------|-------------|
| `int eq_lms_init(LmsEqualiser *eq, int n_taps, double mu)` | Allocate + centre tap init |
| `void eq_lms_free(LmsEqualiser *eq)` | Free memory |
| `Cplx eq_lms_step(LmsEqualiser *eq, Cplx input, Cplx desired, Cplx *error)` | Training mode step |
| `Cplx eq_lms_dd_step(LmsEqualiser *eq, Cplx input, Cplx *error)` | Decision-directed step |

### RLS Adaptive

```c
typedef struct {
    Cplx *w, *buf, *P; /* P = n_taps × n_taps inverse correlation */
    int n_taps, idx;
    double lambda;
} RlsEqualiser;
```

| Function | Description |
|----------|-------------|
| `int eq_rls_init(RlsEqualiser *eq, int n_taps, double lambda, double delta)` | Allocate + P = δ·I |
| `void eq_rls_free(RlsEqualiser *eq)` | Free memory |
| `Cplx eq_rls_step(RlsEqualiser *eq, Cplx input, Cplx desired, Cplx *error)` | RLS update |

### Decision Feedback

```c
typedef struct {
    Cplx *wf, *wb, *fbuf, *bbuf;
    int ff_taps, fb_taps, fidx, bidx;
    double mu;
} DfeEqualiser;
```

| Function | Description |
|----------|-------------|
| `int eq_dfe_init(DfeEqualiser *eq, int ff_taps, int fb_taps, double mu)` | Allocate FF + FB filters |
| `void eq_dfe_free(DfeEqualiser *eq)` | Free memory |
| `Cplx eq_dfe_step(DfeEqualiser *eq, Cplx input, Cplx desired, Cplx *error)` | DFE step |

---

## 9. phy.h — Protocol PHY, MIMO & Link Budget

### Wi-Fi (802.11a/g OFDM)

```c
typedef enum {
    WIFI_RATE_6M, WIFI_RATE_9M, WIFI_RATE_12M, WIFI_RATE_18M,
    WIFI_RATE_24M, WIFI_RATE_36M, WIFI_RATE_48M, WIFI_RATE_54M
} WifiRate;

typedef struct { WifiRate rate; int length; } WifiSignalField;
```

| Function | Description |
|----------|-------------|
| `int wifi_build_ppdu(const uint8_t *payload, int n_bytes, WifiRate rate, Cplx *out, int max_out)` | Full PPDU: preamble + SIGNAL + DATA |
| `void wifi_short_training(Cplx *sts, int *len)` | IEEE STS (10 × 16-sample repeats = 160) |
| `void wifi_long_training(Cplx *lts, int *len)` | LTS (GI₂ + 2 × 64 = 160) |
| `void wifi_scramble(uint8_t init, uint8_t *data, int nbits)` | 802.11 x⁷+x⁴+1 scrambler |

### Bluetooth

```c
typedef enum { BT_MODE_BR, BT_MODE_EDR2, BT_MODE_EDR3, BT_MODE_BLE } BtMode;
typedef struct { BtMode mode; uint32_t lap; uint8_t clock6; int sps; double bt; double h; } BtPacketConfig;
#define BT_ACCESS_CODE_LEN 72
```

| Function | Description |
|----------|-------------|
| `void bt_gen_access_code(uint32_t lap, uint8_t *ac)` | 72-bit access code from LAP |
| `void bt_whiten(uint8_t clock6, uint8_t *data, int nbits)` | Data whitening (LFSR) |
| `int bt_build_packet(const BtPacketConfig *cfg, const uint8_t *payload, int n_bytes, Cplx *out, int max_out)` | Full BT packet (AC + header + payload → GFSK) |

### Zigbee (802.15.4)

```c
typedef struct { int channel; int sps; } ZigbeePhyConfig;
```

| Function | Description |
|----------|-------------|
| `int zigbee_build_ppdu(const uint8_t *psdu, int n_bytes, int sps, Cplx *out)` | SHR + chip spread + O-QPSK |

### LoRa (CSS)

```c
typedef struct { int sf, bw, cr, n_fft; double fs; } LoraParams;
```

| Function | Description |
|----------|-------------|
| `void lora_init(LoraParams *lp, int sf, int bw, int cr)` | Set SF/BW/CR, compute n_fft = 2^SF |
| `void lora_modulate_symbol(const LoraParams *lp, int symbol, Cplx *out)` | Generate one CSS chirp |
| `int lora_preamble(const LoraParams *lp, int n_pre, Cplx *out)` | n_pre + 2.25 sync upchirps |
| `int lora_demodulate_symbol(const LoraParams *lp, const Cplx *in)` | FFT-based dechirp → peak detect |
| `int lora_build_frame(const LoraParams *lp, const uint8_t *payload, int n_bytes, int n_pre, Cplx *out)` | Full LoRa frame |

### ADS-B (Mode S)

```c
typedef struct { uint8_t df; uint32_t icao; uint8_t data[7]; } AdsbMessage;
```

| Function | Description |
|----------|-------------|
| `void adsb_encode(const AdsbMessage *msg, uint8_t *bits112)` | Pack DF + ICAO + data + CRC-24 |
| `int adsb_modulate(const uint8_t *bits112, double *out)` | PPM @ 2 MHz (8µs preamble + 112 bits) |
| `int adsb_demodulate(const double *samples, int n, uint8_t *bits112, int *msg_start)` | Preamble search + PPM decode |
| `uint32_t adsb_crc24(const uint8_t *bits, int nbits)` | CRC-24 (generator 0xFFF409) |

### MIMO

| Function | Description |
|----------|-------------|
| `void mimo_alamouti_encode(const Cplx *s, Cplx *tx0, Cplx *tx1)` | 2×2 Alamouti STBC encode (2 symbols → 2 time slots) |
| `void mimo_alamouti_decode(const Cplx *rx, Cplx h0, Cplx h1, Cplx *s_hat)` | ML decode with known CSI |
| `Cplx mimo_mrc(const Cplx *rx, const Cplx *h, int n_rx)` | Maximum ratio combining (1×N_r) |
| `void mimo_zf_detect(const Cplx *rx, const Cplx *H, int n_tx, int n_rx, Cplx *x_hat)` | ZF spatial multiplexing (2×2) |

### Link Budget

| Function | Description |
|----------|-------------|
| `double link_fspl_db(double distance_m, double freq_hz)` | Free space path loss in dB |
| `double link_friis_dbm(double pt_dbm, double gt_dbi, double gr_dbi, double dist_m, double freq_hz)` | Received power via Friis |
| `double link_noise_floor_dbm(double bandwidth_hz, double noise_figure_db)` | kTB + NF |
| `double link_required_ebn0(double target_ber)` | Inverse Q-function for BPSK |
