# Cross-Reference Map — Three Repository Integration

Maps every chapter/section across the three companion repositories.

## How to Read This Table

- **wireless-comms-suite** — Runnable C code + tutorial (this repo, target for enrichment)
- **dsp-tutorial-suite** — Foundation DSP algorithms in C
- **SDR_Notes** — Hardware/protocol markdown reference

| # | Topic | wireless-comms-suite | dsp-tutorial-suite | SDR_Notes |
|---|-------|---------------------|--------------------|-----------|
| 1 | Signals & Sequences | — | Ch01: Signals and Sequences | 01: Quadrature Signals |
| 2 | Sampling & Aliasing | — | Ch02: Sampling and Aliasing | 01: Sampling Theory |
| 3 | Complex Numbers / IQ | — | Ch03: Complex Numbers | 01: Quadrature Signals |
| 4 | Convolution / LTI | — | Ch04: LTI Systems | — |
| 5 | Z-Transform | — | Ch05: Z-Transform | — |
| 6 | Frequency Response | — | Ch06: Frequency Response | 04: Filters |
| 7 | DFT Theory | — | Ch07: DFT Theory | 04: FFT Spectrum |
| 8 | FFT Fundamentals | Ch14: OFDM (uses FFT) | Ch08: FFT Fundamentals | 04: FFT Spectrum |
| 9 | Window Functions | — | Ch09: Window Functions | — |
| 10 | FIR Filters | Ch04: Pulse Shaping (RC/RRC) | Ch10: Digital Filters | 04: FIR Filters |
| 11 | IIR Filter Design | — | Ch11: IIR Filter Design | 04: IIR Filters |
| 12 | Filter Structures | — | Ch12: Filter Structures | — |
| 13 | Spectral Analysis | — | Ch13: Spectral Analysis | 07: Spectrum Basics |
| 14 | PSD / Welch | — | Ch14: PSD Welch | 07: Noise Floor |
| 15 | Correlation | Ch10: Frame Sync (Barker corr.) | Ch15: Correlation | — |
| 16 | Overlap-Add/Save | — | Ch16: Overlap-Add-Save | — |
| 17 | Multirate DSP | — | Ch17: Multirate DSP | 04: Decimation/Interpolation |
| 18 | Fixed-Point | — | Ch18: Fixed-Point | — |
| 19 | Advanced FFT / Goertzel | — | Ch19: Advanced FFT | — |
| 20 | Hilbert Transform | — | Ch20: Hilbert Transform | — |
| 21 | Signal Averaging | — | Ch21: Signal Averaging | — |
| 22 | Advanced FIR (Remez) | — | Ch22: Advanced FIR | — |
| 23 | Adaptive Filters | Ch13: Equalisation (LMS/RLS) | Ch23: Adaptive Filters | — |
| 24 | Linear Prediction | — | Ch24: Linear Prediction | — |
| 25 | Parametric Spectral | — | Ch25: Parametric Spectral | — |
| 26 | Cepstrum / MFCC | — | Ch26: Cepstrum MFCC | — |
| 27 | 2-D DSP | — | Ch27: 2D DSP | — |
| 28 | Real-Time Streaming | — | Ch28: Real-Time Streaming | — |
| 29 | Optimisation | — | Ch29: Optimisation | — |
| 30 | Capstone Pipeline | Ch24: Transceiver | Ch30: Putting It Together | — |

---

## Wireless-Comms-Suite Chapters → Companion References

