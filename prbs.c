#include "prbs.h"
// PRBS_generate function generates a Pseudo-Random Binary Sequence
void PRBS_generate(int data_length, int bits_per_symbol, int **PRBS) {
    int mask = (1 << bits_per_symbol) - 1; // Create mask for the number of bits per symbol
    int shift_register = 0x7FFF; // Initialize state value with all bits set (15-bit value)
    int new_bit;
    int* PRBS_data_array = malloc(sizeof(int) * data_length); // Allocate memory for the PRBS data array
    
    if (!PRBS_data_array) { // Check for memory allocation failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < data_length; ++i) {
        PRBS_data_array[i] = shift_register & mask; // Store current PRBS value masked to bits_per_symbol
        
        for (int j = 0; j < bits_per_symbol; ++j) { // Change loop condition to j < bits_per_symbol
            // XOR the 15th and 14th bits to create the new bit
            new_bit = ((shift_register >> 14) & 1) ^ ((shift_register >> 13) & 1); // Correct bit positions
            shift_register = ((shift_register << 1) | new_bit) & 0x7FFF; // Ensure state remains 15 bits
        }
    }

    *PRBS = PRBS_data_array; // Return the generated PRBS data array
}