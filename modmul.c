/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */
#include "modmul.h"


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
		mpz_powm(r, m, e, N);

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
	
	mpz_t r;
	mpz_init(r);
	
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
		mpz_powm(r, c, d, N);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", r);
	}

	// Done - cleanup
	mpz_clear(N);
	mpz_clear(d);
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(d_p);
	mpz_clear(d_q);
	mpz_clear(i_p);
	mpz_clear(i_q);
	mpz_clear(c);

	mpz_clear(r);
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
		mpz_powm(c1, g, a1, p);

		mpz_powm(a2, h, a1, p);
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
		mpz_powm(a1, c1, a2, p); // Reusing a1
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
		abort();
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
		abort();
	}
	
	return 0;
}
