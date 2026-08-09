#ifndef PARAF_CLI_H
#define PARAF_CLI_H
#include <stdint.h>

#define PARAF_OUTPUT_INT (1 << 0)
#define PARAF_OUTPUT_SCI (1 << 1)

typedef struct {
	uint64_t f;
	uint64_t threads;
	uint8_t verbosity;
	uint8_t output_format;
} paraf_cli_t;

paraf_cli_t paraf_cli_parse(int argc, char **argv);

#endif
