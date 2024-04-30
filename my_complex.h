#ifndef _MY_COMPLEX_H_
#define _MY_COMPLEX_H_

typedef struct {
    double real;
    double image;
} complex_t;

complex_t complex_add(complex_t x, complex_t y){
    complex_t z;
    z.real = x.real + y.real;
    z.image = x.image + y.image;
    return z;
}

complex_t complex_sub(complex_t x, complex_t y){
    complex_t z;
    z.real = x.real - y.real;
    z.image = x.image - y.image;
    return z;
}

complex_t complex_mul(complex_t x, complex_t y){
    complex_t z;
    z.real = x.real * y.real - x.image * y.image;
    z.image = x.real * y.image + x.image * y.real;
    return z;
}

complex_t complex_div(complex_t x, complex_t y){
    complex_t z;
    z.real = x.real * y.real + x.image * y.image;
    z.image = x.real * y.image - x.image * y.real;
    return z;
}

void swap_complex(complex_t* a, complex_t* b){
    complex_t temp = *a;
    *a = *b;
    *b = temp;
}

complex_t 
conjugate(complex_t a){
    a.image = - a.image;
    return a;
}

#endif



