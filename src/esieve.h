#ifndef PARAF_ESIEVE_H
#define PARAF_ESIEVE_H

uint8_t *paraf_init_esieve(unsigned long max, unsigned long *array_len);

void paraf_mark_non_primes(uint8_t *sieve, unsigned long max);

unsigned long *paraf_esieve_primes(uint8_t *sieve, unsigned long max, unsigned long *n_primes);

#endif

