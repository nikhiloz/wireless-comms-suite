# Chapter 14 — OFDM

## Objective
Implement a complete OFDM transceiver: IFFT modulation, cyclic prefix, FFT demodulation, pilot-based channel estimation.

## Key Concepts
- **OFDM**: Orthogonal Frequency Division Multiplexing — parallel narrowband subcarriers
- **Cyclic prefix**: Guard interval eliminates ISI, makes channel appear circular
- **Pilot tones**: Known symbols for channel estimation and tracking
- **Channel estimation**: Least squares at pilots + linear interpolation

---
[← Equalisation](../13-equalisation/README.md) | [Next: Spread Spectrum →](../15-spread-spectrum/README.md)
