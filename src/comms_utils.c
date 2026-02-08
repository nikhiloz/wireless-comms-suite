/**
 * @file comms_utils.c
 * @brief Core utilities — complex arithmetic, PRNG, bit helpers, ASCII plots.
 *
 * TUTORIAL CROSS-REFERENCES:
 *   Complex numbers      → chapters/01-system-overview/tutorial.md
 *   Bit manipulation     → chapters/03-channel-coding/tutorial.md
 *   Noise generation     → chapters/06-awgn-channel/tutorial.md
 *
 * References:
 *   Box-Muller transform for Gaussian noise generation.
 *   Xoshiro256** PRNG (Blackman & Vigna, 2018).
 */
#define _POSIX_C_SOURCE 199309L   /* clock_gettime */

#include "../include/comms_utils.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ════════════════════════════════════════════════════════════════════
 *  Complex arithmetic
 * ════════════════════════════════════════════════════════════════════ */

Cplx cplx(double re, double im)
{
    Cplx z = { re, im };
    return z;
}

Cplx cplx_add(Cplx a, Cplx b)
{
    return cplx(a.re + b.re, a.im + b.im);
}

Cplx cplx_sub(Cplx a, Cplx b)
{
    return cplx(a.re - b.re, a.im - b.im);
}

Cplx cplx_mul(Cplx a, Cplx b)
{
    return cplx(a.re * b.re - a.im * b.im,
                a.re * b.im + a.im * b.re);
}

Cplx cplx_conj(Cplx z)
{
    return cplx(z.re, -z.im);
}

Cplx cplx_scale(Cplx z, double s)
{
    return cplx(z.re * s, z.im * s);
}

double cplx_mag(Cplx z)
{
    return sqrt(z.re * z.re + z.im * z.im);
}

double cplx_mag2(Cplx z)
{
    return z.re * z.re + z.im * z.im;
}

double cplx_phase(Cplx z)
{
    return atan2(z.im, z.re);
}

Cplx cplx_from_polar(double mag, double phase)
{
    return cplx(mag * cos(phase), mag * sin(phase));
}

Cplx cplx_exp_j(double theta)
{
    return cplx(cos(theta), sin(theta));
}

/* ════════════════════════════════════════════════════════════════════
 *  PRNG — Xoshiro256** (fast, high quality)
 * ════════════════════════════════════════════════════════════════════ */

static uint64_t rng_state[4] = { 1, 2, 3, 4 };

