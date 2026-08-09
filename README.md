# ParaFact

Compute factorials with parallel processing and arbitrary precision. **ParaFact** is a multithreaded factorial calculator that leverages prime factorization and the GMP library to efficiently compute *n!* for very large values of *n*.

## Features

- **Parallel computation** using pthreads to distribute workload across CPU cores
- **Prime factorization** via the Sieve of Eratosthenes and Legendre's formula for optimal performance
- **Arbitrary precision** through the GNU Multiple Precision Arithmetic Library (GMP)
- **Output formats**: Integer (default) or scientific notation
- **Verbose mode** for debugging and performance monitoring

## Limitations

- **Memory-bound**: Large factorials require significant memory (proportional to the number of primes ≤ *n*)
- **Thread count constraint**: Number of threads is automatically rounded up to the next power of two

## Building

### Prerequisites

- **C compiler** — `gcc`, `clang`, or compatible (set via `CC` environment variable)
- **GNU MP (GMP)** – GNU Multiple Precision Arithmetic Library
- **GNU Make**

### Compile

Navigate to the `parafact` directory.
```bash
make
```
This produces the binary at `target/parafact`.

### Install

The currently preferred way is to just create a symlink in a directory in your `PATH`.
```bash
ln -s $PWD/target/parafact ~/bin
```

### Remove build artifacts

```bash
make clean
```

## Usage

```bash
parafact [options]
```

### Options

| Option | Long Form | Argument | Description |
|--------|-----------|----------|-------------|
| `-f` | `--factorial` | `N` | Compute *N!* (defaults to 1) |
| `-t` | `--threads` | `N` | Number of threads to use. Rounded up to the next power of two. |
| `-o` | `--output-format` | `FORMAT` | Output format: `integer` (default) or `scientific` |
| `-v` | `--verbose` | — | Increase verbosity level (0-3) |
| `-h` | `--help` | — | Display help text and exit |
| `-V` | `--version` | — | Display version information and exit |

### Examples

Compute 10! (Default: Integer Output)
```bash
parafact -f 10
```

Compute 20! with Scientific Notation
```bash
parafact -f 20 -o scientific
```

Compute 50! with 8 Threads and Verbose Output
```bash
parafact -f 50 -t 8 -vvv
```

Compute 100! with Maximum Verbosity
```bash
parafact -f 100 -vvv
```

## Algorithms

### Sieve of Eratosthenes

The **Sieve of Eratosthenes** is used to efficiently generate all prime numbers up to *n*. This ancient algorithm iteratively marks the multiples of each prime starting from 2.

- **Reference**: [Sieve of Eratosthenes – Wikipedia](https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes)

### Legendre's Formula

To compute the multiplicity of a prime *p* in *n!*, we use **Legendre's formula**.

- **Reference**: [Legendre's formula – Wikipedia](https://en.wikipedia.org/wiki/Legendre%27s_formula)

### Reduction Tree

The partial products from each thread are combined using a **binary reduction tree**, where pairs of results are multiplied in parallel until only the final result remains.

### GNU Multiple Precision Arithmetic Library (GMP)

**GMP** is a free library for arbitrary precision arithmetic, operating on signed integers, rational numbers, and floating-point numbers.

- **Reference**: [GNU MP – Official Site](https://gmplib.org/)
- **Documentation**: [GMP Manual](https://gmplib.org/manual/)

## Todo

- **Parallelization of decimal output**: Currently the bottleneck is the final conversion from binary (`mpz_t`) to decimal. Parallelizing this step (e.g., via divide-and-conquer on the digit string) could significantly improve performance for very large factorials.

- **Thread count constraint**: The number of threads is currently rounded up to the next power of two, which can create idle threads for small factorials. Adding a `--round-down` option would let users optimize for their use case (e.g., small factorials benefit from rounding down, while large factorials may prefer rounding up).

