#include<stdio.h>
#include<stdlib.h>   

int* PRBS_generate(int data_length, int bits_per_symbol){
    //PRBS 31 = X(31) + X (28) + 1 把第31位與28位XOR做為新位元
    int mask  = (1 << bits_per_symbol) - 1;
    int start = 2147683647;
    int new_bit;
    int* PRBS_data_array = (int*)calloc(data_length, sizeof(int));

    for(int i = 0; i < data_length; ++i){
        //用十進位儲存方便後面Gray code轉換
        PRBS_data_array[i] = start & mask;
        
        for(int j = 1; j <= bits_per_symbol; ++j){
            new_bit = (start >> 31 & 1) ^ (start >> 28 & 1);
            start = (start << 1) | (new_bit);
        }
        //用二進值每一個空間都存0或1也可但很浪費運算時間
    }
    return PRBS_data_array;
}