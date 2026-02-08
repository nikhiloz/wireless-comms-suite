/**
 * @file comms_utils.h
 * @brief Core utilities for the wireless-comms-suite.
 *
 * Complex arithmetic, PRNG, signal helpers, ASCII plotting.
 * Every other module depends on this header.
 */

#ifndef COMMS_UTILS_H
#define COMMS_UTILS_H

#include <stddef.h>
#include <stdint.h>

/* ── Complex number ──────────────────────────────────────────────── */

typedef struct {
    double re;
    double im;
} Cplx;

Cplx   cplx(double re, double im);
Cplx   cplx_add(Cplx a, Cplx b);
Cplx   cplx_sub(Cplx a, Cplx b);
Cplx   cplx_mul(Cplx a, Cplx b);
Cplx   cplx_conj(Cplx z);
Cplx   cplx_scale(Cplx z, double s);
double cplx_mag(Cplx z);
double cplx_mag2(Cplx z);          /* |z|^2 — avoids sqrt */
double cplx_phase(Cplx z);
Cplx   cplx_from_polar(double mag, double phase);
Cplx   cplx_exp_j(double theta);   /* e^{j*theta} */

/* ── PRNG / noise ────────────────────────────────────────────────── */

void   rng_seed(uint64_t seed);
double rng_uniform(void);          /* [0, 1)  */
double rng_gaussian(void);         /* N(0,1)  */
int    rng_bernoulli(double p);    /* 1 with probability p */

/* ── Bit manipulation ────────────────────────────────────────────── */

void   bits_from_bytes(const uint8_t *bytes, int nbytes, uint8_t *bits);
void   bytes_from_bits(const uint8_t *bits, int nbits, uint8_t *bytes);
void   random_bits(uint8_t *bits, int n);
int    bit_errors(const uint8_t *a, const uint8_t *b, int n);
void   print_bits(const uint8_t *bits, int n, const char *label);

/* ── ASCII helpers ───────────────────────────────────────────────── */

void   print_signal_ascii(const char *title, const double *x, int n, int max_show);
void   print_constellation_ascii(const Cplx *syms, int n, int grid_size);
void   print_eye_diagram_ascii(const double *x, int n, int sps, int eyes);
void   print_bar_chart(const char *title, const double *vals,
                       const char **labels, int n);
void   print_separator(const char *title);

/* ── Math helpers ────────────────────────────────────────────────── */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double db_to_linear(double db);
double linear_to_db(double lin);
double sinc(double x);
double clamp(double x, double lo, double hi);
int    next_pow2(int n);

/* ── Timing ──────────────────────────────────────────────────────── */

double get_time_ms(void);          /* wall-clock milliseconds */

#endif /* COMMS_UTILS_H */