static uint64_t rotl(uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

void rng_seed(uint64_t seed)
{
    /* SplitMix64 to initialise state from a single seed */
    for (int i = 0; i < 4; i++) {
        seed += 0x9e3779b97f4a7c15ULL;
        uint64_t z = seed;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        rng_state[i] = z ^ (z >> 31);
    }
}

static uint64_t rng_next(void)
{
    uint64_t result = rotl(rng_state[1] * 5, 7) * 9;
    uint64_t t = rng_state[1] << 17;

    rng_state[2] ^= rng_state[0];
    rng_state[3] ^= rng_state[1];
    rng_state[1] ^= rng_state[2];
    rng_state[0] ^= rng_state[3];
    rng_state[2] ^= t;
    rng_state[3] = rotl(rng_state[3], 45);

    return result;
}

double rng_uniform(void)
{
    return (rng_next() >> 11) * (1.0 / (1ULL << 53));
}

double rng_gaussian(void)
{
    /* Box-Muller transform */
    double u1 = rng_uniform();
    double u2 = rng_uniform();
    while (u1 < 1e-15) u1 = rng_uniform(); /* avoid log(0) */
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

int rng_bernoulli(double p)
{
    return rng_uniform() < p ? 1 : 0;
}

/* ════════════════════════════════════════════════════════════════════
 *  Bit manipulation
 * ════════════════════════════════════════════════════════════════════ */

void bits_from_bytes(const uint8_t *bytes, int nbytes, uint8_t *bits)
{
    for (int i = 0; i < nbytes; i++) {
        for (int b = 7; b >= 0; b--) {
            *bits++ = (bytes[i] >> b) & 1;
        }
    }
}

void bytes_from_bits(const uint8_t *bits, int nbits, uint8_t *bytes)
{
    int nbytes = (nbits + 7) / 8;
    memset(bytes, 0, nbytes);
    for (int i = 0; i < nbits; i++) {
        bytes[i / 8] |= (bits[i] & 1) << (7 - (i % 8));
    }
}

void random_bits(uint8_t *bits, int n)
{
    for (int i = 0; i < n; i++) {
        bits[i] = rng_bernoulli(0.5);
    }
}

int bit_errors(const uint8_t *a, const uint8_t *b, int n)
{
    int count = 0;
    for (int i = 0; i < n; i++) {
        if ((a[i] & 1) != (b[i] & 1)) count++;
    }
    return count;
}

void print_bits(const uint8_t *bits, int n, const char *label)
{
    printf("%s: ", label);
    for (int i = 0; i < n; i++) {
        printf("%d", bits[i] & 1);
        if ((i + 1) % 8 == 0 && i + 1 < n) printf(" ");
    }
    printf("\n");
}

/* ════════════════════════════════════════════════════════════════════
 *  ASCII helpers
 * ════════════════════════════════════════════════════════════════════ */

void print_signal_ascii(const char *title, const double *x, int n, int max_show)
{
    if (max_show <= 0 || max_show > n) max_show = n;
    printf("\n%s (showing %d/%d samples):\n", title, max_show, n);

    double vmin = x[0], vmax = x[0];
    for (int i = 1; i < max_show; i++) {
        if (x[i] < vmin) vmin = x[i];
        if (x[i] > vmax) vmax = x[i];
    }
    double range = vmax - vmin;
    if (range < 1e-12) range = 1.0;

    int bar_width = 50;
    for (int i = 0; i < max_show; i++) {
        int len = (int)((x[i] - vmin) / range * bar_width);
        if (len < 0) len = 0;
        if (len > bar_width) len = bar_width;
        printf("  [%4d] %+8.4f |", i, x[i]);
        for (int j = 0; j < len; j++) printf("█");
        printf("\n");
    }
}

void print_constellation_ascii(const Cplx *syms, int n, int grid_size)
{
    if (grid_size <= 0) grid_size = 21;
    char grid[41][41];
    memset(grid, ' ', sizeof(grid));

    /* Find range */
    double rmax = 0;
    for (int i = 0; i < n; i++) {
        double m = cplx_mag(syms[i]);
        if (m > rmax) rmax = m;
    }
    if (rmax < 1e-12) rmax = 1.0;
    rmax *= 1.2;

    int gs = grid_size < 41 ? grid_size : 40;
    int half = gs / 2;

    /* Plot axes */
    for (int r = 0; r < gs; r++) grid[r][half] = '|';
    for (int c = 0; c < gs; c++) grid[half][c] = '-';
    grid[half][half] = '+';

    /* Plot points */
    for (int i = 0; i < n; i++) {
        int col = half + (int)(syms[i].re / rmax * half);
        int row = half - (int)(syms[i].im / rmax * half);
        if (row >= 0 && row < gs && col >= 0 && col < gs)
            grid[row][col] = '*';
    }

    printf("\nConstellation (%d symbols):\n", n);
    for (int r = 0; r < gs; r++) {
        printf("  ");
        for (int c = 0; c < gs; c++) printf("%c", grid[r][c]);
        printf("\n");
    }
}

void print_eye_diagram_ascii(const double *x, int n, int sps, int eyes)
{
    if (eyes <= 0) eyes = 3;
    int period = sps * eyes;
    int n_traces = n / period;

    printf("\nEye diagram (%d traces, %d eyes):\n", n_traces, eyes);

    int rows = 15, cols = period < 60 ? period : 60;
    char grid[15][60];
    memset(grid, ' ', sizeof(grid));

    double vmin = x[0], vmax = x[0];
    for (int i = 0; i < n; i++) {
        if (x[i] < vmin) vmin = x[i];
        if (x[i] > vmax) vmax = x[i];
    }
    double range = vmax - vmin;
    if (range < 1e-12) range = 1.0;

    for (int t = 0; t < n_traces; t++) {
        for (int c = 0; c < cols && c < period; c++) {
            int idx = t * period + c;
            if (idx >= n) break;
            int row = (int)((1.0 - (x[idx] - vmin) / range) * (rows - 1));
            if (row < 0) row = 0;
            if (row >= rows) row = rows - 1;
            if (c < cols) grid[row][c] = '.';
        }
    }

    for (int r = 0; r < rows; r++) {
        printf("  ");
        for (int c = 0; c < cols; c++) printf("%c", grid[r][c]);
        printf("\n");
    }
}

void print_bar_chart(const char *title, const double *vals,
                     const char **labels, int n)
{
    printf("\n%s:\n", title);
    double vmax = 0;
    for (int i = 0; i < n; i++)
        if (fabs(vals[i]) > vmax) vmax = fabs(vals[i]);
    if (vmax < 1e-12) vmax = 1.0;

    int bar_width = 40;
    for (int i = 0; i < n; i++) {
        int len = (int)(fabs(vals[i]) / vmax * bar_width);
        printf("  %-12s %8.4f |", labels ? labels[i] : "", vals[i]);
        for (int j = 0; j < len; j++) printf("█");
        printf("\n");
    }
}

void print_separator(const char *title)
{
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    if (title) printf("  %s\n", title);
    printf("════════════════════════════════════════════════════════════\n");
}

/* ════════════════════════════════════════════════════════════════════
 *  Math helpers
 * ════════════════════════════════════════════════════════════════════ */

double db_to_linear(double db)
{
    return pow(10.0, db / 10.0);
}

double linear_to_db(double lin)
{
    return 10.0 * log10(lin > 1e-30 ? lin : 1e-30);
}

double sinc(double x)
{
    if (fabs(x) < 1e-12) return 1.0;
    return sin(M_PI * x) / (M_PI * x);
}

double clamp(double x, double lo, double hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

int next_pow2(int n)
{
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ════════════════════════════════════════════════════════════════════
 *  Timing
 * ════════════════════════════════════════════════════════════════════ */

double get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}
