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
    int bandwidth      = 1;
    int sampling_rate  = 64;
    int bits_per_symbol= 4;
    int shift          = FFTSize / 4 - 2;

    int carrier_number = (bandwidth * FFTSize) / sampling_rate;

    int OFDM_symbol    = 256; 
    int cyclic_prefix  = FFTSize / 32; //以3%的冗餘做為cp使用以避免ISI等多路徑干擾
    int data_length    = carrier_number * OFDM_symbol;
    

    //PRBS data generation  formula: PRBS31 = X^31 + X^28 + 1;

    int* PRBS_data_array = PRBS_generate(data_length, bits_per_symbol); 


    //PRBS data to Gray code

    for(int i = 0; i < data_length; ++i){
        PRBS_data_array[i] = GRAY_CODE(PRBS_data_array[i]);
    } 
    
    int table_size = 1 << (bits_per_symbol / 2 + bits_per_symbol % 2);
    int* Gray_code_converter_table = malloc(sizeof(int) * (table_size));
    
    for(int i = 0; i < table_size; ++i){
        Gray_code_converter_table[GRAY_CODE(i)] = i;
    }

    complex_t** signal_data = QAM_mapping(PRBS_data_array, bits_per_symbol, Gray_code_converter_table
                                        , carrier_number, OFDM_symbol, FFTSize);
    //signal_data 為經過 QAM_mapping 的頻域訊號
    free(Gray_code_converter_table), free(PRBS_data_array);
    //釋放用不到的記憶體空間

    for(int i = 0; i < OFDM_symbol; ++i){
        ifft(&signal_data[i], FFTSize_log2);
        //此時已把訊號由頻域轉回時域
    }
    double* signal_after_cp = malloc(sizeof(double) * OFDM_symbol * (FFTSize + cyclic_prefix));

    for(int i = 0; i < OFDM_symbol; ++i){
        for(int j = 0; j < FFTSize + cyclic_prefix; ++j){
            signal_after_cp[i * (FFTSize + cyclic_prefix) + j] = signal_data[i][j % FFTSize].real;
            //加入循環前綴 cp
        }
    }
    
    for(int i = 0; i < OFDM_symbol; ++i){
        free(signal_data[i]);
    }
    free(signal_data);
    return 0;

}




