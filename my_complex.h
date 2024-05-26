#pragma once

typedef struct {
    double real;
    double image;
} complex_t;

complex_t complex_add(complex_t x, complex_t y);

complex_t complex_sub(complex_t x, complex_t y);

complex_t complex_mul(complex_t x, complex_t y);

complex_t complex_div(complex_t x, complex_t y);

void swap_complex(complex_t* a, complex_t* b);

complex_t conjugate(complex_t a);


