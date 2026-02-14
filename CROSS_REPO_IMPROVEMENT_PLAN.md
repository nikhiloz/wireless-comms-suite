# Wireless-Comms-Suite — Cross-Repository Enrichment Plan

**Goal:** Make `wireless-comms-suite` the definitive C-language practical resource for wireless communication, by back-porting DSP algorithms from `dsp-tutorial-suite` and implementing protocols documented in `SDR_Notes`.

**Created:** 2026-02-14  
**Scope:** `wireless-comms-suite` (target), informed by `dsp-tutorial-suite` + `SDR_Notes`  
**Current baseline:** 24 chapters, 9 library modules, 53 unit tests, 5 protocol PHYs

---

## Status Legend

| Symbol | Meaning |
|--------|---------|
| ⬜ | Not Started |
| 🟡 | In Progress |
| ✅ | Completed |
| ❌ | Blocked |

---

## Part 1 — New Protocol PHY Chapters (from SDR_Notes → C code)

SDR_Notes documents these protocols as markdown-only reference. Each becomes a new chapter with `demo.c`, `README.md`, library functions, unit tests, and diagrams.

| # | New Chapter | SDR_Notes Source | Key C Practicals | Status |
|---|-------------|-----------------|-------------------|--------|
| 1.1 | **Ch25: FM Broadcast Receiver** | `05_Modulation/FM_Demodulation.md`, `08_Practical_Projects/FM_Broadcast_Receiver.md` | FM discriminator, de-emphasis filter, stereo pilot detection, RDS frame extraction | ⬜ |
| 1.2 | **Ch26: AM / Envelope Detection** | `05_Modulation/AM_Demodulation.md` | Hilbert envelope detector, AGC, AM demod with carrier recovery | ⬜ |
| 1.3 | **Ch27: SSB Modulation** | `05_Modulation/SSB.md` | Weaver method, phasing method, Hilbert-based SSB mod/demod | ⬜ |
| 1.4 | **Ch28: AIS Marine Receiver** | `06_Protocols/AIS.md`, `08_Practical_Projects/Project_AIS_Receiver.md` | GMSK mod/demod, HDLC framing, NRZI decoding, CRC-16, MMSI parsing | ⬜ |
| 1.5 | **Ch29: DMR Digital Voice** | `06_Protocols/DMR.md` | 4FSK modulation, CACH/EMB extraction, slot structure, colour code | ⬜ |
| 1.6 | **Ch30: POCSAG Pager Decoder** | `06_Protocols/POCSAG.md`, `08_Practical_Projects/Project_Pager_Decoder.md` | FSK demod, batch/codeword framing, BCH error correction, numeric/alpha decode | ⬜ |
| 1.7 | **Ch31: NOAA APT Satellite Image** | `06_Protocols/NOAA_APT.md`, `08_Practical_Projects/Project_Weather_Satellite.md` | AM subcarrier demod, 2080 Hz sync detection, line-by-line image assembly (PGM output) | ⬜ |
| 1.8 | **Ch32: OOK / ISM 433 MHz** | `06_Protocols/ISM_433MHz.md`, `08_Practical_Projects/Project_433MHz_Decoder.md` | OOK demodulator, threshold detection, Manchester decode, protocol pattern matching | ⬜ |

### Library Additions for Part 1

| New Header | Functions | Depends On |
|------------|-----------|------------|
| `analog_demod.h` | `fm_discriminator()`, `fm_deemphasis()`, `am_envelope()`, `ssb_weaver_mod()`, `ssb_weaver_demod()` | `comms_utils.h`, DSP hilbert |
| `phy.h` (extend) | `ais_build_frame()`, `ais_decode_frame()`, `dmr_build_burst()`, `pocsag_decode_batch()`, `noaa_apt_demod_line()`, `ook_demod()` | `coding.h`, `modulation.h` |

### Tests for Part 1

| Test File | Min Tests |
|-----------|-----------|
| `test_analog_demod.c` | 6 (FM round-trip, AM round-trip, SSB round-trip, de-emphasis, carrier recovery, stereo pilot) |
| `test_phy.c` (extend) | +8 (AIS encode/decode, DMR 4FSK, POCSAG BCH, NOAA sync detect, OOK threshold) |

---

## Part 2 — Practical Engineering Chapters (from DSP backport)

