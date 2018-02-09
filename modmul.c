/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */
#include "modmul.h"

/* Performs rop <- x^y (mod N)
 * Using the sliding widow method for exponentiation. 
 * Window size is defined with k.
 */
void sliding_exponentiation(mpz_t rop, 
                            mpz_t x, 
                            mpz_t y, 
                            mpz_t N, 
                            unsigned char k)
{
	
	if (k < 1 || k > 64) abort();

	// Variable Initialisation
	mpz_t x2, temp, _rop;
	mpz_init_set_ui(_rop, 1);
	mpz_init(x2);
	mpz_init(temp);
	
	// Precompute values of: 
	// x^1, x^3, x^5, ... , x^(2*k - 1) under modulo N
	mpz_t *T = malloc((1 << (k-1)) * sizeof(mpz_t));
	mpz_init_set(T[0], x);
	mpz_powm_ui(x2, x, 2, N); // Pre-computed x^2 value
	for (int i = 1; i < (1 << (k-1)); i++) {
		mpz_init(T[i]);
		mpz_mul(T[i], T[i-1], x2);
		mpz_mod(T[i], T[i], N);
	}
	
	int i = mpz_sizeinbase(y, 2) - 1; // Counter/Position
	int l = 0;                        // Least sig. bit pos. of the window
	unsigned long u = 0;              // Binary representation of window
	
	// Loop until we hit the end of the number
	while (i >= 0) {
		// Check if bit of y (exponent) is 0
		if (mpz_tstbit(y, i) == 0) {
			l = i;
			u = 0;
		} else {
			// Find least significant bit of window
			l = MAX(i-k+1, 0);
			l = mpz_scan1(y, l);
			 
			// Extract window into separate variable
			u = 0;
			for (int j=i; j>=l; j--) u = (u << 1) + mpz_tstbit(y,j);
		}

		// Bit shift answer left by window size
		mpz_set_ui(temp, 2);
		mpz_powm_ui(temp, temp, i-l+1, N);
		mpz_powm(_rop, _rop, temp, N);

		// If the window was not empty, multiply by the relevant 
		// precomputed value to the answer for that window.
		if (u != 0) {
			mpz_mul(_rop, _rop, T[((u-1)/2)]);
			mpz_mod(_rop, _rop, N);
		}

		// Continue next iteration of loop to bits less significant than
		// that of the window we just computed.
		i = l - 1;
	}
	
	// Finally set the answer
	mpz_set(rop, _rop);

	// Cleanup
	mpz_clear(x2);
	mpz_clear(temp);
	for (int i = 0; i < (1 << (k-1)); i++) mpz_clear(T[i]);
}

/* Perform stage 1:
 * 
 * - read each 3-tuple of N, e and m from stdin,
 * - compute the RSA encryption c, then
 * - write the ciphertext c to stdout.
 */
void stage1() 
{
	// Initialisation
	mpz_t N, e, m;
	mpz_init(N);
	mpz_init(e);
	mpz_init(m);
	
	mpz_t r;
	mpz_init(r);

	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", N) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", e) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Vanilla RSA encryption
		sliding_exponentiation(r, m, e, N, 5);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", r);
	}

	// Done - cleanup
	mpz_clear(N);
	mpz_clear(e);
	mpz_clear(m);
	
	mpz_clear(r);
}

/* Perform stage 2:
 * 
 * - read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
 * - compute the RSA decryption m, then
 * - write the plaintext m to stdout.
 */
void stage2() 
{
	// Initialisation
	mpz_t m1, m2, a1, a2;
	mpz_init(m1);
	mpz_init(m2);
	mpz_init(a1);
	mpz_init(a2);
	
	mpz_t N, d, p, q, d_p, d_q, i_p, i_q, c;
	mpz_init(N);
	mpz_init(d);
	mpz_init(p);
	mpz_init(q);
	mpz_init(d_p);
	mpz_init(d_q);
	mpz_init(i_p);
	mpz_init(i_q);
	mpz_init(c);
	
	mpz_t m;
	mpz_init(m);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", N) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", d  ) != 1) abort();
		if (gmp_scanf("%ZX", p  ) != 1) abort();
		if (gmp_scanf("%ZX", q  ) != 1) abort();
		if (gmp_scanf("%ZX", d_p) != 1) abort();
		if (gmp_scanf("%ZX", d_q) != 1) abort();
		if (gmp_scanf("%ZX", i_p) != 1) abort();
		if (gmp_scanf("%ZX", i_q) != 1) abort();
		if (gmp_scanf("%ZX", c  ) != 1) abort();

		// Naive vanilla RSA decryption
		//sliding_exponentiation(m, c, d, N, 5);

		// RSA decryption using CRT
		sliding_exponentiation(m1, c, d_p, p, 5);
		sliding_exponentiation(m2, c, d_q, q, 5);
		mpz_sub(a1, m1, m2);
		mpz_mul(a2, i_q, a1);
		mpz_mod(a1, a2, p); // Reusing a1
		mpz_mul(a2, a1, q); // Reusing a2
		mpz_add(m, m2, a2);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", m);
	}

	// Done - cleanup
	mpz_clear(m1);
	mpz_clear(m2);
	mpz_clear(a1);
	mpz_clear(a2);

	mpz_clear(N);
	mpz_clear(d);
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(d_p);
	mpz_clear(d_q);
	mpz_clear(i_p);
	mpz_clear(i_q);
	mpz_clear(c);

	mpz_clear(m);
}

