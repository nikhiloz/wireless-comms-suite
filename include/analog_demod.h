/**
 * @file analog_demod.h
 * @brief Analog demodulation — FM discriminator, AM envelope, SSB Weaver.
 *
 * Provides:
 *   - FM instantaneous-frequency discriminator
 *   - FM pre-emphasis / de-emphasis filters
 *   - FM stereo pilot detection and L/R decode
 *   - AM envelope detection and coherent demod
 *   - SSB demodulation (Weaver method)
 *   - Simple low-pass FIR utility
 */

#ifndef ANALOG_DEMOD_H
#define ANALOG_DEMOD_H

#include "comms_utils.h"

/* ── FM Modulation / Demodulation ────────────────────────────────── */

/**
 * @brief FM modulate: real audio → complex baseband IQ.
 * @param audio    Input audio samples (normalised ±1)
 * @param n        Number of input samples
 * @param freq_dev Frequency deviation (normalised: delta_f / fs)
 * @param out      Output complex IQ (n samples)
 * @return Number of output samples
 */
int fm_modulate(const double *audio, int n, double freq_dev, Cplx *out);

/**
 * @brief FM demodulate via differentiate-atan2 discriminator.
 * @param iq       Input complex baseband IQ
 * @param n        Number of IQ samples
 * @param out      Output instantaneous frequency (n-1 samples)
 * @return Number of output samples (n - 1)
 */
int fm_demodulate(const Cplx *iq, int n, double *out);

/* ── FM Pre-emphasis / De-emphasis ───────────────────────────────── */

/**
 * @brief FM de-emphasis filter (single-pole IIR).
 *
 * Attenuates high-frequency noise per broadcast standard.
 * H(z) = (1-a) / (1 - a*z^{-1}),  a = exp(-1/(tau*fs))
 *
 * @param in       Input signal
 * @param n        Number of samples
 * @param tau_us   Time constant in microseconds (75 USA, 50 Europe)
 * @param fs       Sample rate in Hz
 * @param out      Output de-emphasised signal (may alias in)
 */
void fm_deemphasis(const double *in, int n, double tau_us, double fs,
                   double *out);

/**
 * @brief FM pre-emphasis filter (inverse of de-emphasis).
 * H(z) = (1 - a*z^{-1}) / (1-a)
 */
void fm_preemphasis(const double *in, int n, double tau_us, double fs,
                    double *out);

/* ── FM Stereo ───────────────────────────────────────────────────── */

/**
 * @brief Detect 19 kHz stereo pilot tone via Goertzel algorithm.
 * @param in       Composite FM baseband
 * @param n        Number of samples
 * @param fs       Sample rate
 * @return Pilot strength (0..1); > 0.1 indicates stereo
 */
double fm_stereo_pilot_detect(const double *in, int n, double fs);

/**
 * @brief Decode FM stereo: composite → left and right channels.
 *
 * Composite = (L+R) + pilot_19kHz + (L-R)*cos(2*pilot)
 *
 * @param composite  Input composite baseband signal
 * @param n          Number of samples
 * @param fs         Sample rate
 * @param left       Output left channel (n samples)
 * @param right      Output right channel (n samples)
 * @return 0 on success, -1 if no pilot detected
 */
int fm_stereo_decode(const double *composite, int n, double fs,
                     double *left, double *right);

/* ── AM Modulation / Demodulation ────────────────────────────────── */

/**
 * @brief AM modulate (DSB-LC): audio → complex baseband.
 * @param audio    Input audio (normalised ±1)
 * @param n        Number of samples
 * @param mod_idx  Modulation index m (0..1 for no clipping)
 * @param fc_norm  Carrier frequency normalised to sample rate (fc/fs)
 * @param out      Output complex signal
 * @return n
 */
int am_modulate(const double *audio, int n, double mod_idx,
                double fc_norm, Cplx *out);

/**
 * @brief AM envelope detection (non-coherent).
 * @param iq    Input complex baseband
 * @param n     Number of samples
 * @param out   Output envelope (n samples)
 * @return n
 */
int am_envelope_detect(const Cplx *iq, int n, double *out);

/**
 * @brief AM coherent (synchronous) demodulation.
 * @param iq       Input complex baseband
 * @param n        Number of samples
 * @param fc_norm  Carrier frequency (fc/fs)
 * @param out      Output demodulated audio
 * @return n
 */
int am_coherent_demod(const Cplx *iq, int n, double fc_norm, double *out);

/* ── SSB Modulation / Demodulation ───────────────────────────────── */

/**
 * @brief SSB modulate using Hilbert-transform method.
 * @param audio     Input audio
 * @param n         Number of samples
 * @param upper     1 for USB, 0 for LSB
 * @param fc_norm   Centre frequency offset (fc/fs)
 * @param out       Output complex signal
 * @return n
 */
int ssb_modulate(const double *audio, int n, int upper,
                 double fc_norm, Cplx *out);

/**
 * @brief SSB demodulate (product detector).
 * @param iq       Input complex signal
 * @param n        Number of samples
 * @param fc_norm  Carrier frequency (fc/fs)
 * @param out      Output audio
 * @return n
 */
int ssb_demodulate(const Cplx *iq, int n, double fc_norm, double *out);

/* ── Utility ─────────────────────────────────────────────────────── */

/**
 * @brief Simple low-pass FIR filter (windowed sinc).
 * @param in     Input signal
 * @param n      Number of input samples
 * @param fc     Normalised cutoff frequency (0..0.5)
 * @param taps   Filter order (odd recommended)
 * @param out    Output filtered signal (n samples, delay-compensated)
 */
void lowpass_fir(const double *in, int n, double fc, int taps, double *out);

#endif /* ANALOG_DEMOD_H */