These use algorithms already implemented in `dsp-tutorial-suite` but apply them to wireless communication scenarios — each gets its own chapter with a practical demo.

| # | New Chapter | DSP Module Used | Key C Practicals | Status |
|---|-------------|-----------------|-------------------|--------|
| 2.1 | **Ch33: AGC — Automatic Gain Control** | `dsp: averaging.c` | Power measurement, attack/decay loop, AGC for bursty signals (Wi-Fi preamble lock) | ⬜ |
| 2.2 | **Ch34: Multirate Channeliser** | `dsp: multirate.c` | Polyphase decimation, DDC (digital downconverter), wideband → narrowband extraction, SDR channeliser | ⬜ |
| 2.3 | **Ch35: Spectrum Sensing / Energy Detection** | `dsp: spectrum.c, spectral_est.c` | Welch PSD estimator, noise floor estimation, energy detector threshold, cognitive radio decision | ⬜ |
| 2.4 | **Ch36: Echo & Interference Cancellation** | `dsp: adaptive.c` | LMS/NLMS echo canceller, interference cancellation for co-channel, near-far problem | ⬜ |
| 2.5 | **Ch37: Fixed-Point Wireless** | `dsp: fixed_point.c` | Q15 modulator, Q15 FIR pulse shaper, fixed-point BPSK TX/RX chain, SNR degradation analysis | ⬜ |
| 2.6 | **Ch38: IQ Imbalance Correction** | `dsp: hilbert.c, correlation.c` | IQ gain/phase imbalance model, blind estimation, Gram-Schmidt correction, EVM before/after | ⬜ |
| 2.7 | **Ch39: Streaming Radio Pipeline** | `dsp: streaming.c, realtime.c` | Ring buffer, block-based TX/RX, real-time FFT spectrum display, frame-boundary handling | ⬜ |

### Library Additions for Part 2

| New Header | Functions | Depends On |
|------------|-----------|------------|
| `agc.h` | `agc_init()`, `agc_process()`, `agc_set_target()` | `comms_utils.h` |
| `channeliser.h` | `ddc_init()`, `ddc_process()`, `polyphase_channelise()` | `ofdm.h` (FFT), `comms_utils.h` |
| `impairments.h` | `iq_imbalance_apply()`, `iq_imbalance_correct()`, `iq_estimate_params()` | `comms_utils.h` |

### Tests for Part 2

| Test File | Min Tests |
|-----------|-----------|
| `test_agc.c` | 4 (step response, attack/decay timing, burst signal, saturation) |
| `test_channeliser.c` | 4 (decimation identity, channel extraction, aliasing rejection, reconstruction) |
| `test_impairments.c` | 4 (imbalance round-trip, EVM improvement, blind estimation accuracy, DC offset removal) |

---

## Part 3 — Modern FEC Chapters (Gap in current suite)

| # | New Chapter | Relevance | Key C Practicals | Status |
|---|-------------|-----------|-------------------|--------|
| 3.1 | **Ch40: Turbo Codes** | 3G/LTE, deep-space | Parallel concatenated convolutional codes, MAP/BCJR decoder, iterative decoding, EXIT chart analysis | ⬜ |
| 3.2 | **Ch41: LDPC Codes** | Wi-Fi 6, 5G NR, DVB-S2 | Parity-check matrix construction, belief propagation decoder, min-sum approximation, BER waterfall curve | ⬜ |
| 3.3 | **Ch42: Polar Codes** | 5G NR control channel | Channel polarisation, successive cancellation decoder, CRC-aided list decoder | ⬜ |
| 3.4 | **Ch43: Reed-Solomon Codes** | DVB, QR codes, CD | GF(2^8) arithmetic, syndrome computation, Berlekamp-Massey, Forney algorithm | ⬜ |

### Library Additions for Part 3

| New Header | Functions |
|------------|-----------|
| `coding.h` (extend) | `turbo_encode()`, `turbo_decode_map()`, `ldpc_encode()`, `ldpc_decode_bp()`, `polar_encode()`, `polar_decode_sc()`, `rs_encode()`, `rs_decode()` |

### Tests for Part 3

| Test File | Min Tests |
|-----------|-----------|
| `test_coding.c` (extend) | +10 (turbo round-trip, LDPC waterfall point, polar SC decode, RS error correction up to t errors) |