/* Perform stage 3:
 * 
 * - read each 5-tuple of p, q, g, h and m from stdin,
 * - compute the ElGamal encryption c = (c_1,c_2), then
 * - write the ciphertext c to stdout.
 */
void stage3() 
{
	// Initialisation
	mpz_t c1, c2, a1, a2;
	mpz_init(c1);
	mpz_init(c2);
	mpz_init(a1);
	mpz_init(a2);
	
	mpz_t p, q, g, h, m;
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(h);
	mpz_init(m);
	
	mpz_t r;
	mpz_init(r);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", p) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", q) != 1) abort();
		if (gmp_scanf("%ZX", g) != 1) abort();
		if (gmp_scanf("%ZX", h) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Set R, NOTE:
		// Fixed as 1 for testing, normally random number as below
		mpz_set_ui(r, 1);
		/* gmp_randstate_t state; */
		/* gmp_randinit_default(state); */
		/* mpz_urandomm(r, state, q); */

		// Vanilla ElGamal Encryption
		mpz_mod(a1, r, q);
		sliding_exponentiation(c1, g, a1, p, 5);

		sliding_exponentiation(a2, h, a1, p, 5);
		mpz_mul(a1, a2, m); // Reusing a1
		mpz_mod(c2, a1, p);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n%ZX\n", c1, c2);
	}

	// Done - cleanup
	mpz_clear(c1);
	mpz_clear(c2);
	
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(g);
	mpz_clear(h);
	mpz_clear(m);
	
	mpz_clear(r);
}

/* Perform stage 4:
 * 
 * - read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
 * - compute the ElGamal decryption m, then
 * - write the plaintext m to stdout.
 */
void stage4() 
{
	// Initialisation
	mpz_t a1, a2;
	mpz_init(a1);
	mpz_init(a2);
	
	mpz_t p, q, g, x, c1, c2;
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(x);
	mpz_init(c1);
	mpz_init(c2);
	
	mpz_t m;
	mpz_init(m);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", p) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", q ) != 1) abort();
		if (gmp_scanf("%ZX", g ) != 1) abort();
		if (gmp_scanf("%ZX", x ) != 1) abort();
		if (gmp_scanf("%ZX", c1) != 1) abort();
		if (gmp_scanf("%ZX", c2) != 1) abort();

		mpz_neg(a1, x);
		mpz_mod(a2, a1, q);
		sliding_exponentiation(a1, c1, a2, p, 5); // Reusing a1
		mpz_mul(a2, a1, c2);
		mpz_mod(m, a2, p);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", m);
	}

	// Done - cleanup
	mpz_clear(a1);
	mpz_clear(a2);
	
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(g);
	mpz_clear(x);
	mpz_clear(c1);
	mpz_clear(c2);
	
	mpz_clear(m);
}

/* The main function acts as a driver for the assignment by simply invoking the
 * correct function for the requested stage.
 */
int main(int argc, char* argv[])
{
	if (argc != 2) {
		
		mpz_t rop, m, e, N;
		mpz_init(rop);
		mpz_init_set_ui(m, 4);
		mpz_init_set_ui(e, 4);
		mpz_init_set_ui(N, 10000);

		sliding_exponentiation(rop, m, e, N, 5);
		/* gmp_printf("ANSWER: %Zd\n", rop); */

		return 1;
	}

	if (!strcmp(argv[1], "stage1")) {
		stage1();
	} else if (!strcmp(argv[1], "stage2")) {
		stage2();
	} else if (!strcmp(argv[1], "stage3")) {
		stage3();
	} else if (!strcmp(argv[1], "stage4")) {
		stage4();
	} else {
		return 1;
	}
	
	return 0;
}
