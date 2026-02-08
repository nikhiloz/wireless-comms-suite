# Chapter 18 — Zigbee / IEEE 802.15.4 PHY

## Objective
Implement the 2.4 GHz IEEE 802.15.4 PHY: O-QPSK modulation with DSSS chip spreading.

## Key Concepts
- **O-QPSK**: Offset QPSK — half-symbol stagger, half-sine pulse
- **DSSS**: 4-bit symbol → 32-chip PN sequence (processing gain ≈ 9 dB)
- **PPDU**: SHR (preamble+SFD) → PHR (frame length) → PSDU (data)
- **Data rate**: 250 kbps, chip rate 2 Mchip/s

## References
- IEEE 802.15.4-2020, §12

---
[← Bluetooth](../17-bluetooth-baseband/README.md) | [Next: LoRa →](../19-lora-phy/README.md)
