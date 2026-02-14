# Wireless Communications — Quick Reference Cheat Sheet

## 1. Core Formulas

### Signal-to-Noise Ratio
| Quantity | Formula | Notes |
|----------|---------|-------|
| SNR (linear) | $SNR = P_s / P_n$ | Signal power / noise power |
| SNR (dB) | $SNR_{dB} = 10 \log_{10}(P_s / P_n)$ | |
| Eb/N0 → SNR | $SNR = (E_b/N_0) \cdot (k/T_s)$ | k = bits/sym, Ts = sym period |
| SNR (dB) | $SNR_{dB} = E_b/N_0(dB) + 10\log_{10}(k \cdot R_c / SPS)$ | Rc = code rate, SPS = samples/sym |

### BER Theoretical (AWGN)
| Modulation | BER Formula | Eb/N0 for BER=10⁻⁵ |
|------------|-------------|---------------------|
| BPSK | $Q(\sqrt{2 E_b/N_0})$ | 9.6 dB |
| QPSK | $Q(\sqrt{2 E_b/N_0})$ | 9.6 dB (same as BPSK) |
| 8-PSK | $(2/3) Q(\sqrt{6 E_b/N_0} \sin(\pi/8))$ | ~13 dB |
| 16-QAM | $(3/8) Q(\sqrt{4 E_b/(5 N_0)})$ | ~13.4 dB |
| 64-QAM | $(7/24) Q(\sqrt{2 E_b/(7 N_0)})$ | ~17.8 dB |
| BFSK (coh.) | $Q(\sqrt{E_b/N_0})$ | 12.6 dB |
| GFSK (BT=0.5) | ≈ non-coherent FSK | ~14 dB |

### Q-Function
$$Q(x) = \frac{1}{2} \operatorname{erfc}\left(\frac{x}{\sqrt{2}}\right)$$

### Friis Path Loss
$$FSPL(dB) = 20\log_{10}(d) + 20\log_{10}(f) - 147.55$$
where d = metres, f = Hz.

### Link Budget
$$P_{rx} = P_{tx} + G_{tx} + G_{rx} - FSPL - L_{misc} + M_{fade}$$

### Shannon Capacity
$$C = B \log_2(1 + SNR)$$ bits/sec, B = bandwidth (Hz)

### Nyquist Symbol Rate
$$R_s = 2B / (1 + \alpha)$$ where α = roll-off factor

---

## 2. Modulation Comparison

| Scheme | Bits/Sym | Bandwidth Eff. | Min Eb/N0 @ 10⁻⁵ | Complexity | Used In |
|--------|----------|----------------|-------------------|------------|---------|
| BPSK | 1 | 1 b/s/Hz | 9.6 dB | Very Low | DSSS, DVB-S |
| QPSK | 2 | 2 b/s/Hz | 9.6 dB | Low | Wi-Fi, DVB-S2, LTE |
| 8-PSK | 3 | 3 b/s/Hz | ~13 dB | Medium | BT EDR (π/4-DQPSK, 8DPSK) |
| 16-QAM | 4 | 4 b/s/Hz | ~13.4 dB | Medium | Wi-Fi, LTE |
| 64-QAM | 6 | 6 b/s/Hz | ~17.8 dB | High | Wi-Fi, Cable |
| GFSK | 1 | 1 b/s/Hz | ~14 dB | Low | BT Classic, BLE |
| O-QPSK | 2 | 2 b/s/Hz | 9.6 dB | Low | Zigbee (802.15.4) |
| CSS | var. | SF-dependent | -20 dB (SF12) | Medium | LoRa |
| OOK | 1 | 1 b/s/Hz | 12.6 dB | Very Low | ISM 433 MHz |
| PPM | 1 | 1 b/s/Hz | ~10 dB | Low | ADS-B |
| OFDM | var. | subcarrier-dep. | varies | High | Wi-Fi, LTE, DVB-T |
| GMSK | 1 | 1.35 b/s/Hz | ~10 dB | Low | AIS, GSM |

---

## 3. Protocol PHY Parameters

