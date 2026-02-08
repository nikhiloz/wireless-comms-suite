# Chapter 08 — Symbol Timing Recovery

## Objective
Recover symbol timing at the receiver when the sampling clock isn't synchronised to the transmitter.

## Key Concepts
- **Gardner TED**: Non-data-aided, works at 2 samples/symbol
- **Mueller-Muller TED**: Decision-directed, uses detected symbols
- **Loop filter**: PI controller drives NCO for sample timing adjustment
- **Interpolation**: Linear/cubic interpolation between samples

---
[← Fading Channels](../07-fading-channels/README.md) | [Next: Carrier Sync →](../09-carrier-sync/README.md)
