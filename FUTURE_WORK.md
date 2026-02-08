# Future Work — Content Enrichment Roadmap

Captured: 8 February 2026

---

## Priority 1 — Gnuplot Data Plots (High Impact)

Add `--csv` output mode to demos so they dump raw data, then gnuplot scripts generate publication-quality PNGs embedded in chapter READMEs.

| Chapter | Plot Type | What It Shows |
|---------|-----------|---------------|
| 01, 05, 22 | **BER curves** | Simulated vs theoretical, multiple modulations on one graph |
| 04 | **Pulse/filter response** | Time-domain RC/RRC pulses + frequency-domain rolloff |
| 04 | **Eye diagram** | Overlaid traces showing ISI, noise margin, timing jitter |
| 05 | **Constellation scatter** | Received IQ points with noise cloud around ideal positions |
| 06, 07 | **Noise/fading histogram** | Gaussian PDF fit, Rayleigh envelope distribution |
| 08, 09 | **Convergence curves** | Timing error / phase error vs iteration (loop lock-in) |
| 10 | **Correlation peak** | Barker-13 autocorrelation with sidelobes visible |
| 11 | **Coded vs uncoded BER** | Viterbi coding gain clearly visible |
| 13 | **LMS MSE curve** | Steady-state convergence of adaptive equaliser |
| 14 | **OFDM spectrum** | Subcarrier orthogonality, CP overhead |
| 19 | **Chirp spectrogram** | Frequency vs time for LoRa upchirp/downchirp |
| 21 | **Link margin vs distance** | Friis path loss with fade margin bands |
| 23 | **MIMO diversity gain** | SISO vs Alamouti vs MRC BER slopes |

**Approach**: Modify demos to optionally dump CSV (`./build/bin/05-modulation --csv > data.csv`), write gnuplot `.gp` scripts per chapter under `chapters/XX/plots/`, generate PNGs, embed in READMEs. Estimated ~25–30 plots total.

---

## Priority 2 — Theory & Equations in READMEs (Medium-High Impact)

Expand each chapter README with a **"Theory"** section containing:

- Core equations rendered in GitHub math (e.g., `` $`BER = Q(\sqrt{2 E_b/N_0})`$ ``)
- Derivation sketches (not full proofs — "where the formula comes from")
- Worked numerical examples (e.g., "At Eb/N0 = 10 dB, BPSK BER = 3.87 × 10⁻⁶")
- Design trade-off tables (e.g., modulation: bits/sym vs dB penalty vs complexity)

---

## Priority 3 — Exercises per Chapter (Medium Impact)

Add 3–5 questions at the end of each chapter README:

- Conceptual: "What happens to BER if you double the noise power?"
- Hands-on: "Modify demo.c to use 64-QAM — predict the SNR penalty before running"
- Design: "Change the interleaver depth from 8 to 4 — which burst length can it still handle?"

Makes the suite usable as a teaching/self-study resource.

---

## Priority 4 — Additional PlantUML Diagram Types (Medium Impact)

Beyond existing concept + code flow diagrams:

- **State machine diagrams** — Viterbi trellis states (ch11), timing loop states (ch08), PLL lock states (ch09)
- **Sequence diagrams** — Wi-Fi PPDU TX/RX handshake (ch16), BT packet exchange (ch17)
- **Timing/waveform diagrams** — Manchester encoding transitions (ch04), PPM bit slots (ch20), OFDM symbol timing with CP (ch14)

Estimated ~12–15 new PUML files.

---

## Priority 5 — Quick-Reference Cheat Sheet (Low-Medium Impact)

A single `reference/CHEATSHEET.md` with:

- All key formulas on one page
- Modulation comparison table (scheme, bits/sym, BER formula, min Eb/N0 at 10⁻⁵)
- Protocol PHY parameter summary (Wi-Fi, BT, Zigbee, LoRa, ADS-B side by side)
- Common dB conversions and unit reference

---

## Summary

| Priority | Item | Effort | Deliverables |
|----------|------|--------|--------------|
| **1** | Gnuplot data plots | High | ~25–30 .gp scripts + PNGs + demo CSV flags |
| **2** | Theory + equations | Medium | 24 README edits |
| **3** | Exercises | Low | 24 README appends |
| **4** | State/sequence/timing PUMLs | Medium | ~12–15 new diagrams |
| **5** | Cheat sheet | Low | 1 new reference file |
