#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gmp.h>
#include <pthread.h>
#include <unistd.h>

#include "helpers.h"
#include "esieve.h"
#include "cli.h"

struct mulPrimes {
	unsigned long *primes;
	unsigned *exp;
	int len;
	int slot;
};

struct finalMul {
	mpz_t *a;
	mpz_t *b;
	int slot;
};

void *mul_par(void* mulprimes);
void *finalize_par(void* bignums);
void *markNonPrimes_par(void *bounds);

uint8_t *sieve;

mpz_t *results, *thread_res;

pthread_mutex_t mul_mutex;
uint8_t thread_verbosity;

int main(int argc, char **argv) {
	unsigned b, i, n, ctr_temp;
	unsigned long *primes = NULL;
	unsigned long sieve_len, nr_primes;
	unsigned *exps = NULL;
	long threads = sysconf(_SC_NPROCESSORS_ONLN);
	unsigned shrink_threads = 0;
	pthread_t *pth = NULL;
	struct mulPrimes *mupr = NULL;
	struct finalMul *fmul = NULL;
	int main_err = PARAF_OK;

	paraf_cli_t cli = paraf_cli_parse(argc, argv);

	mpz_t fc;
	mpz_init(fc);

	b = cli.f;
	thread_verbosity = cli.verbosity;

	/* fallback to integer output */
	if (!cli.output_format)
		cli.output_format = PARAF_OUTPUT_INT;

	if (b < 2) {
		if (cli.output_format & PARAF_OUTPUT_INT)
			printf("1\n");
		if (cli.output_format & PARAF_OUTPUT_SCI)
			printf("1.0e0\n");
		goto cleanup;
	}

	sieve = paraf_init_esieve((unsigned long)b, &sieve_len);
	if (sieve == NULL) {
		fprintf(stderr, "unable to initialize prime sieve\n");
		return 1;
	}
	
	paraf_mark_non_primes(sieve, (unsigned long)b);

	primes = paraf_esieve_primes(sieve, (unsigned long)b, &nr_primes); 
	if (sieve)
		free(sieve);

	if (cli.verbosity > 1)
		fprintf(stderr, "found %lu primes\n", nr_primes);

	exps = malloc(sizeof(unsigned) * nr_primes);
	if(exps == NULL) {
		fprintf(stderr, "malloc failed for prime exponents\n");
		main_err = PARAF_ERR_NOMEM;
		goto cleanup;
	}

	/* count primes */
	unsigned first_nexp = 0;
	for (i = 0; i < nr_primes; i++) {
		exps[i] = paraf_multiplicity(b, primes[i]);
		if (exps[i] > 7)
			first_nexp++;
	}

	if (cli.verbosity > 1)
		fprintf(stderr, "first %u elements will be distributed\n", first_nexp);

	if (first_nexp > 0) {  // Only spread if there are primes to spread
		paraf_spread_first_n(primes, exps, first_nexp, nr_primes);
	}

	/* determine number of threads with fallback */
	if (threads <= 0) {
		threads = 1;
	}

	if (cli.threads > 0) {
		threads = cli.threads;
	}

	/* round threads up to the next power of 2 */
	threads = paraf_next_pow2_64(threads);

	shrink_threads = (unsigned)threads;
	pth = malloc(sizeof(pthread_t) * threads);
	if (pth == NULL) {
		fprintf(stderr, "malloc failed for pthreads\n");
		main_err = PARAF_ERR_NOMEM;
		goto cleanup;
	}

	/* allocate results as an array of mpz_t structures */
	results = malloc(sizeof(mpz_t) * threads);
	for (i = 0; i < threads; i++) {
		mpz_init(results[i]);
	}

	/* Allocate thread_res as an array of mpz_t structures */
	thread_res = malloc(sizeof(mpz_t) * (shrink_threads / 2));
	for (i = 0; i < shrink_threads / 2; i++) {
		mpz_init(thread_res[i]);
	}

	if (cli.verbosity > 0)
		fprintf(stderr, "using %ld threads\n", threads);

	n = nr_primes / threads;
	ctr_temp = nr_primes;

	mupr = malloc( sizeof( struct mulPrimes ) * threads );
	if (mupr == NULL) {
		fprintf(stderr, "malloc failed for prime multiplication\n");
		main_err = PARAF_ERR_NOMEM;
		goto cleanup;
	}

	/* first threads, all with equal load */
	for( i = 0; i < threads - 1; i++ ) {
		( mupr + i ) -> primes = ( primes + i * n );
		( mupr + i ) -> exp = ( exps + i * n );
		( mupr + i ) -> len = n;
		( mupr + i ) -> slot = i;
		ctr_temp -= n;
	}

	/* last thread, will get the rest */

	( mupr + threads - 1 ) -> primes = ( primes + ( threads - 1 ) * n );
	( mupr + threads - 1 ) -> exp = ( exps + ( threads - 1 ) * n );
	( mupr + threads - 1 ) -> len = ctr_temp;
	( mupr + threads - 1 ) -> slot = threads - 1;

	pthread_mutex_init(&mul_mutex, NULL);

	for( i = 0; i < threads; i++ ) {
		if (pthread_create(pth + i, NULL, mul_par, (mupr + i)) != 0) {
			fprintf(stderr, "pthread creation failed\n");
			goto cleanup;
		}
	}

	for( i = 0; i < threads; i++ ) {
		pthread_join(*(pth + i), NULL );
	}

	fmul = malloc( sizeof( struct finalMul ) * shrink_threads / 2 );
	if (fmul == NULL) {
		fprintf(stderr, "malloc failed for final multiplication\n");
		goto cleanup;
	}

	thread_res = malloc(sizeof(mpz_t) * shrink_threads / 2);
	if (thread_res == NULL) {
		fprintf(stderr, "malloc failed for thread local results\n");
		goto cleanup;
	}

	for (i = 0; i < shrink_threads / 2; i++) {
		mpz_init(thread_res[i]);
	}

	while(shrink_threads > 2) {
		shrink_threads >>= 1;

		if (cli.verbosity > 2)
			fprintf(stderr, "shrink_threads = %u\n", shrink_threads);

		for( i = 0; i < shrink_threads; i++ ) {
			(fmul + i)->a = (results + (i * 2));
			(fmul + i)->b = (results + (i * 2) + 1);
			(fmul + i)->slot = i;
			if (pthread_create(pth + i, NULL, finalize_par, (fmul + i )) != 0) {
				fprintf(stderr, "pthread creation failed\n");
				goto cleanup;
			}
		}

		for( i = 0; i < shrink_threads; i++ ) {
			pthread_join( *(pth + i), NULL );
		}

		for( i = 0; i < shrink_threads; i++ ) {
			mpz_set( *( results + i ), *( thread_res + i ) );
		}

	}


	mpz_set_ui( fc, 1 );

	/* the very last calculation in the main thread */
	for( i = 0; i < shrink_threads; i++ ) {
		mpz_mul( fc, fc, *( results + i ) );
	}

	if (cli.verbosity > 0)
		fprintf( stderr, "calculations completed, formatting to base 10 ... \n" );

	if (cli.output_format & PARAF_OUTPUT_INT) {
		mpz_out_str(stdout, 10, fc);
		printf("\n");
	}

	/* scientific floating point notation */
	if (cli.output_format & PARAF_OUTPUT_SCI) {
		mpf_t f;
		mpf_init(f);
		mpf_set_z(f, fc);

		gmp_printf("%Fe\n", f);
		mpf_clear(f);
	}

cleanup:
	mpz_clear(fc);

	free(primes);
	free(exps);

	if (results != NULL) {
		for (i = 0; i < threads; i++) {
			mpz_clear(results[i]);
		}
		free(results);
	}

	if (pth != NULL) {
		free(pth);
	}

	if (mupr != NULL) {
		free(mupr);
	}

	if (fmul != NULL) {
		free(fmul);
	}

	if (thread_res != NULL) {
		for (i = 0; i < (threads / 2); i++) {
			mpz_clear(thread_res[i]);
		}
		free(thread_res);
	}

	pthread_mutex_destroy(&mul_mutex);

	return main_err;
}

