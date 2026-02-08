# Chapter 10 — Frame Synchronisation

## Objective
Detect frame boundaries in a received signal using correlation with known sequences.

## Key Concepts
- **Barker codes**: Low sidelobe autocorrelation (7, 11, 13 lengths)
- **Correlation**: Sliding dot product to find preamble
- **Threshold**: SNR-dependent detection threshold
- **Scrambler**: Whitens data to avoid long runs of identical bits

---
[← Carrier Sync](../09-carrier-sync/README.md) | [Next: Convolutional Coding →](../11-convolutional-viterbi/README.md)
