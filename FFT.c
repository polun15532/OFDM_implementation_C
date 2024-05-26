#include "FFT.h"

// Function to reverse bits
static int reverse_bits_n(int x, int bit_number){
    int reverse_x = 0;
    for(int i = 0; i < bit_number; ++i){
    	reverse_x |= ((x >> i) & 1) << ((bit_number - 1) - i);
    }
    return reverse_x;
}

// Function to compute twiddle factors
static void twiddle_factors(complex_t** twiddle_factors_array, int bit_number){
    int FFT_size = 1 << bit_number;
    complex_t* W = malloc(sizeof(complex_t) * FFT_size);

    if (!W) {
        fprintf(stderr, "Memory allocation failed for twiddle factors\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < FFT_size; ++i){
        W[i].real = cos(2 * PI / FFT_size * i);
        W[i].image = - sin(2 * PI / FFT_size * i);
    }
    *twiddle_factors_array = W;
}

// Function to rearrange elements based on bit reversal

static void rearrenge_elements(complex_t** data, int bit_number){

   int FFT_size = 1 << bit_number;

    for(int i = 0; i < FFT_size; ++i){
        int reverse_bit = reverse_bits_n(i, bit_number);
        if(reverse_bit > i)
            swap_complex(&(*data)[i], &(*data)[reverse_bit]);
    }
}

// FFT function
void fft(complex_t** data, int bit_number){
    rearrenge_elements(data, bit_number);
    complex_t* W;
    twiddle_factors(&W, bit_number);   

    int FFT_size = 1 << bit_number, m;

    complex_t x, y, z;
    
    for(int i = 0; i < bit_number; ++i){
        //i為第i層的蝶型運算
        m = 1 << i;
        for(int j = 0; j < FFT_size; j += m << 1){
            
            for(int k = 0; k < m; ++k){
                x = complex_mul((*data)[k + j + m], W[FFT_size * k / (m << 1)]);
                y = complex_add((*data)[j + k], x);
                z = complex_sub((*data)[j + k], x);
                (*data)[j + k] = y;
                (*data)[j + k + m] = z;
            }
        }
    }
    free(W);
}

void ifft(complex_t** data, int bit_number){
    // IFFT(X) = 1 / N conj(FFT(conj(x)));
    int FFT_size = 1 << bit_number;
    for(int i = 0; i < FFT_size; ++i){
        (*data)[i].image *= -1;
    }
    fft(data, bit_number);
    for(int i = 0; i < FFT_size; ++i){
        (*data)[i].image /=  -FFT_size;
        (*data)[i].real /= FFT_size;
    }    
}

