#ifndef PARAF_HELPERS_H
#define PARAF_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

#define PARAF_OK        0
#define PARAF_ERR_NOMEM 1

bool paraf_is_prime(unsigned n);

unsigned paraf_multiplicity(unsigned x, unsigned prime);

void paraf_permutate(unsigned *primes, unsigned *exps, unsigned n);

void paraf_spread_first_n(unsigned long *primes, unsigned *exps, unsigned first, unsigned n);

int paraf_is_power_of_two(long x);

uint64_t paraf_next_pow2_64(uint64_t);

#endif
