#include "qam_mapping.h"
static QAM_symbol_t* hash_find(QAM_symbol_t **hash_table, int key){
    QAM_symbol_t *entry;
    HASH_FIND_INT(*hash_table, &key, entry);
    return entry;
}

static void hash_insert(QAM_symbol_t **hash_table, int key, int real, int image){
    QAM_symbol_t *entry = hash_find(hash_table, key);
    if(entry)
        return;
    entry = malloc(sizeof(QAM_symbol_t));
    entry->key = key;
    entry->real = real;
    entry->image = image;
    HASH_ADD_INT(*hash_table, key, entry);
}

static void hash_free(QAM_symbol_t **hash_table){
    QAM_symbol_t *entry = *hash_table, *temp;
    while(entry){
        temp = entry;
        entry = entry->hh.next;
        HASH_DEL(*hash_table, temp);
        free(temp);
    }
}

static void swap(int* a, int*b){
    if(*a == *b) return;
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

// generate QAM symbol and insert into hash table
static void generate_QAM_symbol(QAM_symbol_t **QAM_mapping_table, int bits_per_symbol, int *Gray_code_converter_table){
    
    if(bits_per_symbol <= 0) {
        fprintf(stderr, "Error: bits_per_symbol must be a positive integer\n");
        exit(EXIT_FAILURE);
    }
    
    int bits_per_side = bits_per_symbol >> 1;
    int mask = (1 << bits_per_side) - 1;
    int QAM_number = 1 << bits_per_symbol;
    int real, image;

    if(bits_per_symbol & 1) goto odd_number_case;

    for(int i = 0; i < QAM_number; ++i){
        real = (1 << bits_per_side) - 1 - 2 * Gray_code_converter_table[i >> bits_per_side];
        image = (1 << bits_per_side) - 1 - 2 * Gray_code_converter_table[i & mask];
        hash_insert(QAM_mapping_table, i, real, image);
    }

odd_number_case:

    if(bits_per_symbol == 1){

        for(int i = 0; i < QAM_number; ++i){
            real = 0;
            image = 1 - 2 * i;
            hash_insert(QAM_mapping_table, i, real, image);
        }

    }else if(bits_per_symbol == 3){

        for(int i = 0; i < QAM_number; ++i){

            image = 2 - 2 * Gray_code_converter_table[i & mask]; 
            real = 4 - 2 * Gray_code_converter_table[i >> bits_per_side];
            if(real == 4){
                real = image > 0 ? -3 : 3;
                swap(&image, &real);
            }
            hash_insert(QAM_mapping_table, i, real, image);
        }

    }else if(bits_per_symbol == 5){

        for(int i = 0; i < QAM_number; ++i){
  
            image = 3 - 2 * Gray_code_converter_table[i & mask]; 
            real = 7 - 2 * Gray_code_converter_table[i >> bits_per_side];

            if(abs(real) == 7){
                real = (real > 0) ? 5 : -5;                   
                swap(&real, &image);
            }
            hash_insert(QAM_mapping_table, i, real, image);
        }
    }
}

// generate signal in frequenct domain
void QAM_mapping(int *data, int bits_per_symbol, int *Gray_code_converter_table, int carrier_number, int OFDM_symbol, int FFT_size, complex_t ***QAM_data){

    QAM_symbol_t *QAM_mapping_table = NULL;
    generate_QAM_symbol(&QAM_mapping_table, bits_per_symbol, Gray_code_converter_table);
    /*  
        data_length (資料長度) = OFDM_symbol (OFDM符號數量) * carrier_number (子載波數量);
        由於資料的長度遠大於 QAM 符號的數量，我們使用哈希表來存取每一個格雷碼所對應的 QAM 符號。
        在進行 QAM 映射後，下一步是進行 IFFT，因此陣列的大小設定為 OFDM_symbol * FFT_size。
        每一個 OFDM 符號分配的點數為 FFT 的窗口大小，即 2 的 n 次方大小的 FFT_size。
        在頻域中沒有分配到 QAM 符號的點將設為零。
        由於時域信號只有實部，每分配一個 QAM 符號就需要在對應的負頻率位置產生共軛項。
    */

    complex_t **QAM_mapping_data = malloc(sizeof(complex_t*) * OFDM_symbol);

    for(int i = 0; i < OFDM_symbol; ++i){
        QAM_mapping_data[i] = malloc(sizeof(complex_t) * FFT_size);

        for(int j = 0; j < FFT_size; ++j){
            QAM_mapping_data[i][j].real = QAM_mapping_data[i][j].image = 0;
        }
    }

    /*
        由於時域信號只有實部，我們在陣列的後半部分添加共軛項，對應關係為第 j 項對第 FFT_size - j - 1 項。
        在傅立葉轉換時，第 FFT_size 項的旋轉因子為 1，因此第 FFT_size - j - 1 項可視為反向第 j 項，對應負頻率。
    */  

    QAM_symbol_t* entry;
    for(int i = 0; i < OFDM_symbol; ++i){
        for(int j = 0; j < carrier_number; ++j){
            entry = hash_find(&QAM_mapping_table, data[i * carrier_number + j]);
            QAM_mapping_data[i][j + 1].real = entry->real;
            QAM_mapping_data[i][j + 1].image = entry->image;
            QAM_mapping_data[i][FFT_size - j - 1] = conjugate(QAM_mapping_data[i][j + 1]);

        } 
    }
    
    hash_free(&QAM_mapping_table);
    *QAM_data = QAM_mapping_data;
}