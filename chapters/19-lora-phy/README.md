# Chapter 19 — LoRa PHY

## Objective
Implement LoRa CSS (Chirp Spread Spectrum) modulation and demodulation.

## Key Concepts
- **CSS**: Chirp Spread Spectrum — frequency sweeps encode data via cyclic shifts
- **Spreading factor**: SF7–SF12, trade-off data rate vs range
- **Dechirping**: Multiply by conjugate base chirp, then FFT to find peak bin
- **Preamble**: Upchirps for detection + downchirps for sync

## References
- Semtech AN1200.22 (LoRa Modulation Basics)

---
[← Zigbee](../18-zigbee-phy/README.md) | [Next: ADS-B →](../20-adsb-phy/README.md)
