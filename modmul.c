/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */
#include "modmul.h"

#define ENTROPY_SOURCE "/dev/urandom"
#define RANDOM_SAMPLES 4
#define RANDOM_SIZE GMP_LIMB_BITS * RANDOM_SAMPLES

#define WINDOW_SIZE 4

/**
 * Reads a unsigned long from a file descriptor.
 * (Used for reading from entropy source devices)
 */
unsigned long read_lu(const int fd) {
	unsigned long seed_data;
	ssize_t res = read(fd, &seed_data, sizeof(seed_data));
	if (res < 0) {
		printf("Could not read from %s as an entropy source\n", 
		       ENTROPY_SOURCE);
		abort();
	}
	return seed_data;
}

/**
 * Generates a random number (rop) between 0 and 2^n-1 inclusive.
 */
void generate_random(mpz_t rop, const mp_bitcnt_t n)
{
	int rnd_file = open("/dev/urandom", O_RDONLY);
	if (rnd_file < 0) {
		printf("Could not open %s as an entropy source\n", 
		       ENTROPY_SOURCE);
		abort();
	} else {
		mpz_t seed;
		mpz_init2(seed, RANDOM_SIZE);
		unsigned long seed_data;
		
		for (int i = 0; i < RANDOM_SAMPLES; i++) {
			// Read 64 bits from entropy source, add to seed limb
			seed_data = read_lu(rnd_file);
			seed->_mp_d[i] = seed_data;
			seed->_mp_size++;
		}
		
		// Set up GMP Random Number State
		gmp_randstate_t state;
		gmp_randinit_mt(state);
		gmp_randseed(state, seed);

		// Generate Random Number
		mpz_urandomb(rop, state, n);

		// Clean up
		mpz_clear(seed);
		gmp_randclear(state);
	}
}

/** 
 * Perform stage 1:
 * 
 * - read each 3-tuple of N, e and m from stdin,
 * - compute the RSA encryption c, then
 * - write the ciphertext c to stdout.
 */
void stage1() 
{
	// Initialisation - Inputs
	mpz_t N, e, m;
	mpz_init(N);
	mpz_init(e);
	mpz_init(m);

	// Initialisation - Working Variables
	mpz_t R;
	mpz_init(R);

	// Initialisation - Results
	mpz_t rop;
	mpz_init(rop);

	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", N) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", e) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Montgomery Conversion/Setup
		mont_findR(R, N);
		mont_convert(m, m, N, R);

		// Vanilla RSA encryption
		mont_powm(rop, m, e, N, R, WINDOW_SIZE);

		// Output result as capitalised HEX
		mont_redux(rop, rop, N, R);
		gmp_printf("%ZX\n", rop);
	}

	// Cleanup - Inputs
	mpz_clear(N);
	mpz_clear(e);
	mpz_clear(m);

	// Cleanup - Working Variables
	mpz_clear(R);
	
	// Cleanup - Results
	mpz_clear(rop);
}

/**
 * Perform stage 2:
 * 
 * - read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
 * - compute the RSA decryption m, then
 * - write the plaintext m to stdout.
 */
void stage2() 
{
	// Initialisation - Inputs
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

	// Initialisation - Working Variables
	mpz_t Rp, Rq, m1, m2, a1, cp, cq;
	mpz_init(Rp);
	mpz_init(Rq);
	mpz_init(m1);
	mpz_init(m2);
	mpz_init(a1);
	mpz_init(cp);
	mpz_init(cq);

	// Initialisation - Results
	mpz_t rop;
	mpz_init(rop);
	
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
		
		// Montgomery Conversion/Setup
		mont_findR(Rp, p);
		mont_findR(Rq, q);
		mont_convert(cp, c, p, Rp);
		mont_convert(cq, c, q, Rq);

		// Naive vanilla RSA decryption
		//mont_powm(m, c, d, N, NULL, 5);

		// RSA decryption using CRT
		mont_powm(m1, cp, d_p, p, Rp, WINDOW_SIZE);
		mont_powm(m2, cq, d_q, q, Rq, WINDOW_SIZE);

		mont_redux(m1, m1, p, Rp);
		mont_redux(m2, m2, q, Rq);
		mpz_sub(a1, m1, m2);
		mpz_mul(a1, i_q, a1);
		mpz_mod(a1, a1, p);
		mpz_mul(a1, a1, q);
		mpz_add(rop, m2, a1);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", rop);
	}

	// Cleanup - Inputs
	mpz_clear(N);
	mpz_clear(d);
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(d_p);
	mpz_clear(d_q);
	mpz_clear(i_p);
	mpz_clear(i_q);
	mpz_clear(c);

	// Cleanup - Working Variables
	mpz_clear(Rp);
	mpz_clear(Rq);
	mpz_clear(m1);
	mpz_clear(m2);
	mpz_clear(a1);
	mpz_clear(cp);
	mpz_clear(cq);

	// Clearnup - Results
	mpz_clear(rop);
}

