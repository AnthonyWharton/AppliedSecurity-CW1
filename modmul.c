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

#define WINDOW_SIZE 5

/**
 * Reads a unsigned long from a file descriptor.
 * (Used for reading from entropy source devices)
 */
unsigned long read_lu(int fd) {
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
void generate_random(mpz_t rop, mp_bitcnt_t n)
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
 * Performs rop <- x^y (mod N)
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

/**
 * Finds an R such that R = 2^k, R > N, for the smallest k.
 * Give N
 * Sets R = 2^k, R > N, for the smallest k.
 */
void mont_findR(mpz_t R, mpz_t N)
{
	mpz_t g;
	mpz_init(g);

	unsigned long i = 0;
	while (mpz_cmp_ui(g, 1) != 0) {
		// Set R to be 2^i until R > N, then start calculating GCD(R,N)
		if (i < sizeof(i)-1) {
			mpz_set_ui(R, (1 << ++i));
		} else {
			mpz_mul_ui(R, R, 2);
		}
		if (mpz_cmp(R, N) <= 0) continue;
		mpz_gcd(g, R, N);
	}

	mpz_clear(g);
}

/**
 * Converts T into Montgomery Form.
 * Give T, N, R
 * Sets t = T * R (mod N)
 */
void mont_convert(mpz_t t, mpz_t T, mpz_t N, mpz_t R)
{
	mpz_mul(t, T, R);
	mpz_mod(t, t, N);
}

/**
 * Produces the Montgomery Reduction of T modulo N.
 * Give T, N, R
 * Sets t = T * R^(-1) (mod N)
 */
void mont_redux(mpz_t t, mpz_t T, mpz_t N, mpz_t R)
{
	mpz_t _t, g, ri, ni, m;
	mpz_init(g);
	mpz_init(ri);
	mpz_init(ni);
	mpz_init(m);
	mpz_init_set(_t, t);

	// Work out inverses of R and N
	mpz_gcdext(g, ri, ni, R, N);

	// Work out the Montgomery Reduction of T modulo N
	// m = T * (-N^(-1)) (mod R)
	mpz_neg(ni, ni);
	mpz_mul(m, T, ni);
	mpz_mod(m, m, R);
	// t = T + mN
	mpz_mul(_t, m, N);
	mpz_add(_t, T, _t);
	// t = t / R (mod N)
	mpz_fdiv_q(_t, _t, R);
	mpz_mod(_t, _t, N);

	mpz_set(t, _t);

	mpz_clear(_t);
	mpz_clear(g);
	mpz_clear(ri);
	mpz_clear(ni);
	mpz_clear(m);
}

/**
 * Multiplies two numbers, a and b, in Montgomery form, under modulo N. Returns
 * answer in Montgomery Form.
 * Give a, b, N
 * Sets rop = a * b (mod N)
 */
void mont_mul(mpz_t rop, mpz_t a, mpz_t b, mpz_t N, mpz_t R)
{
	mpz_mul(rop, a, b);
	mpz_mod(rop, rop, N);
	mont_redux(rop, rop, N, R);
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
	
	// Initialisation - Results
	mpz_t rop;
	mpz_init(rop);

	// Main loop, Check for EOF/Read in first input
	while (gmp_scanf("%ZX", N) == 1) {
		// Read rest of inputs
		if (gmp_scanf("%ZX", e) != 1) abort();
		if (gmp_scanf("%ZX", m) != 1) abort();

		// Vanilla RSA encryption
		sliding_exponentiation(rop, m, e, N, 5);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n", rop);
	}

	// Cleanup - Inputs
	mpz_clear(N);
	mpz_clear(e);
	mpz_clear(m);
	
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
	mpz_t m1, m2, a1, a2;
	mpz_init(m1);
	mpz_init(m2);
	mpz_init(a1);
	mpz_init(a2);

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

		// Naive vanilla RSA decryption
		//sliding_exponentiation(m, c, d, N, 5);

		// RSA decryption using CRT
		sliding_exponentiation(m1, c, d_p, p, 5);
		sliding_exponentiation(m2, c, d_q, q, 5);
		mpz_sub(a1, m1, m2);
		mpz_mul(a2, i_q, a1);
		mpz_mod(a1, a2, p); // Reusing a1
		mpz_mul(a2, a1, q); // Reusing a2
		mpz_add(rop, m2, a2);

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
	mpz_clear(m1);
	mpz_clear(m2);
	mpz_clear(a1);
	mpz_clear(a2);

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
void stage3(unsigned char test) 
{
	// Initialisation - Inputs
	mpz_t p, q, g, h, m;
	mpz_init(p);
	mpz_init(q);
	mpz_init(g);
	mpz_init(h);
	mpz_init(m);
	
	// Initialisation - Working Variables
	mpz_t r;
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

		// Vanilla ElGamal Encryption
		mpz_mod(r, r, q);
		sliding_exponentiation(c1, g, r, p, 5);
		sliding_exponentiation(c2, h, r, p, 5);
		mpz_mul(c2, c2, m);
		mpz_mod(c2, c2, p);

		// Output result as capitalised HEX
		gmp_printf("%ZX\n%ZX\n", c1, c2);
	}
	
	// Cleanup - Inputs
	mpz_clear(p);
	mpz_clear(q);
	mpz_clear(g);
	mpz_clear(h);
	mpz_clear(m);
	
	// Cleanup - Working Variables
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
	mpz_t a1, a2;
	mpz_init(a1);
	mpz_init(a2);
	
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

		mpz_neg(a1, x);
		mpz_mod(a2, a1, q);
		sliding_exponentiation(a1, c1, a2, p, 5); // Reusing a1
		mpz_mul(a2, a1, c2);
		mpz_mod(rop, a2, p);

		// Output result as capitalised HEX
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
	mpz_clear(a1);
	mpz_clear(a2);
	
	// Cleanup - Results
	mpz_clear(rop);
}

/**
 * The main function acts as a driver for the assignment by simply invoking the
 * correct function for the requested stage.
 */
int main(int argc, char* argv[])
{
	if (argc != 2) {
		mpz_t rop, rop_m, R, t1, t2, N, T1, T2;
		mpz_init(rop);
		mpz_init(rop_m);
		mpz_init(R);
		mpz_init(t1);
		mpz_init(t2);
		mpz_init_set_ui(N, 109);
		mpz_init_set_ui(T1, 68);
		mpz_init_set_ui(T2, 57);
		
		mont_findR(R, N);
		gmp_printf("N:%Zd, R:%Zd\n", N, R);

		mont_convert(t1, T1, N, R);
		mont_convert(t2, T2, N, R);
		gmp_printf("t1:%Zd\nt2:%Zd\n", t1, t2);

		mont_mul(rop_m, t1, t2, N, R);
		gmp_printf("rop_m:%Zd\n", rop_m);

		mont_redux(rop, rop_m, N, R);
		gmp_printf("rop:%Zd\n", rop);

		mpz_clear(rop);
		mpz_clear(rop_m);
		mpz_clear(R);
		mpz_clear(t1);
		mpz_clear(t2);
		mpz_clear(N);
		mpz_clear(T1);
		mpz_clear(T2);

		return 1;
	}

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