| Parameter | Wi-Fi (11a/g) | BT Classic | BLE | Zigbee (2.4GHz) | LoRa | ADS-B |
|-----------|---------------|------------|-----|-----------------|------|-------|
| Frequency | 2.4 / 5 GHz | 2.4 GHz | 2.4 GHz | 2.4 GHz | Sub-GHz / 2.4 | 1090 MHz |
| Bandwidth | 20 MHz | 1 MHz | 2 MHz | 2 MHz (5 MHz chip) | 125–500 kHz | ~50 kHz |
| Modulation | OFDM (BPSK–64QAM) | GFSK (BT=0.5) | GFSK (BT=0.5) | O-QPSK + DSSS | CSS | PPM (OOK) |
| Symbol Rate | 250 kbaud × 52 subcarr | 1 Msym/s | 1 Msym/s | 62.5 ksym/s | BW/2^SF | 1 Mbps |
| Data Rate | 6–54 Mbps | 1 Mbps (BR) | 1 / 2 Mbps | 250 kbps | 0.3–50 kbps | ~1 Mbps |
| FEC | Conv 1/2, 2/3, 3/4 | None (BR) | None | None | Hamming 4/8 | None |
| CRC | CRC-32 | CRC-16 (HEC) | CRC-24 | CRC-16 | CRC-16 | CRC-24 |
| Packet Preamble | Short + Long training | Access Code (72 bit) | Access Addr (32 bit) | SHR (4+1 bytes) | 8–12 upchirps | 8 μs preamble |
| Range (typical) | 50–100 m | 10–100 m | 10–30 m | 10–100 m | 2–15 km | ~400 km (LOS) |
| Mod Index | N/A | h = 0.32 | h = 0.5 | N/A | N/A | N/A |

---

## 4. FEC Coding Summary

| Code | Type | Rate | Gain (dB) | Used In |
|------|------|------|-----------|---------|
| Hamming (7,4) | Block | 4/7 | ~1.5 | Educational |
| CRC-16-CCITT | Detection | — | N/A (detect ≤2 bit) | BT, Zigbee |
| CRC-32 | Detection | — | N/A (detect ≤3 bit) | Wi-Fi, Ethernet |
| Conv K=7, R=1/2 | Convolutional | 1/2 | ~5 @ 10⁻⁵ | Wi-Fi, cdma2000 |
| Conv K=7, R=3/4 | Punctured Conv | 3/4 | ~4 @ 10⁻⁵ | Wi-Fi |
| Turbo | Concatenated | ~1/3 | ~7 @ 10⁻⁵ | LTE, 3G |
| LDPC | Sparse parity | ~1/2–5/6 | ~8 @ 10⁻⁵ | Wi-Fi 6, 5G, DVB-S2 |
| Polar | Polarisation | ~1/2 | ~7 @ 10⁻⁵ | 5G NR control |
| Reed-Solomon | Block (GF) | (255,223) | ~6 | DVB, QR codes |
| BCH | Block | various | ~3–5 | POCSAG, flash memory |

---

## 5. Common dB Conversions

| Linear | dB |
|--------|-----|
| 1 | 0 dB |
| 2 | 3.01 dB |
| 4 | 6.02 dB |
| 10 | 10 dB |
| 100 | 20 dB |
| 1000 | 30 dB |
| 0.5 | -3.01 dB |
| 0.1 | -10 dB |
| 0.01 | -20 dB |

### Power ↔ dBm
$$P_{dBm} = 10 \log_{10}(P_{mW})$$

| Power | dBm |
|-------|-----|
| 1 mW | 0 dBm |
| 10 mW | 10 dBm |
| 100 mW | 20 dBm |
| 1 W | 30 dBm |
| 1 μW | -30 dBm |

### Voltage ↔ dBμV
$$V_{dB\mu V} = 20 \log_{10}(V_{\mu V})$$

---

## 6. Key Library Functions (wireless-comms-suite)

| Module | Key Functions |
|--------|---------------|
| `comms_utils.h` | `randn()`, `cplx_mul()`, `db_to_linear()`, `linear_to_db()` |
| `modulation.h` | `mod_modulate()`, `mod_demodulate()`, `gfsk_modulate()`, `raised_cosine()` |
| `coding.h` | `conv_encode()`, `viterbi_decode()`, `hamming74_encode()`, `crc16_ccitt()` |
| `channel.h` | `channel_awgn()`, `channel_rayleigh_flat()`, `ebn0_to_snr()` |
| `sync.h` | `frame_sync_detect()`, `scrambler()`, `timing_init()`, `carrier_init()` |
| `ofdm.h` | `fft()`, `ifft()`, `ofdm_modulate()`, `ofdm_demodulate()` |
| `spread_spectrum.h` | `pn_msequence()`, `pn_gold()`, `dsss_spread()`, `fhss_init()` |
| `equaliser.h` | `eq_zf_flat()`, `eq_lms_step()`, `eq_rls_step()`, `eq_dfe_step()` |
| `phy.h` | `wifi_build_ppdu()`, `bt_build_packet()`, `lora_mod()`, `adsb_encode()` |
| `analog_demod.h` | `fm_discriminator()`, `fm_deemphasis()`, `am_envelope()`, `ssb_weaver_mod()` |