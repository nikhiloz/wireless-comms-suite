# Chapter 20 — ADS-B / Mode S PHY

## Objective
Implement ADS-B transponder message encoding, PPM modulation, and decoding.

## Key Concepts
- **Mode S**: Selective interrogation — each aircraft has unique ICAO address
- **PPM**: Pulse Position Modulation — bit 1 = [high, low], bit 0 = [low, high]
- **Preamble**: 8 µs with 4 fixed pulses for detection and sync
- **CRC-24**: 24-bit CRC for error detection (generator 0xFFF409)

## References
- ICAO Annex 10, Volume IV

---
[← LoRa](../19-lora-phy/README.md) | [Next: Link Budget →](../21-link-budget/README.md)
