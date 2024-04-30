#include <stdio.h>
#include <stdlib.h>
#include "uthash.h"
#include"my_complex.h"
typedef struct{
    int key;
    int real;
    int image;
    UT_hash_handle hh;
} QAM_symbol_t;

QAM_symbol_t* hash_find(QAM_symbol_t **hash_table, int key){
    QAM_symbol_t *pEntry = NULL;
    HASH_FIND_INT(*hash_table, &key, pEntry);
    return pEntry;
}

void hash_insert(QAM_symbol_t **hash_table, int key, int real, int image){
    QAM_symbol_t *pEntry = hash_find(hash_table, key);
    if(pEntry)
        return;
    pEntry = malloc(sizeof(QAM_symbol_t));
    pEntry->key = key;
    pEntry->real = real;
    pEntry->image = image;
    HASH_ADD_INT(*hash_table, key, pEntry);
}

void hash_free(QAM_symbol_t **hash_table){
    QAM_symbol_t *pEntry = *hash_table, *temp;
    while(pEntry){
        temp = pEntry;
        pEntry = pEntry->hh.next;
        HASH_DEL(*hash_table, temp);
        free(temp);
    }
}

void swap(int* a, int*b){
    if(*a == *b) return;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}
    /*
        以64QAM為例 假設一個數的格雷碼為 010 001  前三碼用於計算 星座圖的x座標後三碼用於計算y座標
        010 001 & mask  = 010 001 & 000 111 = 001  對應格雷碼的1
        010 001 >> bits_per_side         = 010  對應格雷碼的4
        所以星座圖的座標為 (-6 + 1 + 2 * 1) + j (-6 + 1 + 2 * 4) = -3 + 3j
    */
void generate_QAM_symbol(QAM_symbol_t **QAM_mapping_table, int bits_per_symbol, int *Gray_code_converter_table){
    int bits_per_side = bits_per_symbol >> 1;
    int mask = (1 << bits_per_side) - 1;
    int QAM_number = 1 << bits_per_symbol;
    int real, image;

    if( bits_per_symbol & 1) goto odd_number_case;

    for(int i = 0; i < QAM_number; ++i){
        real = (1 << bits_per_side) - 1 - 2 * Gray_code_converter_table[i >> bits_per_side];
        image = (1 << bits_per_side) - 1 - 2 * Gray_code_converter_table[i & mask];
        hash_insert(QAM_mapping_table, i, real, image);
    }

odd_number_case:

    int temp;
    if(bits_per_symbol == 1){

        for(int i = 0; i < QAM_number; ++i){
            real = 0;
            image = 1 - 2 * i;
            hash_insert(QAM_mapping_table, i, real, image);
        }

    }else if(bits_per_symbol == 3){

        for(int i = 0; i < QAM_number; ++i){

            temp = i >> bits_per_side;   
            image = (1 << bits_per_side) - 1 * Gray_code_converter_table[i & mask]; 

            if(temp == 0){
                real = image > 0 ? -3 : 3;
                swap(&image, &real);
            }else{
                real = (1 << bits_per_side + 1) - 2 * Gray_code_converter_table[i >> bits_per_side];
            }

            hash_insert(QAM_mapping_table, i, real, image);
        }

    }else if(bits_per_symbol == 5){

        for(int i = 0; i < QAM_number; ++i){

            temp = i >> bits_per_side;    
            image = (1 << bits_per_side) - 1 - 2 * Gray_code_converter_table[i & mask]; 

            if(temp == 0 || temp == (1 << bits_per_side)){

                real = (temp > 0)? bits_per_symbol : - bits_per_symbol;                   
                swap(&real, &image);

            }else{
                real = (1 << bits_per_side + 1) - 1 - 2 * Gray_code_converter_table[temp];
            }
            hash_insert(QAM_mapping_table, i, real, image);
        }
    }
}

complex_t** QAM_mapping(int *data, int bits_per_symbol, int *Gray_code_converter_table, int carrier_number, int OFDM_symbol, int FFT_size){

    QAM_symbol_t *QAM_mapping_table = NULL;
    generate_QAM_symbol(&QAM_mapping_table, bits_per_symbol, Gray_code_converter_table);
    /*  
        data_length (資料長度) = OFDM_symbol (OFDM_symbol 數量) * carrier_number (子載波數量);
        考量到資料的長度遠遠大於QAM symbol的數量，在這裡我弄了哈希表來存取每一個格雷碼所對應的值
        QAM_mapping後下一步就是IFFT反傅立葉轉換，因此陣列大小將設定為 OFDM_symbol * FFT_size
        每一個OFDM symbol 所分配的點數為FFT的窗口大小即 2^n FFTsize，頻域上沒有分配到QAM symbol的點做補零
        考量到時域上只有實部訊號，每分配一個QAM symbol就要在對應的負頻率位置產生共軛項
    */

    complex_t **QAM_mapping_data = malloc(sizeof(complex_t*) * OFDM_symbol);

    for(int i = 0; i < OFDM_symbol; ++i){
        QAM_mapping_data[i] = malloc(sizeof(complex_t) * FFT_size);

        for(int j = 0; j < FFT_size; ++j){
            QAM_mapping_data[i][j].real = QAM_mapping_data[i][j].image = 0;
            //補零
        }
    }

    QAM_symbol_t* pEntry;
   
    for(int i = 0; i < OFDM_symbol; ++i){
        for(int j = 0; j < carrier_number; ++j){
            pEntry = hash_find(&QAM_mapping_table, data[i * carrier_number + j]);
            QAM_mapping_data[i][j + 1].real = pEntry->real;
            QAM_mapping_data[i][j + 1].image = pEntry->image;
            QAM_mapping_data[i][FFT_size - j - 1] = conjugate(QAM_mapping_data[i][j + 1]);
            /*
                由於時域訊號只有實部，我們在陣列的後半部分添加共軛項，此對應關係為 第 i 項 對 第 FFT_size - i 項
                在傅立葉轉換時，第 FFT_size 項的旋轉因子為 1 因此 第 FFT_size - i項 可看做是反走 i項 也就是對應負頻率
            */
        } 
    }
    
    hash_free(&QAM_mapping_table);
    //釋放哈希表
    return QAM_mapping_data;
}