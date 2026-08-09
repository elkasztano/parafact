/* based on the Sieve of Eratosthenes */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t getBit(uint8_t *sieve, size_t x) {
	return( ( *( sieve + ( x / 8 ) ) >> ( x % 8 ) ) & 1 );
}

void unsetBit(uint8_t *sieve, size_t x) {
	*( sieve + ( x / 8 ) ) &= ~( 1 << ( x % 8 ) );
}

/* caller is responsible for deallocating memory */
uint8_t *paraf_init_esieve(unsigned long max, unsigned long *array_len) {
	uint8_t *allocated;
	unsigned long i;

	/* one byte can represent 16 numbers because we can leave
	 * the even numbers out as they are all multiples of two */
	*array_len = max / 16 + 2;

	allocated = malloc(*array_len);
	if(allocated == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	for(i = 0; i < *array_len; i++) {
		*(allocated + i) = 0xFF;
	}

	return allocated;
}

/* attempt for efficacy */
void paraf_mark_non_primes(uint8_t *sieve, unsigned long max) {
	unsigned long i,p;
	
	for(p = 3; p * p <= max; p+=2) {
		if(getBit( sieve, p/2 - 1)) {
			for(i = p * p; i <= max; i += p)
				if(i & 1UL) /* if > 2 only odd numbers can be prime */
					unsetBit( sieve, i/2 - 1);
		}
	}
}

unsigned long *paraf_esieve_primes(uint8_t *sieve, unsigned long max, unsigned long *n_primes) {
	unsigned long i, ctr = 0;
	unsigned long *list;
	/* count found primes */
	if(max < 2UL) {
		*n_primes = 0UL;
		return NULL;
	} else if(max == 2UL) {
		*n_primes = 1UL;
		list = malloc( sizeof( unsigned long ) );
		*list = 2UL;
		return list;
	}
	
	*n_primes = 1UL; /* two is the first prime number */
	for(i = 3; i <= max; i+=2) {
		if( getBit( sieve, i/2 - 1) )
			*n_primes += 1;
	}
	
	/* allocate memory for found prime numbers */
	list = ( unsigned long *)malloc( sizeof( unsigned long ) * *n_primes );
	if( list == NULL ) {
		fprintf( stderr, "out of memory\n" );
		exit( 1 );
	}

	/* it is known that the first prime number is two */
	list[ctr++] = 2UL;

	/* put numbers in list */
	for( i = 3; i <= max; i+=2 ) {
		if( getBit( sieve, i/2 - 1 ) )
			list[ctr++] = i;
	}

	return list;
}

