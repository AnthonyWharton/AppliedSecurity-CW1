#include "montgomery.h"

void mont_findR(mpz_t R, const mpz_t N)
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

void mont_convert(mpz_t t, const mpz_t T, const mpz_t N, const mpz_t R)
{
	mpz_mul(t, T, R);
	mpz_mod(t, t, N);
}

void mont_convert_ui(mpz_t t, unsigned long T, const mpz_t N, const mpz_t R)
{
	mpz_mul_ui(t, R, T);
	mpz_mod(t, t, N);
}

void mont_redux(mpz_t t, const mpz_t T, const mpz_t N, const mpz_t R)
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

void mont_add(mpz_t rop, const mpz_t a, const mpz_t b, const mpz_t N)
{
	mpz_add(rop, a, b);
	mpz_mod(rop, rop, N);
}

void mont_sub(mpz_t rop, const mpz_t a, const mpz_t b, const mpz_t N)
{
	mpz_sub(rop, a, b);
	mpz_mod(rop, rop, N);
}

void mont_mul(mpz_t rop, const mpz_t a, const mpz_t b, 
              const mpz_t N, const mpz_t R)
{
	mpz_mul(rop, a, b);
	mpz_mod(rop, rop, N);
	mont_redux(rop, rop, N, R);
}

void mont_debug_print(const char *msg, const mpz_t var, const mpz_t N, const mpz_t R)
{
	mpz_t rog, debug;
	mpz_init(rog);
	mpz_init_set(debug, var);
	mont_redux(rog, debug, N, R);
	gmp_printf("DEBUG[%s] %Zd\n", msg, rog);
	mpz_clear(rog);
	mpz_clear(debug);
}

void mont_powm(mpz_t rop, const mpz_t base, const mpz_t exp, 
               const mpz_t N, const mpz_t R, const unsigned char k)
{	
	if (k < 1 || k > 64) abort();

	// Variable Initialisation
	mpz_t base2, _rop, __rop;
	mpz_init(_rop);
	mpz_init(__rop);
	mont_convert_ui(_rop, 1, N, R);
	mpz_init(base2);
	
	// Precompute values of: 
	// base^1, base^3, base^5, ... , base^(2*k - 1) under modulo N
	mpz_t *T = malloc((1 << (k-1)) * sizeof(mpz_t));
	mpz_init_set(T[0], base);
	/* mpz_powm_ui(base2, base, 2, N); // Pre-computed base^2 value */	
	mont_mul(base2, base, base, N, R);

	for (int i = 1; i < (1 << (k-1)); i++) {
		mpz_init(T[i]);
		/* mpz_mul(T[i], T[i-1], base2); */
		/* mpz_mod(T[i], T[i], N); */
		mont_mul(T[i], T[i-1], base2, N, R);
	}
	
	int i = mpz_sizeinbase(exp, 2) - 1; // Counter/Position
	int l = 0;                          // Least sig. bit pos. of the window
	unsigned long u = 0;                // Binary representation of window
	
	// Loop until we hit the end of the number
	while (i >= 0) {
		// Check if bit of exp is 0
		if (mpz_tstbit(exp, i) == 0) {
			l = i;
			u = 0;
		} else {
			// Find least significant bit of window
			l = MAX(i-k+1, 0);
			l = mpz_scan1(exp, l);
			 
			// Extract window into separate variable
			u = 0;
			for (int j=i; j>=l; j--) {
				u = (u << 1) + mpz_tstbit(exp, j);
			}
		}

		/* mpz_set_ui(temp, 1 << (i-l+1)); */
		/* mpz_powm(_rop, _rop, temp, N); */
		mpz_set(__rop, _rop);
		for (unsigned long j = 1; j < (1 << (i-l+1)); j++) {
			mont_mul(_rop, _rop, __rop, N, R);
		}

		// If the window was not empty, multiply by the relevant 
		// precomputed value to the answer for that window.
		if (u != 0) {
			/* mpz_mul(_rop, _rop, T[((u-1)/2)]); */
			/* mpz_mod(_rop, _rop, N); */
			mont_mul(_rop, _rop, T[((u-1)/2)], N, R);
		}

		// Continue next iteration of loop to bits less significant than
		// that of the window we just computed.
		i = l - 1;
	}
	
	// Finally set the answer
	mpz_set(rop, _rop);

	// Cleanup
	mpz_clear(base2);
	mpz_clear(_rop);
	mpz_clear(__rop);
	for (int i = 0; i < (1 << (k-1)); i++) mpz_clear(T[i]);
}
