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
	mpz_t rop, N, e, m;
	mpz_init(rop);
	mpz_init(N);
	mpz_init(e);
	mpz_init(m);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", N) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", e) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Vanilla RSA encryption
		mpz_powm(rop, m, e, N);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", rop);
	}

	// Done - cleanup
	mpz_clear(rop);
	mpz_clear(N);
	mpz_clear(e);
	mpz_clear(m);
}

/* Perform stage 2:
 * 
 * - read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
 * - compute the RSA decryption m, then
 * - write the plaintext m to stdout.
 */
void stage2() 
{

	// fill in this function with solution

}

/* Perform stage 3:
 * 
 * - read each 5-tuple of p, q, g, h and m from stdin,
 * - compute the ElGamal encryption c = (c_1,c_2), then
 * - write the ciphertext c to stdout.
 */
void stage3() 
{

	// fill in this function with solution

}

/* Perform stage 4:
 * 
 * - read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
 * - compute the ElGamal decryption m, then
 * - write the plaintext m to stdout.
 */
void stage4() 
{

	// fill in this function with solution

}

/* The main function acts as a driver for the assignment by simply invoking the
 * correct function for the requested stage.
 */
int main(int argc, char* argv[])
{
	if (argc != 2) {
		abort();
	}
	
	if      (!strcmp(argv[1], "stage1")) {
		stage1();
	}
	else if (!strcmp(argv[1], "stage2")) {
		stage2();
	}
	else if (!strcmp(argv[1], "stage3")) {
		stage3();
	}
	else if (!strcmp(argv[1], "stage4")) {
		stage4();
	}
	else {
		abort();
	}
	
	return 0;
}
