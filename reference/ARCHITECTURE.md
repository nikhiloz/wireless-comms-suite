# Architecture — wireless-comms-suite

## Design Philosophy

wireless-comms-suite is a **pure C99 teaching library** that implements a
complete digital communications system stack.  It bridges two companion
repositories:

| Repository | Focus |
|-----------|-------|
| **dsp-tutorial-suite** | Pure DSP primitives — FFT, filters, spectral analysis |
| **SDR_Notes** | SDR documentation — hardware, software, protocols, projects |
| **wireless-comms-suite** | Wireless comms — TX/RX chain, coding, sync, real protocol PHY |

Every design decision optimises for *learning*: the code must be
readable, self-contained, and runnable on any POSIX system.

---

## Constraints

| Rule | Detail |
|------|--------|
| Language | C99 (`-std=c99 -Wall -Wextra -Werror`) |
| Dependencies | `math.h`, `stdlib.h`, `string.h`, `stdio.h` only — no external libs |
| Memory model | Caller-allocates-buffers; no hidden `malloc` in hot paths |
| Naming | `module_verb_noun()` snake_case; module prefix on every public symbol |
| Error handling | Return lengths / status codes; no `errno` or exceptions |
| Documentation | Doxygen `@file`/`@brief`; heavy ASCII section dividers |

---

## Repository Layout

```
wireless-comms-suite/
├── README.md                  ← Top-level overview + 24-chapter table
├── Makefile                   ← GNU Makefile (release/debug/test/chapters)
│
├── include/                   ← Public headers (9 modules)
│   ├── comms_utils.h          ← Core types (Cplx), PRNG, bit helpers, plot, maths
│   ├── modulation.h           ← BPSK/QPSK/16-QAM/64-QAM/GFSK/O-QPSK, pulse shaping
│   ├── coding.h               ← Huffman, RLE, CRC, Hamming, convolutional/Viterbi
│   ├── channel.h              ← AWGN, Rayleigh/Rician, multipath, Doppler
│   ├── sync.h                 ← Timing recovery, carrier sync, frame sync, scrambler
│   ├── ofdm.h                 ← FFT/IFFT, OFDM modulator/demodulator, channel estimation
│   ├── spread_spectrum.h      ← PN sequences, DSSS, FHSS, Zigbee chip mapping
│   ├── equaliser.h            ← ZF, MMSE, LMS, RLS, DFE adaptive equalisers
│   └── phy.h                  ← Wi-Fi/BT/Zigbee/LoRa/ADS-B PHY, MIMO, link budget
│
├── src/                       ← Implementations (1:1 with headers)
│   ├── comms_utils.c
│   ├── modulation.c
│   ├── coding.c
│   ├── channel.c
│   ├── sync.c
│   ├── ofdm.c
│   ├── spread_spectrum.c
│   ├── equaliser.c
│   └── phy.c
│
├── tests/                     ← Test framework + per-module test suites
│   ├── test_framework.h       ← Header-only: TEST_SUITE, TEST_ASSERT, TEST_ASSERT_NEAR
│   ├── test_modulation.c
│   ├── test_coding.c
│   ├── test_channel.c
│   ├── test_sync.c
│   ├── test_ofdm.c
│   ├── test_spread.c
│   ├── test_equaliser.c
│   └── test_phy.c
│
├── chapters/                  ← 24 progressive tutorials
│   ├── 01-system-overview/    ← Each: demo.c + README.md
│   ├── 02-source-coding/
│   │   ...
│   └── 24-transceiver/
│
├── build/                     ← Build output (generated)
│   ├── obj/                   ← Object files
│   ├── lib/                   ← libwireless_comms.a
│   └── bin/                   ← Chapter & test binaries
│
└── reference/                 ← This directory
    ├── ARCHITECTURE.md        ← You are here
    └── API.md                 ← Full API surface reference
```

---

## Module Dependency Graph

```
 ┌──────────────────────────────────────────────────────────────┐
 │                        phy.h                                 │
 │  Wi-Fi · Bluetooth · Zigbee · LoRa · ADS-B · MIMO · Link   │
 └──────┬───────┬───────┬───────┬──────┬───────┬───────────────┘
        │       │       │       │      │       │
        ▼       ▼       ▼       ▼      ▼       ▼
   ┌────────┐ ┌──────┐ ┌────┐ ┌──────────┐ ┌────────────┐
   │ ofdm.h │ │sync.h│ │mod │ │spread_sp.│ │equaliser.h │
   └───┬────┘ └──┬───┘ │.h  │ │   .h     │ └──────┬─────┘
       │         │     └─┬──┘ └─────┬────┘        │
       │         │       │          │              │
       ▼         ▼       ▼          ▼              ▼
   ┌─────────────────────────────────────────────────────┐
   │                   channel.h                          │
   └───────────────────────┬─────────────────────────────┘
                           │
                           ▼
   ┌─────────────────────────────────────────────────────┐
   │                   coding.h                           │
   └───────────────────────┬─────────────────────────────┘
                           │
                           ▼
   ┌─────────────────────────────────────────────────────┐
   │                 comms_utils.h                        │
   │   Cplx type · PRNG · bit helpers · plot · maths     │
   └─────────────────────────────────────────────────────┘
```

