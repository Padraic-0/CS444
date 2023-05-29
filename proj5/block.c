#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "image.h"
#include "free.h"
#include "block.h"
#include "macros.h"


unsigned char *bread(int block_num, unsigned char *block){
    int buffer = BLOCK_SIZE * block_num;
    lseek(image_fd, buffer, SEEK_SET);

    read(image_fd, block, BLOCK_SIZE);

    return block;
}

void bwrite(int block_num, unsigned char *block){
    int buffer = BLOCK_SIZE * block_num;
    lseek(image_fd, buffer, SEEK_SET);

    write(image_fd, block, BLOCK_SIZE);
}

int alloc(void){
    unsigned char data_map[BLOCK_SIZE] = {0};
    bread(FREE_BLOCK_MAP,data_map);

    int free_data = find_free(data_map);

    if (free_data != -1){
        set_free(data_map, free_data, 1);
        bwrite(FREE_BLOCK_MAP, data_map);
    }
    
    return free_data;
}