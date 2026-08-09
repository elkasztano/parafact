#include "helpers.h"

bool paraf_is_prime(unsigned n) {
	unsigned i;

	if(n == 1 || n == 0)
		return false;

	for( i = 2; i * i <= n; i++ ) {
		if( n % i == 0 )
			return false;
	}

	return true;
}

unsigned paraf_multiplicity( unsigned x, unsigned prime ) {
	unsigned q = x;
	unsigned sum = 0;
	if( prime > x ) {
		return 0;
	} else if( prime > (x/2) ) {
		return 1;
	} else {
		while( q >= prime ) {
			q /= prime;
			sum += q;
		}
		return sum;
	}
}

void paraf_permutate(unsigned *primes, unsigned *exps, unsigned n) {
       unsigned i, x, tmp_prime, tmp_exp;

       for( i = 0; i < n/2; i++ ) {
	       x = n - i - 1;
	       tmp_prime = *(primes + i);
	       tmp_exp = *(exps + i);
	       *(primes + i) = *(primes + x);
	       *(exps + i) = *(exps + x);
	       *(primes + x) = tmp_prime;
	       *(exps + x) = tmp_exp;
       }
}

void paraf_spread_first_n(unsigned long *primes, unsigned *exps, unsigned first, unsigned n) {
	unsigned long i, x, tmp_prime, tmp_exp;
	unsigned long ratio = n / first;

	for( i = 0; i < first; i++ ) {
		x = ratio * i;
		tmp_prime = *(primes + i);
		tmp_exp = *(exps + i);
		*(primes + i) = *(primes + x);
		*(exps + i) = *(exps + x);
		*(primes + x) = tmp_prime;
		*(exps + x) = tmp_exp;
	}
}

int paraf_is_power_of_two(long x) {
	return x && ( !( x & ( x - 1 ) ) );
}

uint64_t paraf_next_pow2_64(uint64_t n) {
    if (n == 0) return 1;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n + 1;
}
