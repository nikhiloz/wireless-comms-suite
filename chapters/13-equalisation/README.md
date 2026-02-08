# Chapter 13 — Channel Equalisation

## Objective
Compensate for ISI introduced by frequency-selective channels.

## Key Concepts
- **ZF equaliser**: H⁻¹ — perfect ISI removal, noise enhancement at nulls
- **MMSE equaliser**: Minimises mean squared error, balances ISI and noise
- **LMS adaptive**: Iterative, low complexity, O(N) per sample
- **RLS adaptive**: Faster convergence, O(N²) per sample
- **DFE**: Decision feedback eliminates post-cursor ISI

---
[← Interleaving](../12-interleaving/README.md) | [Next: OFDM →](../14-ofdm/README.md)