All modules ultimately depend on `comms_utils.h` for the `Cplx` type,
PRNG, and helper utilities.

---

## Build System

### Targets

| Target | Description |
|--------|-------------|
| `make` / `make release` | Build static library `build/lib/libwireless_comms.a` |
| `make debug` | Debug build with `-g -O0 -DDEBUG` |
| `make lib` | Library only |
| `make chapters_build` | Compile all 24 chapter demos |
| `make tests_build` | Compile all test binaries |
| `make test` | Build + run all tests |
| `make run` | Build + run all chapter demos sequentially |
| `make format` | `clang-format -i` all source |
| `make lint` | Static analysis with `cppcheck` |
| `make memcheck` | Valgrind on all tests |
| `make clean` | Remove `build/` |
| `make distclean` | Remove `build/` and editor backups |
| `make install` | Install headers + library to `PREFIX` |

### Build Flow

```
src/*.c → build/obj/*.o → build/lib/libwireless_comms.a
                                      ↕
chapters/*/demo.c ──────→ build/bin/NN-chapter-name
tests/test_*.c ─────────→ build/bin/test_module
```

---

## Coding Conventions

### Naming
- Functions: `module_verb_noun()` — e.g., `mod_modulate()`, `channel_awgn()`
- Types/structs: `PascalCase` — e.g., `OfdmParams`, `LmsEqualiser`
- Enums: `MOD_BPSK`, `WIFI_RATE_6M` (UPPER_SNAKE prefix)
- Constants: `BARKER_13`, `BT_ACCESS_CODE_LEN`

### Memory
- Caller allocates all output buffers
- Functions return output length where applicable
- `init`/`free` pairs for stateful objects (`LmsEqualiser`, `Interleaver`)

### Documentation
- Every file: `@file` + `@brief` Doxygen header
- Section dividers: `/* ════════ section ════════ */`
- Cross-references to SDR_Notes and dsp-tutorial-suite chapters

---

## Chapter Progression

### Part I — Foundations (Ch 01–05)
System overview → source coding → channel coding → pulse shaping → modulation

### Part II — Channel & Synchronisation (Ch 06–10)
AWGN → fading → timing recovery → carrier sync → frame sync

### Part III — Advanced Techniques (Ch 11–15)
Convolutional/Viterbi → interleaving → equalisation → OFDM → spread spectrum

### Part IV — Real Protocol PHY (Ch 16–20)
Wi-Fi 802.11 → Bluetooth → Zigbee 802.15.4 → LoRa → ADS-B

### Part V — System Design (Ch 21–24)
Link budget → BER simulation → MIMO → Full transceiver capstone

---

## Test Framework

The custom header-only test framework (`tests/test_framework.h`) provides:

```c
TEST_SUITE("Module Name")
TEST_CASE_BEGIN("test description")
    TEST_ASSERT(condition);
    TEST_ASSERT_NEAR(actual, expected, tolerance);
    TEST_ASSERT_INT_EQ(actual, expected);
TEST_PASS_STMT
TEST_SUMMARY
```

Uses `goto _test_next` for assertion flow control (no `setjmp`/`longjmp`).
Each test binary is self-contained and returns 0 on success, nonzero on failure.

---

## Cross-References

| This Library Concept | dsp-tutorial-suite Chapter | SDR_Notes Section |
|---------------------|---------------------------|-------------------|
| FFT/IFFT | Ch 05 — DFT/FFT | 04_DSP_Fundamentals/ |
| Filter design | Ch 07-09 — FIR/IIR | 04_DSP_Fundamentals/ |
| Spectral analysis | Ch 13 — Windowing | 07_Spectrum_Analysis/ |
| Modulation | Ch 15 — AM/FM/PM | 05_Modulation/ |
| OFDM | — | 06_Protocols/ |
| Protocol PHY | — | 06_Protocols/ |
| SDR hardware | — | 02_Hardware/ |
