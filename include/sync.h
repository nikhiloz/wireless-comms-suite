/**
 * @file sync.h
 * @brief Synchronisation — timing recovery, carrier sync, frame sync.
 *
 * Covers:
 *   - Symbol timing recovery (Gardner, Mueller-Muller)
 *   - Carrier synchronisation (Costas loop, PLL)
 *   - Frame sync (preamble correlation, Barker codes)
 */

#ifndef SYNC_H
#define SYNC_H

#include "comms_utils.h"
#include <stdint.h>

/* ── Symbol Timing Recovery ──────────────────────────────────────── */

typedef struct {
    double mu;           /* fractional delay estimate [0, 1)        */
    double mu_step;      /* nominal step = 1/sps                    */
    double loop_bw;      /* loop bandwidth (normalised)             */
    double damping;      /* damping factor ζ                        */
    double ki, kp;       /* loop filter gains (integral, prop.)     */
    double integrator;   /* loop filter state                       */
    int    sps;          /* samples per symbol                      */
} TimingRecovery;

/**
 * @brief Initialise Gardner/Mueller-Muller timing recovery loop.
 * @param tr      Struct to initialise
 * @param sps     Nominal samples per symbol
 * @param loop_bw Loop bandwidth (try 0.01 to start)
 * @param damping Damping factor (1.0 = critically damped)
 */
void timing_init(TimingRecovery *tr, int sps, double loop_bw, double damping);

/**
 * @brief Process block and recover symbol-rate samples.
 * @param tr     Timing recovery state
 * @param in     Input oversampled signal
 * @param n      Number of input samples
 * @param out    Output symbol-rate samples
 * @return Number of output symbols
 */
int timing_recover_gardner(TimingRecovery *tr, const Cplx *in, int n, Cplx *out);

int timing_recover_mm(TimingRecovery *tr, const Cplx *in, int n, Cplx *out);

/* ── Carrier Synchronisation ─────────────────────────────────────── */

typedef struct {
    double freq;         /* current frequency estimate (normalised)  */
    double phase;        /* current phase estimate (radians)         */
    double loop_bw;
    double damping;
    double alpha, beta;  /* 2nd-order loop gains                     */
} CarrierSync;

/**
 * @brief Initialise carrier recovery (Costas loop for BPSK/QPSK).
 * @param cs      Struct to initialise
 * @param loop_bw Loop bandwidth (normalised, try 0.01)
 * @param damping Damping factor
 */
void carrier_init(CarrierSync *cs, double loop_bw, double damping);

/**
 * @brief Costas loop — remove carrier offset from BPSK signal.
 * @return Final frequency estimate
 */
double carrier_costas_bpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out);

/**
 * @brief Costas loop — QPSK variant (4th-power phase detector).
 */
double carrier_costas_qpsk(CarrierSync *cs, const Cplx *in, int n, Cplx *out);

/**
 * @brief PLL — generic phase-locked loop.
 * @param phase_det  Phase detector function pointer
 */
typedef double (*PhaseDetFunc)(Cplx sample, double phase_est);
double carrier_pll(CarrierSync *cs, const Cplx *in, int n,
                   PhaseDetFunc pdet, Cplx *out);

/* ── Frame Synchronisation ───────────────────────────────────────── */

/** 13-bit Barker code {+1,+1,+1,+1,+1,-1,-1,+1,+1,-1,+1,-1,+1}. */
extern const int BARKER_13[13];

/** 11-bit Barker code. */
extern const int BARKER_11[11];

/** 7-bit Barker code. */
extern const int BARKER_7[7];

/**
 * @brief Cross-correlate signal with known preamble to find frame start.
 * @param signal    Input signal (real or Re part)
 * @param sig_len   Signal length
 * @param preamble  Known preamble sequence (±1)
 * @param pre_len   Preamble length
 * @param corr      Output correlation values (sig_len - pre_len + 1)
 * @return Index of correlation peak (frame start)
 */
int frame_sync_correlate(const double *signal, int sig_len,
                         const int *preamble, int pre_len,
                         double *corr);

/**
 * @brief Detect preamble with threshold.
 * @param threshold  Detection threshold (normalised, 0.0–1.0)
 * @return Index of first detection, or -1 if not found.
 */
int frame_sync_detect(const double *signal, int sig_len,
                      const int *preamble, int pre_len,
                      double threshold);

/* ── Scrambler / Descrambler ─────────────────────────────────────── */

/**
 * @brief Additive scrambler using LFSR polynomial.
 * @param poly   Generator polynomial (e.g. 0x48 for x^7+x^4+1)
 * @param init   Initial LFSR state
 * @param bits   Input bits (modified in-place)
 * @param n      Number of bits
 */
void scrambler(uint16_t poly, uint16_t init, uint8_t *bits, int n);

#endif /* SYNC_H */
