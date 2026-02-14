# Chapter 25 — FM Broadcast Receiver

## Objective
Implement a complete FM broadcast receiver pipeline: FM modulation, atan2
discriminator demodulation, pre-emphasis / de-emphasis filtering, and stereo
pilot detection.  Also demonstrates AM envelope detection for comparison.

## Key Concepts
- **FM Modulation**: Instantaneous frequency proportional to audio amplitude;
  phase = integral of message signal × 2π × Δf
- **FM Discriminator**: arg(s[n] · conj(s[n-1])) recovers instantaneous
  frequency from complex baseband IQ
- **De-emphasis**: Single-pole IIR low-pass (τ = 75 µs USA / 50 µs Europe)
  attenuates high-frequency noise boosted by differentiation
- **Pre-emphasis**: Inverse high-pass filter applied at transmitter to
  compensate for noise shaping in FM
- **Stereo Pilot**: 19 kHz tone in composite signal; doubled to 38 kHz
  sub-carrier for L−R difference signal
- **Capture Effect**: FM exhibits threshold behaviour — above SNR threshold,
  the stronger signal dominates
- **AM Envelope Detection**: |s(t)| recovers amplitude-modulated audio;
  simpler but no noise advantage

## Demo

```bash
make build/bin/25-fm-broadcast
./build/bin/25-fm-broadcast
```

The demo runs eight sections:
1. Generate 1 kHz test tone at 240 kHz sample rate
2. FM modulate with ±75 kHz deviation (normalised 0.3125)
3. Pass through AWGN channel at 20 dB SNR
4. FM demodulate via atan2 discriminator
5. Apply 75 µs de-emphasis filter
6. Verify pre-emphasis ↔ de-emphasis round-trip
7. Demonstrate AM modulation + envelope detection
8. Full mono FM receiver pipeline with output SNR measurement

## Formulas

| Quantity | Formula |
|----------|---------|
| FM baseband | s(t) = exp(j·2π·Δf · ∫ m(τ) dτ) |
| Discriminator | f_inst[n] = arg(s[n]·s*[n-1]) / π |
| De-emphasis | H(z) = (1−α)/(1−α·z⁻¹), α = e^{−1/(τ·fs)} |
| Carson bandwidth | BW = 2·(Δf + f_audio) |
| FM SNR gain | SNR_out ≈ 3·β²·(β+1)·SNR_in (above threshold) |

## References
- Haykin, *Communication Systems*, 4th ed. — Ch. 4 (FM Demodulation)
- dsp-tutorial-suite Ch. 21 (FM Modulation) — complementary DSP perspective
- SDR_Notes §5 (Modulation & Demodulation) — practical SDR context

---
## Diagrams

### Concept — FM Broadcast Receiver
```
Audio ──► Pre-emphasis ──► FM Modulator ──► Channel (AWGN)
                                                │
          ┌─────────────────────────────────────┘
          ▼
     FM Discriminator ──► De-emphasis ──► Audio Out
     arg(s[n]·s*[n-1])    IIR LPF(τ)
```

### Code Flow — `demo.c`
```
main()
 ├─ 1. Generate 1 kHz test tone
 ├─ 2. fm_modulate() → IQ samples
 ├─ 3. channel_awgn() → noisy IQ
 ├─ 4. fm_demodulate() → recovered audio
 ├─ 5. fm_deemphasis() → filtered audio
 ├─ 6. preemphasis/deemphasis round-trip test
 ├─ 7. am_modulate() + am_envelope_detect()
 └─ 8. Full pipeline → SNR measurement
```

---
[← Transceiver](../24-transceiver/README.md) | [Next: AM/SSB Receiver →](../26-am-ssb-receiver/README.md)