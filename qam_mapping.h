#include <stdio.h>
#include <stdlib.h>
#include "uthash.h"
#include "my_complex.h"
typedef struct{
    int key;
    int real;
    int image;
    UT_hash_handle hh;
} QAM_symbol_t;

// generate signal in frequenct domain
void QAM_mapping(int *data, int bits_per_symbol, int *Gray_code_converter_table, int carrier_number, int OFDM_symbol, int FFT_size, complex_t ***QAM_data);