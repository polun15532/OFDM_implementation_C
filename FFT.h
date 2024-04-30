#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_complex.h"

#ifndef PI
# define PI 3.14159265358979323846264338327950288
#endif


int reverse_bits_n(int x, int bit_number){
    int reverse_x = 0;
    for(int i = 0; i < bit_number; ++i){
    	reverse_x |= ((x >> i) & 1) << ((bit_number - 1) - i);
        /*
            ((x >> i) & 1) 用於獲取要待反轉的位元值
            ((bit_number - 1) - i 則是用於計算位元的位置
            ex: x = 0b 00001 
            令 i = 0, bit_number = 5, ((x >> 0) & 1) 相當於取最低位的bit 其值為1
            ((bit_number - 1) - 0) = 4也就是要把這個取值左移4bit
             reverse_x = 0b 10000
        */
    }
    return reverse_x;
}

void twiddle_factor(complex_t** temp, int bit_number){
    int FFT_size = 1 << bit_number;
    complex_t* W = malloc(sizeof(complex_t) * FFT_size);

    for(int i = 0; i < FFT_size; ++i){
        W[i].real = cos(2 * PI / FFT_size * i);
        W[i].image = - sin(2 * PI / FFT_size * i);
    }
    *temp = W;
}

void rearrenge_element(complex_t** data, int bit_number){
    /*
        碼位倒序重組數組
        以 FFT_SIZE = 8為例
        交換前（000，001，010，011，100，101，110，111）
        交換後（000，100，010，110，001，101，011，111)
        由於一次會交換兩個元素所以只需要交換FFT_size / 2次就好
        為了避免元素被重複交換，只有當前元素小於反轉位元後的元素才交換
        反轉位元前後的兩個值相等時不用交換
    */
   int FFT_size = 1 << bit_number;

    for(int i = 0; i < FFT_size; ++i){
        int reverse_bit = reverse_bits_n(i, bit_number);
        if(reverse_bit <= i)
            continue;
        swap_complex(&(*data)[i], &(*data)[reverse_bit]);
    }
}

void fft(complex_t** data, int bit_number){
    rearrenge_element(data, bit_number);
    complex_t* W;
    twiddle_factor(&W, bit_number);   

    int FFT_size = 1 << bit_number, m;

    complex_t x, y, z;
    
    for(int i = 0; i < bit_number; ++i){
        //i為第i層的蝶型運算
        m = 1 << i;
        for(int j = 0; j < FFT_size; j += m << 1){
            
            for(int k = 0; k < m; ++k){
                x = complex_mul((*data)[k + j + m], W[FFT_size * k / m / 2]);
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

