#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include"prbs.h"
#include"qam_mapping.h"
#include"FFT.h"

#define GRAY_CODE(num) (num ^ (num >> 1))


int main(){
    //Parameter setting
    int FFTSize_log2   = 12;
    int FFTSize        = 1 << FFTSize_log2;
    int bandwidth      = 3;
    int sampling_rate  = 64;
    int bits_per_symbol= 2;
    int carrier_number = FFTSize * bandwidth / sampling_rate;
    int OFDM_symbol    = 256; 
    int cyclic_prefix  = FFTSize / 8; // Cyclic prefix length, set to 12.5% of FFT size
    int data_length    = carrier_number * OFDM_symbol;

    int *PRBS_data, *TS_data;

    // PRBS data generation with formula: PRBS31 = X^15 + X^14 + 1
    PRBS_generate(data_length, bits_per_symbol, &PRBS_data); 
    PRBS_generate(carrier_number, 1, &TS_data); 

   // Convert PRBS data to Gray code
    for(int i = 0; i < data_length; ++i)
        PRBS_data[i] = GRAY_CODE(PRBS_data[i]);
    
    int table_size = 1 << (bits_per_symbol / 2 + bits_per_symbol % 2);
    int *Gray_code_converter_table = malloc(sizeof(int) * (table_size));
    
    for(int i = 0; i < table_size; ++i)
        Gray_code_converter_table[GRAY_CODE(i)] = i;

    complex_t **signal_data, **TS_signal_data;

    // Map PRBS data and training symbol to QAM frequency domain signals
    QAM_mapping(PRBS_data, bits_per_symbol, Gray_code_converter_table, 
                carrier_number, OFDM_symbol, FFTSize, &signal_data);
    QAM_mapping(TS_data, 1, Gray_code_converter_table, 
                carrier_number, 1, FFTSize, &TS_signal_data);

    // Free memory
    free(Gray_code_converter_table), free(PRBS_data), free(TS_data);

    // Perform IFFT on training symbol
    ifft(TS_signal_data, FFTSize_log2);

    // Perform IFFT on each OFDM symbol
    for(int i = 0; i < OFDM_symbol; ++i)
        ifft(&signal_data[i], FFTSize_log2);

    double avg_amp = 0, avg_TS = 0;
    double *signal_after_cp = malloc(sizeof(double) * (OFDM_symbol + 2) * (FFTSize + cyclic_prefix));

    // Add cyclic prefix to training symbol
    for(int i = 0; i < 2; ++i){
        for(int j = 0; j < FFTSize + cyclic_prefix; ++j){
            signal_after_cp[i * (FFTSize + cyclic_prefix) + j] = TS_signal_data[0][(j + FFTSize - cyclic_prefix) % FFTSize].real;
            avg_TS += signal_after_cp[i * (FFTSize + cyclic_prefix) + j];          
        }
    }

    // Add cyclic prefix to signal data
    for(int i = 2; i < OFDM_symbol + 2; ++i){
        for(int j = 0; j < FFTSize + cyclic_prefix; ++j){
            signal_after_cp[i * (FFTSize + cyclic_prefix) + j] = signal_data[i - 2][(j + FFTSize - cyclic_prefix) % FFTSize].real;
            avg_amp += signal_after_cp[i * (FFTSize + cyclic_prefix) + j];
        }
    }

    // Calculate average amplitudes
    avg_TS /= 2 * (FFTSize + cyclic_prefix), avg_amp /= OFDM_symbol * (FFTSize + cyclic_prefix);
    // Adjust signal amplitude based on training symbol average amplitude
    for(int i = 0; i < 2 * (FFTSize + cyclic_prefix); ++i)
        signal_after_cp[i] *= avg_amp / avg_TS;
    
    for(int i = 0; i < OFDM_symbol; ++i)
        free(signal_data[i]);
    
    printf("success");
    free(signal_data);
    free(TS_signal_data[0]), free(TS_signal_data);

    char buffer[40] = {0};
    sprintf(buffer, "%d_QAM_%d_Gbaud_signal.txt", 1 << bits_per_symbol, bandwidth);
    FILE* file = fopen(buffer, "w");
    if(!file) {
        perror("Unable to open file");
        return 1;
    }

    for(int i = 0; i < (OFDM_symbol + 2) * (FFTSize + cyclic_prefix); ++i)
        fprintf(file, "%.9llf\n", signal_after_cp[i]);
    fclose(file);
    free(signal_after_cp);
    return 0;

}



