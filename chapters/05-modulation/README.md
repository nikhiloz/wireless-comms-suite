# Chapter 05 — Digital Modulation

## Objective
Implement and compare BPSK, QPSK, and 16-QAM modulation schemes.

## Key Concepts
- **BPSK**: 1 bit/symbol, most robust, ±1 on real axis
- **QPSK**: 2 bits/symbol, same BER as BPSK per bit, 4 constellation points
- **16-QAM**: 4 bits/symbol, higher throughput, needs more SNR
- **Gray Coding**: Adjacent constellation points differ by 1 bit
- **BER Curves**: Performance comparison over AWGN channel

## Demo
```bash
make build/bin/05-modulation && ./build/bin/05-modulation
```

## References
- Proakis, *Digital Communications*, Ch. 4
- Haykin, *Communication Systems*, §5

---
[← Pulse Shaping](../04-pulse-shaping/README.md) | [Next: AWGN Channel →](../06-awgn-channel/README.md)