void *mul_par(void* mulprimes) {
	struct mulPrimes *local = (struct mulPrimes *)mulprimes;
	mpz_t lclres, tmp;
	mpz_init( lclres );
	mpz_init( tmp );
	mpz_set_ui( lclres, 1U );

	for( int i = 0; i < local->len; i++ ) {
		mpz_ui_pow_ui( tmp, *(local->primes + i), *(local->exp + i) );
		mpz_mul( lclres, lclres, tmp );
	}

	pthread_mutex_lock( &mul_mutex );
	mpz_init( *(results + local->slot) );
	mpz_set( *(results + local->slot), lclres );
	pthread_mutex_unlock( &mul_mutex );

	mpz_clear( lclres );
	mpz_clear( tmp );
	if (thread_verbosity > 2)
		fprintf( stderr, "thread %u finished\n", local->slot );
	return NULL;
}

void *finalize_par( void* bignums ) {
	struct finalMul *local = (struct finalMul *)bignums;
	mpz_t lclres;
	mpz_init( lclres );
	mpz_mul( lclres, *(local->a), *(local->b) );
	pthread_mutex_lock( &mul_mutex );
	mpz_set( *(thread_res + local->slot), lclres );
	pthread_mutex_unlock( &mul_mutex );
	mpz_clear( lclres );
	return NULL;
}

