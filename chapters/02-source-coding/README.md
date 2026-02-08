# Chapter 02 — Source Coding

## Objective
Learn lossless compression techniques for reducing data redundancy before transmission.

## Key Concepts
- **Entropy**: H(X) = -Σ p(x) log₂ p(x) — lower bound on avg. bits/symbol
- **Huffman Coding**: Optimal prefix-free codes for known symbol probabilities
- **Run-Length Encoding**: Compress runs of repeated values

## Demo
```bash
make build/bin/02-source-coding && ./build/bin/02-source-coding
```

## References
- Shannon, "A Mathematical Theory of Communication" (1948)
- Huffman, "A Method for the Construction of Minimum-Redundancy Codes" (1952)

---
[← System Overview](../01-system-overview/README.md) | [Next: Channel Coding →](../03-channel-coding/README.md)