/**
 * Perform stage 3:
 * 
 * - read each 5-tuple of p, q, g, h and m from stdin,
 * - compute the ElGamal encryption c = (c_1,c_2), then
 * - write the ciphertext c to stdout.
 *
 * test == 0: output as required by the spec, using random number generation
 * test != 0: output as required by the spec, using fixed "random" value of 1
 *            for testing against given input/output files
 */
void stage3(const unsigned char test) 
{
	// Initialisation - Inputs
	mpz_t p, q, g, h, m;
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(h);
	mpz_init(m);
	
	// Initialisation - Working Variables
	mpz_t R, r;
	mpz_init(R);
	mpz_init(r);
	
	// Initialisation - Results
	mpz_t c1, c2;
	mpz_init(c1);
	mpz_init(c2);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", p) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", q) != 1) abort();
		if (gmp_scanf("%ZX", g) != 1) abort();
		if (gmp_scanf("%ZX", h) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Set r, refer to documentation at function declaration
		if (test) {
			mpz_set_ui(r, 1);
		} else {
			generate_random(r, RANDOM_SIZE);
		}

		// Montgomery Conversion/Setup
		mont_findR(R, p);
		mont_convert(g, g, p, R);
		mont_convert(h, h, p, R);
		mont_convert(m, m, p, R);

		// Vanilla ElGamal Encryption
		mpz_mod(r, r, q);
		mont_powm(c1, g, r, p, R, WINDOW_SIZE);
		mont_powm(c2, h, r, p, R, WINDOW_SIZE);
		mont_mul(c2, c2, m, p, R);

		// Output result as capitalised HEX
		mont_redux(c1, c1, p, R);
		mont_redux(c2, c2, p, R);
		gmp_printf("%ZX\n%ZX\n", c1, c2);
	}
	
	// Cleanup - Inputs
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(g);
	mpz_clear(h);
	mpz_clear(m);
	
	// Cleanup - Working Variables
	mpz_clear(R);
	mpz_clear(r);

	// Cleanup - Results
	mpz_clear(c1);
	mpz_clear(c2);
}

/**
 * Perform stage 4:
 * 
 * - read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
 * - compute the ElGamal decryption m, then
 * - write the plaintext m to stdout.
 */
void stage4() 
{
	// Initialisation - Inputs
	mpz_t p, q, g, x, c1, c2;
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(x);
	mpz_init(c1);
	mpz_init(c2);
	
	// Initialisation - Working Variables
	mpz_t R, a1;
	mpz_init(R);
	mpz_init(a1);
	
	// Initialisation - Results
	mpz_t rop;
	mpz_init(rop);
	
	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", p) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", q ) != 1) abort(); 
		if (gmp_scanf("%ZX", g ) != 1) abort();
		if (gmp_scanf("%ZX", x ) != 1) abort();
		if (gmp_scanf("%ZX", c1) != 1) abort();
		if (gmp_scanf("%ZX", c2) != 1) abort();

		// Montgomery Conversion/Setup
		mont_findR(R, p);
		mont_convert(c1, c1, p, R);
		mont_convert(c2, c2, p, R);

		// ElGamal Decryption
		mpz_neg(a1, x);
		mpz_mod(a1, a1, q);
		mont_powm(a1, c1, a1, p, R, WINDOW_SIZE);
		mont_mul(rop, a1, c2, p, R);

		// Output result as capitalised HEX
		mont_redux(rop, rop, p, R);
		gmp_printf("%ZX\n", rop);
	}

	// Cleanup - Inputs
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(g);
	mpz_clear(x);
	mpz_clear(c1);
	mpz_clear(c2);
	
	// Cleanup - Working Variables
	mpz_clear(R);
	mpz_clear(a1);
	
	// Cleanup - Results
	mpz_clear(rop);
}

/**
 * The main function acts as a driver for the assignment by simply invoking the
 * correct function for the requested stage.
 */
int main(int argc, char* argv[])
{
	if (argc != 2) return 1;

	if (!strcmp(argv[1], "stage1")) {
		stage1();
	} else if (!strcmp(argv[1], "stage2")) {
		stage2();
	} else if (!strcmp(argv[1], "stage3")) {
		stage3(0);
	} else if (!strcmp(argv[1], "stage3-test")) {
		stage3(1);
	} else if (!strcmp(argv[1], "stage4")) {
		stage4();
	} else {
		return 1;
	}
	
	return 0;
}