---

## Part 4 — Content Enrichment (from FUTURE_WORK.md + DSP parity)

These enhance existing chapters without adding new ones.

### 4.1 Gnuplot Data Plot Pipeline (FUTURE_WORK.md Priority 1)

| # | Task | Chapters Affected | Status |
|---|------|-------------------|--------|
| 4.1.1 | Add `--csv` flag to all 24 existing demos | All | ⬜ |
| 4.1.2 | Write gnuplot `.gp` scripts per chapter | All with numeric output | ⬜ |
| 4.1.3 | Generate PNGs and embed in READMEs | ~25-30 plots | ⬜ |
| 4.1.4 | BER curves: simulated vs theoretical | Ch01, 05, 22 | ⬜ |
| 4.1.5 | Eye diagrams | Ch04 | ⬜ |
| 4.1.6 | Constellation scatter plots | Ch05 | ⬜ |
| 4.1.7 | Noise/fading histograms | Ch06, 07 | ⬜ |
| 4.1.8 | Loop convergence curves | Ch08, 09 | ⬜ |
| 4.1.9 | Correlation peak plot | Ch10 | ⬜ |
| 4.1.10 | Coded vs uncoded BER | Ch11 | ⬜ |
| 4.1.11 | OFDM spectrum | Ch14 | ⬜ |
| 4.1.12 | Chirp spectrogram (LoRa) | Ch19 | ⬜ |
| 4.1.13 | Link margin vs distance | Ch21 | ⬜ |
| 4.1.14 | MIMO diversity gain | Ch23 | ⬜ |

### 4.2 Tutorial.md Files (DSP Suite Parity)

The DSP suite has `tutorial.md` with detailed theory in every chapter. The wireless suite only has `README.md`.

| # | Task | Status |
|---|------|--------|
| 4.2.1 | Add `tutorial.md` to all 24 existing chapters with: core equations (GitHub math), derivation sketches, worked numerical examples, design trade-off tables | ⬜ |
| 4.2.2 | Add `tutorial.md` to all new chapters (Part 1, 2, 3) | ⬜ |

### 4.3 Exercises per Chapter (FUTURE_WORK.md Priority 3)

| # | Task | Status |
|---|------|--------|
| 4.3.1 | Add 3-5 exercises to each of the 24 existing chapter READMEs (conceptual, hands-on, design) | ⬜ |
| 4.3.2 | Add exercises to all new chapters | ⬜ |

### 4.4 Additional Diagrams (FUTURE_WORK.md Priority 4)

| # | Task | Status |
|---|------|--------|
| 4.4.1 | State machine diagrams: Viterbi trellis (Ch11), timing loop (Ch08), PLL states (Ch09) | ⬜ |
| 4.4.2 | Sequence diagrams: Wi-Fi PPDU handshake (Ch16), BT packet exchange (Ch17) | ⬜ |
| 4.4.3 | Timing/waveform diagrams: Manchester (Ch04), PPM (Ch20), OFDM CP (Ch14) | ⬜ |

### 4.5 Reference Materials

| # | Task | Status |
|---|------|--------|
| 4.5.1 | Create `reference/CHEATSHEET.md` — all key formulas, modulation comparison, protocol PHY params side-by-side, dB conversions | ⬜ |
| 4.5.2 | Create `reference/CROSS_REFERENCES.md` — mapping between wireless-comms-suite chapters ↔ dsp-tutorial-suite chapters ↔ SDR_Notes sections | ⬜ |
| 4.5.3 | Update `reference/API.md` for all new modules | ⬜ |
| 4.5.4 | Update `reference/ARCHITECTURE.md` with new module dependency graph | ⬜ |

---

## Part 5 — Build System & Infrastructure

| # | Task | Status |
|---|------|--------|
| 5.1 | Extend Makefile for new chapters (Ch25–Ch43) | ⬜ |
| 5.2 | Add `make plots` target that runs gnuplot on all `.gp` scripts | ⬜ |
| 5.3 | Add `make csv` target that runs all demos with `--csv` | ⬜ |
| 5.4 | Add Valgrind / ASan CI for all new code | ⬜ |
| 5.5 | Add `make docs` target to render all PlantUML diagrams | ⬜ |
| 5.6 | Update `.clang-format` to cover new files | ⬜ |
| 5.7 | Add GitHub Actions CI (build + test + memcheck) | ⬜ |

