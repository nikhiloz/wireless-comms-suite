# Digital Modulation Techniques Repo Hierarchy

## 1. Introduction
- Overview of modulation
- Difference between analog vs digital modulation
- Applications in communication systems

## 2. Fundamentals
- Signal representation (baseband vs passband)
- Constellation diagrams
- Bit Error Rate (BER) basics
- Noise models (AWGN, fading channels)

## 3. Digital Modulation Families
### Amplitude-based
- ASK (Amplitude Shift Keying)
- QAM (Quadrature Amplitude Modulation)
  - 4-QAM, 16-QAM, 64-QAM, etc.

### Frequency-based
- FSK (Frequency Shift Keying)
  - BFSK, MFSK, GFSK

### Phase-based
- PSK (Phase Shift Keying)
  - BPSK, QPSK, 8-PSK
- DPSK (Differential PSK)

### Hybrid / Advanced
- OFDM (Orthogonal Frequency Division Multiplexing)
- Spread Spectrum (DSSS, FHSS)
- CPM (Continuous Phase Modulation)

## 4. Performance Analysis
- BER vs SNR plots
- Spectral efficiency comparisons
- Complexity trade-offs

## 5. Implementation
- MATLAB/Python/C demos
- ASCII diagrams for clarity
- Recruiter-friendly worked examples

## 6. Applications
- Wireless communication (Wi-Fi, LTE, 5G)
- Satellite communication
- IoT devices

## 7. References
- Textbooks, standards (IEEE, ITU), and research papers


# Suggested Folder Structure

digital_modulation_repo/
│── README.md
│── docs/
│   ├── intro.md
│   ├── fundamentals.md
│   ├── performance.md
│── techniques/
│   ├── amplitude/
│   │   ├── ask.c
│   │   ├── qam.c
│   ├── frequency/
│   │   ├── fsk.c
│   ├── phase/
│   │   ├── bpsk.c
│   │   ├── qpsk.c
│   ├── hybrid/
│   │   ├── ofdm.c
│── examples/
│   ├── ber_vs_snr.py
│   ├── constellation_ascii.txt
│── applications/
│   ├── wifi.md
│   ├── iot.md