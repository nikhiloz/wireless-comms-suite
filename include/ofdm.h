/**
 * @file ofdm.h
 * @brief OFDM system primitives — IFFT/FFT TX/RX, cyclic prefix, pilots.
 *
 * Implements a complete OFDM modulator/demodulator chain:
 *   TX: symbol mapping → subcarrier assignment → IFFT → CP insertion
 *   RX: CP removal → FFT → pilot-based equalisation → demapping
 *
 * Also provides a simple FFT/IFFT (radix-2 DIT) used internally.
 */

#ifndef OFDM_H
#define OFDM_H

#include "comms_utils.h"
#include <stdint.h>

/* ── FFT (radix-2, in-place) ─────────────────────────────────────── */

void fft(Cplx *x, int n);     /* Forward FFT   */
void ifft(Cplx *x, int n);    /* Inverse FFT   */

/* ── OFDM parameters ────────────────────────────────────────────── */

#define OFDM_MAX_CARRIERS 1024
#define OFDM_MAX_PILOTS    64

typedef struct {
    int  n_fft;              /* FFT size (must be power of 2)        */
    int  n_cp;               /* cyclic prefix length (samples)       */
    int  n_data;             /* number of data subcarriers           */
    int  n_pilot;            /* number of pilot subcarriers          */
    int  data_idx[OFDM_MAX_CARRIERS];   /* data subcarrier indices  */
    int  pilot_idx[OFDM_MAX_PILOTS];    /* pilot subcarrier indices */
    Cplx pilot_val;          /* pilot symbol value (e.g. 1+0j)      */
    int  n_guard_lo;         /* lower guard subcarriers              */
    int  n_guard_hi;         /* upper guard subcarriers              */
} OfdmParams;

/**
 * @brief Initialise OFDM parameters with sensible defaults.
 * @param p        Params struct to fill
 * @param n_fft    FFT size
 * @param n_cp     Cyclic prefix length
 * @param n_pilot  Number of pilots (evenly spaced)
 */
void ofdm_init(OfdmParams *p, int n_fft, int n_cp, int n_pilot);

/* ── OFDM TX ─────────────────────────────────────────────────────── */

/**
 * @brief Map data symbols onto subcarriers and generate one OFDM symbol.
 * @param p         OFDM parameters
 * @param data_syms Input data symbols (p->n_data entries)
 * @param out       Output time-domain samples (n_fft + n_cp)
 * @return Number of output samples
 */
int ofdm_modulate(const OfdmParams *p, const Cplx *data_syms, Cplx *out);

/**
 * @brief Modulate multiple OFDM symbols.
 * @param n_symbols Number of OFDM symbols
 * @param data_syms All data (n_symbols * p->n_data)
 * @param out       Output (n_symbols * (n_fft + n_cp))
 * @return Total output samples
 */
int ofdm_modulate_block(const OfdmParams *p, int n_symbols,
                        const Cplx *data_syms, Cplx *out);

/* ── OFDM RX ─────────────────────────────────────────────────────── */

/**
 * @brief Demodulate one OFDM symbol (CP removal → FFT → pilot EQ).
 * @param p          OFDM parameters
 * @param in         Input time-domain samples (n_fft + n_cp)
 * @param data_syms  Output data symbols (p->n_data)
 * @param h_est      Output channel estimates at data positions (or NULL)
 * @return Number of data symbols extracted
 */
int ofdm_demodulate(const OfdmParams *p, const Cplx *in,
                    Cplx *data_syms, Cplx *h_est);

int ofdm_demodulate_block(const OfdmParams *p, int n_symbols,
                          const Cplx *in, Cplx *data_syms);

/* ── Channel estimation (via pilots) ─────────────────────────────── */

/**
 * @brief Estimate channel at pilot positions, interpolate to data.
 * @param p            OFDM parameters
 * @param rx_freq      Received frequency-domain symbols (n_fft)
 * @param h_interp     Output: interpolated channel at data indices
 */
void ofdm_channel_estimate(const OfdmParams *p, const Cplx *rx_freq,
                           Cplx *h_interp);

/**
 * @brief Zero-forcing single-tap equalisation.
 */
void ofdm_equalise_zf(const Cplx *data, const Cplx *h, int n, Cplx *out);

#endif /* OFDM_H */
