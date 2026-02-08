# Chapter 03 — Channel Coding Basics

## Objective
Introduce error detection and correction codes for reliable transmission.

## Key Concepts
- **Parity**: Simplest error detection — single-bit overhead
- **Hamming(7,4)**: Corrects 1-bit errors in 7-bit codewords
- **CRC-16/32**: Cyclic Redundancy Check for burst error detection

## Demo
```bash
make build/bin/03-channel-coding && ./build/bin/03-channel-coding
```

## References
- Hamming, "Error Detecting and Error Correcting Codes" (1950)
- Proakis, *Digital Communications*, Ch. 6

---
[← Source Coding](../02-source-coding/README.md) | [Next: Pulse Shaping →](../04-pulse-shaping/README.md)
