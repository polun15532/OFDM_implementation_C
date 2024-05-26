#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_complex.h"
#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

// FFT function
void fft(complex_t** data, int bit_number);
// IFFT function
void ifft(complex_t** data, int bit_number);

