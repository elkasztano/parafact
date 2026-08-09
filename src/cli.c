#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cli.h"

void print_help_text(char *progname);

paraf_cli_t paraf_cli_parse(int argc, char **argv) {
	int c, option_index = 0;

	paraf_cli_t cli;
	
	/* default values */
	cli.f = 0;
	cli.verbosity = 0;
	cli.threads = 0;
	cli.output_format = 0;

	while( 1 ) {
		static struct option long_options[] = {
			{ "factorial", required_argument, 0, 'f' },
			{ "verbose", no_argument, 0, 'v' },
			{ "threads", required_argument, 0, 't' },
			{ "output-format", required_argument, 0, 'o' },
			{ "help", no_argument, 0, 'h' },
			{ "version", no_argument, 0, 'V' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long( argc, argv, "hVvf:t:o:", long_options, &option_index );
		
		if( c == -1 )
			break;

		switch( c ) {
			case 'f':
				cli.f = strtoull(optarg, NULL, 10);
				break;
			case 't':
				cli.threads = strtoull(optarg, NULL, 10);
				break;
			case 'v':
				cli.verbosity++;
				break;
			case 'o':
				if (!strcmp("integer", optarg))
					cli.output_format |= PARAF_OUTPUT_INT;
				else if (!strcmp("scientific", optarg))
					cli.output_format |= PARAF_OUTPUT_SCI;
				break;
			case 'h':
				print_help_text(*argv);
				exit(0);
			case 'V':
				printf("ParaFact 1.0\n");
				exit(0);
			case '?':
				print_help_text(*argv);
				exit(1);
			default:
				print_help_text(*argv);
				exit(1);
		}
	}

	return(cli);

}

void print_help_text(char *progname) {
	printf( "Usage: %s [options]\n"
		"-f, --factorial ......... number to calculate the factorial of, the n in n!\n"
		"-t, --threads ........... specify number of threads\n"
		"                          value is rounded up to the next power of two\n"
		"-o, --output-format ..... output style, valid: 'integer' (default), 'scientific'\n"
		"-v, --verbosity ......... verbosity level (0..3)\n"
		"-h, --help .............. show help and exit\n"
		"-V, --version ........... show version and exit\n",
		progname );
}

