#ifndef __MONTGOMERY_H
#define __MONTGOMERY_H

#include  <stdio.h>
#include <stdlib.h>
#include    <gmp.h>

#define MAX(a,b) (a > b ? a : b)

/**
 * Finds an R such that R = 2^k, R > N, for the smallest k.
 * Give N
 * Sets R = 2^k, R > N, for the smallest k.
 */
void mont_findR(mpz_t R, const mpz_t N);

/**
 * Converts T into Montgomery Form.
 * Give T, N, R
 * Sets t = T * R (mod N)
 */
void mont_convert(mpz_t t, const mpz_t T, const mpz_t N, const mpz_t R);
void mont_convert_ui(mpz_t t, unsigned long T, const mpz_t N, const mpz_t R);

/**
 * Produces the Montgomery Reduction of T modulo N.
 * Give T, N, R
 * Sets t = T * R^(-1) (mod N)
 */
void mont_redux(mpz_t t, const mpz_t T, const mpz_t N, const mpz_t R);

/**
 * Adds two numbers, a and b, in Montgomery form, under modulo N. Returns answer
 * in Mongomery Form.
 * Give a, b, N
 * Sets rop = a + b (mod N)
 */
void mont_add(mpz_t rop, const mpz_t a, const mpz_t b, const mpz_t N);

/**
 * Subtracts two numbers, a and b, in Montgomery form, under modulo N. Returns 
 * answer in Mongomery Form.
 * Give a, b, N
 * Sets rop = a - b (mod N)
 */
void mont_sub(mpz_t rop, const mpz_t a, const mpz_t b, const mpz_t N);

/**
 * Multiplies two numbers, a and b, in Montgomery form, under modulo N. Returns
 * answer in Montgomery Form.
 * Give a, b, N
 * Sets rop = a * b (mod N)
 */
void mont_mul(mpz_t rop, const mpz_t a, const mpz_t b, 
              const mpz_t N, const mpz_t R);

/**
 * Performs the exponentiation of two numbers under modulo N; base and exp, 
 * where base is in Montgomery form, and exp is in regular base 2. Returns 
 * answer in Montgomery Form. Uses the sliding widow method for exponentiation. 
 * Window size is defined with k.
 * Give base, exp > 0, N, R, k
 * Sets rop = base ^ exp (mod N)
 */
void mont_powm(mpz_t rop, const mpz_t base, const mpz_t exp, 
               const mpz_t N, const mpz_t R, const unsigned char k);

#endif
