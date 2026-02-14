/**
 * @file demo.c
 * @brief Chapter 25 — FM Broadcast Receiver
 *
 * Demonstrates FM modulation/demodulation, de-emphasis filtering,
 * stereo pilot detection, and a complete mono FM receiver pipeline.
 * Also shows AM envelope detection as a comparison.
 *
 * Build:  make build/bin/25-fm-broadcast
 * Run:    ./build/bin/25-fm-broadcast
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/analog_demod.h"
#include "../../include/channel.h"

#define FS          240000   /* sample rate 240 kHz (typical FM IF)     */
#define AUDIO_LEN   2400     /* 10 ms of audio at 240 kHz               */
#define FM_DEV      0.3125   /* +/-75 kHz / 240 kHz = normalised dev    */

int main(void)
{
    rng_seed(25);
    print_separator("Chapter 25: FM Broadcast Receiver");

    /* ── 1. Generate test audio (1 kHz tone) ─────────────────── */
    printf("1. Generate Test Audio (1 kHz tone, %d samples @ %d Hz)\n",
           AUDIO_LEN, FS);
    double *audio = malloc(AUDIO_LEN * sizeof(double));
    for (int i = 0; i < AUDIO_LEN; i++) {
        audio[i] = 0.8 * sin(2.0 * M_PI * 1000.0 / FS * i);
    }
    printf("   Audio peak: +/-0.80\n\n");

    /* ── 2. FM modulate ──────────────────────────────────────── */
    printf("2. FM Modulate (delta_f = +/-75 kHz, normalised dev = %.4f)\n",
           FM_DEV);
    Cplx *iq = malloc(AUDIO_LEN * sizeof(Cplx));
    int n_mod = fm_modulate(audio, AUDIO_LEN, FM_DEV, iq);
    printf("   Generated %d IQ samples\n", n_mod);
    printf("   Signal power: %.4f (unit circle)\n\n",
           signal_power(iq, n_mod));

    /* ── 3. Add channel noise ────────────────────────────────── */
    printf("3. AWGN Channel (SNR = 20 dB)\n");
    Cplx *noisy = malloc(AUDIO_LEN * sizeof(Cplx));
    double sigma2 = channel_awgn(iq, n_mod, 20.0, noisy);
    printf("   Noise variance: %.6f\n\n", sigma2);

    /* ── 4. FM demodulate ────────────────────────────────────── */
    printf("4. FM Demodulate (differentiate-atan2 discriminator)\n");
    int demod_len = AUDIO_LEN - 1;
    double *demod = malloc(demod_len * sizeof(double));
    int n_demod = fm_demodulate(noisy, n_mod, demod);
    printf("   Recovered %d samples\n", n_demod);
    print_signal_ascii("Demodulated FM (first 80 samples)", demod,
                       (n_demod > 80) ? 80 : n_demod, 80);

    /* ── 5. De-emphasis filter ───────────────────────────────── */
    printf("\n5. De-emphasis Filter (tau = 75 us, USA standard)\n");
    double *de_emph = malloc(demod_len * sizeof(double));
    fm_deemphasis(demod, n_demod, 75.0, (double)FS, de_emph);
    printf("   Applied IIR de-emphasis to %d samples\n", n_demod);
    print_signal_ascii("De-emphasised output (first 80)", de_emph,
                       (n_demod > 80) ? 80 : n_demod, 80);

    /* ── 6. Pre/De-emphasis round-trip ───────────────────────── */
    printf("\n6. Pre-emphasis / De-emphasis Round-Trip\n");
    double *pre = malloc(AUDIO_LEN * sizeof(double));
    double *roundtrip = malloc(AUDIO_LEN * sizeof(double));
    fm_preemphasis(audio, AUDIO_LEN, 75.0, (double)FS, pre);
    fm_deemphasis(pre, AUDIO_LEN, 75.0, (double)FS, roundtrip);
    double max_err = 0.0;
    for (int i = 10; i < AUDIO_LEN; i++) {   /* skip transient */
        double e = fabs(roundtrip[i] - audio[i]);
        if (e > max_err) max_err = e;
    }
    printf("   Max round-trip error (after transient): %.2e\n", max_err);
    printf("   %s\n\n", max_err < 0.01
           ? "PASS -- filters are inverses"
           : "Note: transient settling");

    /* ── 7. AM envelope detection comparison ─────────────────── */
    printf("7. AM Modulation & Envelope Detection\n");
    Cplx *am_sig = malloc(AUDIO_LEN * sizeof(Cplx));
    double *am_out = malloc(AUDIO_LEN * sizeof(double));
    am_modulate(audio, AUDIO_LEN, 0.8, 0.1, am_sig);
    am_envelope_detect(am_sig, AUDIO_LEN, am_out);

    double corr = 0.0, pwr_a = 0.0, pwr_b = 0.0;
    for (int i = 0; i < AUDIO_LEN; i++) {
        corr  += audio[i] * am_out[i];
        pwr_a += audio[i] * audio[i];
        pwr_b += am_out[i] * am_out[i];
    }
    double rho = (pwr_a > 0 && pwr_b > 0)
                 ? corr / sqrt(pwr_a * pwr_b) : 0.0;
    printf("   AM modulation index: 0.8\n");
    printf("   Envelope-original correlation: %.4f\n\n", rho);

    /* ── 8. Complete mono FM receiver pipeline ───────────────── */
    printf("8. Full Mono FM Receiver Pipeline\n");
    printf("   audio -> FM mod -> AWGN(20dB) -> discriminator -> de-emphasis\n");
    double *pipeline_out = malloc(demod_len * sizeof(double));
    fm_deemphasis(demod, n_demod, 75.0, (double)FS, pipeline_out);

    /* Measure SNR of recovered audio */
    double sig_pwr = 0.0, noise_pwr = 0.0;
    for (int i = 50; i < n_demod && i < AUDIO_LEN; i++) {
        double expected = audio[i] * (2.0 * FM_DEV);
        double err = pipeline_out[i] - expected;
        sig_pwr   += expected * expected;
        noise_pwr += err * err;
    }
    double snr_out = (noise_pwr > 0) ? 10.0 * log10(sig_pwr / noise_pwr) : 99.0;
    printf("   Output SNR estimate: %.1f dB\n", snr_out);
    printf("   FM capture effect provides SNR improvement above threshold\n");

    /* ── Cleanup ─────────────────────────────────────────────── */
    free(audio);
    free(iq);
    free(noisy);
    free(demod);
    free(de_emph);
    free(pre);
    free(roundtrip);
    free(am_sig);
    free(am_out);
    free(pipeline_out);

    print_separator("End of Chapter 25");
    return 0;
}