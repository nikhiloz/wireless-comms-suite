# Wireless Communication Systems — C Tutorial Suite

**A progressive C99 tutorial covering the complete digital communication chain — from source coding through real wireless protocol PHY implementations.**

> **Status**: Outline captured. Implementation next.
>
> **TODO**: Scaffold full repo structure (chapters/, include/, src/, tests/, Makefile, CMakeLists.txt) and begin chapter-by-chapter implementation following the [dsp-tutorial-suite](https://github.com/nikhiloz/dsp-tutorial-suite) conventions.

---

## Overview

This repository takes you from digital comms fundamentals through a full TX → channel → RX simulation, then applies everything to real-world protocol PHY layers (Wi-Fi, Bluetooth, Zigbee, LoRa, ADS-B). Pure C99, zero external dependencies, paired `.md` + `.c` per chapter.

### Companion Repositories

| Repository | Focus |
|-----------|-------|
| [dsp-tutorial-suite](https://github.com/nikhiloz/dsp-tutorial-suite) | Pure DSP — FFT, filters, spectral analysis (30 chapters) |
| [SDR_Notes](https://github.com/nikhiloz/SDR_Notes) | SDR documentation — hardware, software, protocols, projects |
| **This repo** | Wireless comms — TX/RX chain, coding, sync, real protocol PHY |

---

## Chapter Outline (24 Chapters)

### Part I — Foundations

| Ch | Topic | Key Code |
|----|-------|----------|
| 01 | Digital comms system overview (TX/RX block diagram) | `system_model.c` |
| 02 | Source coding (Huffman, run-length, entropy) | `source_coding.c` |
| 03 | Channel coding basics (parity, Hamming, CRC-16/32) | `channel_coding.c` |
| 04 | Pulse shaping & line coding (NRZ, Manchester, raised cosine, eye diagram) | `pulse_shaping.c` |
| 05 | Digital modulation (BPSK, QPSK, 16-QAM — constellation + BER) | `modulation.c` |

### Part II — Channel & Synchronisation

| Ch | Topic | Key Code |
|----|-------|----------|
| 06 | AWGN channel simulation (Gaussian noise gen, SNR control) | `awgn_channel.c` |
| 07 | Fading channels (Rayleigh, Rician, multipath, Doppler) | `fading_channel.c` |
| 08 | Symbol timing recovery (Gardner, Mueller-Muller) | `timing_recovery.c` |
| 09 | Carrier synchronisation (Costas loop, PLL, freq offset) | `carrier_sync.c` |
| 10 | Frame synchronisation (preamble correlation, Barker codes) | `frame_sync.c` |

### Part III — Advanced Techniques

| Ch | Topic | Key Code |
|----|-------|----------|
| 11 | Convolutional codes + Viterbi decoder | `viterbi.c` |
| 12 | Interleaving & burst error protection | `interleaver.c` |
| 13 | Channel equalisation (ZF, MMSE, adaptive) | `equaliser.c` |
| 14 | OFDM system (IFFT/FFT TX/RX, cyclic prefix, pilot tones) | `ofdm.c` |
| 15 | Spread spectrum (DSSS, FHSS, PN sequences, processing gain) | `spread_spectrum.c` |

### Part IV — Real Protocol PHY Implementations

| Ch | Topic | Key Code |
|----|-------|----------|
| 16 | 802.11a/g OFDM PHY (scrambler → conv coder → interleaver → OFDM) | `wifi_phy.c` |
| 17 | Bluetooth baseband (GFSK, whitening, access code, packet format) | `bt_baseband.c` |
| 18 | Zigbee / 802.15.4 (O-QPSK, DSSS chip spreading) | `zigbee_phy.c` |
| 19 | LoRa PHY (CSS chirps, spreading factors, demodulation) | `lora_phy.c` |
| 20 | ADS-B / Mode S (PPM, preamble detect, CRC-24) | `adsb_phy.c` |

### Part V — System Design

| Ch | Topic | Key Code |
|----|-------|----------|
| 21 | Link budget (Friis, noise figure, fade margin) | `link_budget.c` |
| 22 | BER/PER Monte Carlo simulation framework | `ber_simulation.c` |
| 23 | MIMO & spatial diversity (Alamouti, MRC, ZF detection) | `mimo.c` |
| 24 | Full transceiver capstone (TX → channel → RX pipeline) | `transceiver.c` |

---

## Planned Folder Structure

```
wireless-comms-suite/
├── README.md
├── Makefile
├── CMakeLists.txt
├── chapters/
│   ├── 01-system-overview.md
│   ├── 01-system-overview.c
│   ├── 02-source-coding.md
│   ├── 02-source-coding.c
│   ├── ...
│   └── 24-transceiver.c
├── include/
│   ├── modulation.h
│   ├── channel.h
│   ├── coding.h
│   ├── sync.h
│   ├── ofdm.h
│   ├── phy.h
│   └── ...
├── src/
│   ├── modulation.c
│   ├── channel.c
│   ├── coding.c
│   ├── sync.c
│   ├── ofdm.c
│   └── ...
├── tests/
│   ├── test_modulation.c
│   ├── test_channel.c
│   ├── test_coding.c
│   └── ...
├── build/
│   ├── bin/
│   ├── lib/
│   └── obj/
└── reference/
    ├── ARCHITECTURE.md
    └── API.md
```

---

## Conventions

- **C99**, no external dependencies — just `math.h` and `stdlib.h`
- Paired `.md` tutorial + `.c` demo per chapter
- ASCII block diagrams, constellation plots, and BER curves in terminal
- Shared library (`src/` + `include/`) for reuse across chapters
- Progressive — each chapter builds on the previous
- Test suite with automated validation

---

## References

- Haykin, *Communication Systems* (4th ed.)
- Proakis & Salehi, *Digital Communications* (5th ed.)
- Sklar, *Digital Communications: Fundamentals and Applications*
- IEEE 802.11-2020 (Wi-Fi PHY)
- Bluetooth Core Specification v5.4
- IEEE 802.15.4-2020 (Zigbee PHY)
- Semtech AN1200.22 (LoRa modulation basics)
- ICAO Annex 10, Vol. IV (Mode S / ADS-B)

---

*Author: Nikhil Pandey*