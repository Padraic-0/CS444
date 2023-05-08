#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "block.h"
#include "free.h"
#include "macros.h"


void set_free(unsigned char *block, int num, int set){
    int byte_num = num / 8;  // 8 bits per byte
    int bit_num = num % 8;

    if (set){
        block[byte_num] |= (1 << bit_num); //set to 1
    }else{
        block[byte_num] &= ~(1 << bit_num); //free to 0
    }

}
int find_low_clear_bit(unsigned char x)
{
    for (int i = 0; i < 8; i++)
        if (!(x & (1 << i)))
            return i;
    
    return -1;
}

int find_free(unsigned char *block){
    int index = 0;
    for (int i = 0; i < BLOCK_SIZE; i++){
        index = find_low_clear_bit(block[i]);
        if (index != -1){
            return (8 * i + index) ;
        }
    }

    return -1;

}