| wireless-comms-suite Chapter | dsp-tutorial-suite Prereqs | SDR_Notes Reference |
|------------------------------|---------------------------|---------------------|
| Ch01: System Overview | — | — |
| Ch02: Source Coding | — | — |
| Ch03: Channel Coding | — | — |
| Ch04: Pulse Shaping | Ch10 (FIR), Ch06 (Freq Response) | — |
| Ch05: Modulation | Ch03 (Complex), Ch08 (FFT) | 05: PSK, QAM |
| Ch06: AWGN Channel | — | — |
| Ch07: Fading Channels | — | — |
| Ch08: Timing Recovery | Ch23 (Adaptive) | — |
| Ch09: Carrier Sync | Ch03 (Complex) | — |
| Ch10: Frame Sync | Ch15 (Correlation) | — |
| Ch11: Conv + Viterbi | — | — |
| Ch12: Interleaving | — | — |
| Ch13: Equalisation | Ch23 (Adaptive) | — |
| Ch14: OFDM | Ch08 (FFT), Ch09 (Windows) | 05: OFDM |
| Ch15: Spread Spectrum | Ch15 (Correlation) | — |
| Ch16: Wi-Fi PHY | Ch08 (FFT), Ch23 (Adaptive) | — |
| Ch17: BT Baseband | — | — |
| Ch18: Zigbee PHY | Ch15 (Correlation) | — |
| Ch19: LoRa PHY | Ch08 (FFT) | 06: LoRa |
| Ch20: ADS-B PHY | — | 06: ADS-B Decoding |
| Ch21: Link Budget | — | 01: Decibels Power, RF Basics |
| Ch22: BER Simulation | — | — |
| Ch23: MIMO | Ch23 (Adaptive) | 09: Phased Arrays |
| Ch24: Transceiver | All above | — |
| **Ch25: FM Broadcast** (NEW) | Ch20 (Hilbert) | 05: FM Demod, 08: FM Receiver |
| **Ch26: AM Demod** (NEW) | Ch20 (Hilbert), Ch21 (Averaging) | 05: AM Demod |
| **Ch27: SSB** (NEW) | Ch20 (Hilbert) | 05: SSB |
| **Ch28: AIS** (NEW) | — | 06: AIS, 08: AIS Receiver |
| **Ch29: DMR** (NEW) | — | 06: DMR |
| **Ch30: POCSAG** (NEW) | — | 06: POCSAG, 08: Pager Decoder |
| **Ch31: NOAA APT** (NEW) | Ch21 (Averaging) | 06: NOAA APT, 08: Weather Satellite |
| **Ch32: OOK/ISM** (NEW) | — | 06: ISM 433MHz, 08: 433MHz Decoder |
| **Ch33: AGC** (NEW) | Ch21 (Averaging) | 04: AGC |
| **Ch34: Channeliser** (NEW) | Ch17 (Multirate) | 04: Decimation/Interpolation |
| **Ch35: Spectrum Sensing** (NEW) | Ch14 (PSD Welch) | 07: Noise Floor, Signal ID |
| **Ch36: Echo Cancel** (NEW) | Ch23 (Adaptive) | — |
| **Ch37: Fixed-Point** (NEW) | Ch18 (Fixed-Point) | — |
| **Ch38: IQ Imbalance** (NEW) | Ch20 (Hilbert), Ch15 (Correlation) | 09: Freq Calibration |
| **Ch39: Streaming** (NEW) | Ch28 (Real-Time Streaming) | 09: Remote SDR |

---

## SDR_Notes Sections → Implementation Status

| SDR_Notes Section | C Implementation | Status |
|-------------------|------------------|--------|
| 01: What Is SDR | — (conceptual) | N/A |
| 01: RF Basics | Ch21: Link Budget | ✅ |
| 01: Decibels & Power | `comms_utils.h`: `db_to_linear()` | ✅ |
| 01: Sampling Theory | — (covered in dsp-tutorial-suite) | In DSP repo |
| 01: Quadrature Signals | `comms_utils.h`: `Cplx` type | ✅ |
| 01: Antenna Basics | Ch21: Link Budget (Friis) | ✅ |
| 04: DSP Overview | — (covered in dsp-tutorial-suite) | In DSP repo |
| 04: FFT & Spectrum | `ofdm.h`: `fft()` / `ifft()` | ✅ |
| 04: FIR Filters | `modulation.h`: `raised_cosine()` | ✅ |
| 04: IIR Filters | — | In DSP repo |
| 04: Decimation/Interp | — | Planned: Ch34 |
| 04: AGC | — | Planned: Ch33 |
| 05: AM Demodulation | — | Planned: Ch26 |
| 05: FM Demodulation | — | Planned: Ch25 |
| 05: SSB | — | Planned: Ch27 |
| 05: PSK | `modulation.h`: BPSK/QPSK/8PSK | ✅ |
| 05: QAM | `modulation.h`: 16QAM/64QAM | ✅ |
| 05: FSK | `modulation.h`: GFSK | ✅ |
| 05: OFDM | Ch14, `ofdm.h` | ✅ |
| 06: ADS-B | Ch20, `phy.h` | ✅ |
| 06: AIS | — | Planned: Ch28 |
| 06: DMR | — | Planned: Ch29 |
| 06: ISM 433MHz | — | Planned: Ch32 |
| 06: LoRa | Ch19, `phy.h` | ✅ |
| 06: NOAA APT | — | Planned: Ch31 |
| 06: POCSAG | — | Planned: Ch30 |
| 07: Spectrum Basics | — | Planned: Ch35 |
| 07: Noise Floor | — | Planned: Ch35 |
| 08: FM Receiver | — | Planned: Ch25 |
| 08: ADS-B Receiver | Ch20 | ✅ |
| 08: AIS Receiver | — | Planned: Ch28 |
| 08: Pager Decoder | — | Planned: Ch30 |
| 08: Weather Satellite | — | Planned: Ch31 |
| 08: 433MHz Decoder | — | Planned: Ch32 |
| 09: Phased Arrays | Ch23: MIMO | ✅ (simplified) |
| 09: Frequency Calibration | — | Planned: Ch38 |

---

*Generated: 2026-02-14*