---

## Execution Priority & Schedule

### Phase A — Quick Wins (Est. 1-2 weeks)

| Priority | Items | Deliverables | Why First |
|----------|-------|--------------|-----------|
| **A1** | 4.1.1–4.1.3 | CSV output + gnuplot pipeline | Highest visual impact; infrastructure for all future plots |
| **A2** | 4.5.1 | Cheat sheet | Single file, high reference value |
| **A3** | 4.5.2 | Cross-reference map | Connects the 3 repos |

### Phase B — New Protocol PHYs (Est. 3-4 weeks)

| Priority | Items | Deliverables | Why Next |
|----------|-------|--------------|----------|
| **B1** | 1.1 (FM) | Ch25 + analog_demod.h | Most requested SDR practical; uses Hilbert from DSP suite |
| **B2** | 1.6 (POCSAG) | Ch30 | Simple FSK protocol, good BCH coding practical |
| **B3** | 1.4 (AIS) | Ch28 | GMSK + HDLC, reuses existing modulation module |
| **B4** | 1.8 (OOK) | Ch32 | Simplest protocol, good for beginners |
| **B5** | 1.2, 1.3 (AM, SSB) | Ch26, 27 | Classic analog, pairs with FM chapter |
| **B6** | 1.7 (NOAA APT) | Ch31 | Unique satellite practical, generates images |
| **B7** | 1.5 (DMR) | Ch29 | Most complex; deferred until 4FSK is proven |

### Phase C — Practical Engineering (Est. 2-3 weeks)

| Priority | Items | Deliverables | Why Here |
|----------|-------|--------------|----------|
| **C1** | 2.1 (AGC) | Ch33 | Foundation for all receiver chains |
| **C2** | 2.2 (Channeliser) | Ch34 | Key SDR building block |
| **C3** | 2.6 (IQ Imbalance) | Ch38 | Practical real-world impairment |
| **C4** | 2.5 (Fixed-Point) | Ch37 | Embedded focus, unique content |
| **C5** | 2.3, 2.4, 2.7 | Ch35, 36, 39 | Advanced topics |

### Phase D — Modern FEC (Est. 2-3 weeks)

| Priority | Items | Deliverables | Why Last |
|----------|-------|--------------|----------|
| **D1** | 3.4 (Reed-Solomon) | Ch43 | Most practical (DVB, QR codes) |
| **D2** | 3.2 (LDPC) | Ch41 | Wi-Fi 6 / 5G relevance |
| **D3** | 3.1 (Turbo) | Ch40 | LTE relevance |
| **D4** | 3.3 (Polar) | Ch42 | Cutting-edge, 5G NR |

### Phase E — Content Polish (Ongoing, parallel with B-D)

| Priority | Items | Deliverables |
|----------|-------|--------------|
| **E1** | 4.2 (tutorial.md) | Theory write-ups for each chapter as it ships |
| **E2** | 4.3 (exercises) | 3-5 exercises per chapter |
| **E3** | 4.4 (diagrams) | State/sequence/timing PUMLs |
| **E4** | 5.x (infra) | CI, docs target, format |

---

## Final State After Completion

| Metric | Current | After Plan |
|--------|---------|------------|
| Chapters | 24 | **43** |
| Library modules | 9 headers | **13 headers** (+analog_demod, agc, channeliser, impairments) |
| Protocol PHYs | 5 (Wi-Fi, BT, Zigbee, LoRa, ADS-B) | **13** (+FM, AM, SSB, AIS, DMR, POCSAG, NOAA APT, OOK) |
| Unit tests | 53 | **~95** |
| Gnuplot PNGs | 0 | **~40** |
| tutorial.md files | 0 | **43** |
| Exercises | 0 | **~170** (43 chapters x ~4 each) |
| PlantUML diagrams | 43 | **~70** |
| FEC algorithms | Hamming, conv/Viterbi, CRC | + **Turbo, LDPC, Polar, Reed-Solomon** |
| SDR_Notes protocols w/ C impl | 5 of 13 | **13 of 13** |
| DSP practical backports | 0 | **7 chapters** |

---

## Change Log

| Date | Items | Change |
|------|-------|--------|
| 2026-02-14 | — | Initial cross-repo improvement plan created |
