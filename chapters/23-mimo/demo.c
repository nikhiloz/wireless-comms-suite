/**
 * @file demo.c
 * @brief Chapter 23 — MIMO & Spatial Diversity (Alamouti, MRC, ZF)
 *
 * Build:  make build/bin/23-mimo
 * Run:    ./build/bin/23-mimo
 */

#include <stdio.h>
#include <math.h>
#include "../../include/comms_utils.h"
#include "../../include/modulation.h"
#include "../../include/channel.h"
#include "../../include/phy.h"

#define N_PAIRS 500

int main(void)
{
    rng_seed(23);
    print_separator("Chapter 23: MIMO & Spatial Diversity");

    /* ── Alamouti STBC (2×1) ─────────────────────────────────── */
    printf("1. Alamouti Space-Time Block Code (2 TX, 1 RX)\n\n");
    printf("   Eb/N0(dB)  SISO BER     Alamouti BER  Diversity Gain\n");
    printf("   ─────────  ──────────   ────────────  ──────────────\n");

    for (double ebn0 = 0; ebn0 <= 20; ebn0 += 4) {
        int siso_errs = 0, stbc_errs = 0;
        int total_bits = 0;

        for (int p = 0; p < N_PAIRS; p++) {
            /* Generate BPSK pair */
            uint8_t bits[2];
            random_bits(bits, 2);
            Cplx s[2];
            mod_modulate(MOD_BPSK, bits, 2, s);

            /* SISO: Rayleigh channel */
            double snr_lin = pow(10, ebn0 / 10.0);
            Cplx h_siso = cplx(rng_gaussian() * 0.707, rng_gaussian() * 0.707);
            double noise_std = 1.0 / sqrt(2.0 * snr_lin);
            Cplx n_siso = cplx(rng_gaussian() * noise_std, rng_gaussian() * noise_std);
            Cplx r_siso = cplx_add(cplx_mul(h_siso, s[0]), n_siso);
            /* Equalise */
            Cplx s_hat = cplx_scale(cplx_mul(cplx_conj(h_siso), r_siso),
                                    1.0 / cplx_mag2(h_siso));
            if ((s_hat.re > 0) != (s[0].re > 0)) siso_errs++;

            /* Alamouti 2×1 */
            Cplx h0 = cplx(rng_gaussian() * 0.707, rng_gaussian() * 0.707);
            Cplx h1 = cplx(rng_gaussian() * 0.707, rng_gaussian() * 0.707);
            Cplx tx0[2], tx1[2];
            mimo_alamouti_encode(s, tx0, tx1);

            Cplx rx[2];
            for (int t = 0; t < 2; t++) {
                Cplx n = cplx(rng_gaussian() * noise_std, rng_gaussian() * noise_std);
                rx[t] = cplx_add(cplx_add(cplx_mul(h0, tx0[t]),
                                           cplx_mul(h1, tx1[t])), n);
            }
            Cplx s_alam[2];
            mimo_alamouti_decode(rx, h0, h1, s_alam);
            for (int b = 0; b < 2; b++) {
                uint8_t det = (s_alam[b].re > 0) ? 0 : 1;
                if (det != bits[b]) stbc_errs++;
            }
            total_bits += 2;
        }

        double ber_siso = (double)siso_errs / N_PAIRS;
        double ber_stbc = (double)stbc_errs / total_bits;
        double gain = (ber_stbc > 0 && ber_siso > 0) ?
            10 * log10(ber_siso / ber_stbc) : 0;
        printf("   %5.1f      %.4e     %.4e     %+.1f dB\n",
               ebn0, ber_siso, ber_stbc, gain);
    }

    /* ── MRC (1×N receive diversity) ─────────────────────────── */
    printf("\n2. MRC (Maximum Ratio Combining, 1 TX, varied RX)\n");
    printf("   N_RX  BER at 10 dB Eb/N0\n");
    printf("   ────  ──────────────────\n");

    double ebn0 = 10.0;
    double snr_lin = pow(10, ebn0 / 10.0);
    double noise_std = 1.0 / sqrt(2.0 * snr_lin);

    int nrx_vals[] = {1, 2, 4};
    for (int nr = 0; nr < 3; nr++) {
        int n_rx = nrx_vals[nr];
        int errs = 0;
        int N = 2000;
        for (int i = 0; i < N; i++) {
            uint8_t bit = rng_bernoulli(0.5) ? 1 : 0;
            Cplx s = bit ? cplx(-1,0) : cplx(1,0);

            Cplx h[4], rx_arr[4];
            for (int r = 0; r < n_rx; r++) {
                h[r] = cplx(rng_gaussian() * 0.707, rng_gaussian() * 0.707);
                Cplx n = cplx(rng_gaussian() * noise_std, rng_gaussian() * noise_std);
                rx_arr[r] = cplx_add(cplx_mul(h[r], s), n);
            }
            Cplx combined = mimo_mrc(rx_arr, h, n_rx);
            uint8_t det = (combined.re > 0) ? 0 : 1;
            if (det != bit) errs++;
        }
        printf("   %d     %.4e\n", n_rx, (double)errs / N);
    }

    print_separator("End of Chapter 23");
    return 0;
}
