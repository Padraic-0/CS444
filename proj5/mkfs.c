#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "block.h"
#include "mkfs.h"
#include "macros.h"
#include "macros.h"


void mkfs(void){
    unsigned char buffer[BLOCK_SIZE*NUM_BLOCK] = {0};
    write(image_fd, buffer, BLOCK_SIZE*NUM_BLOCK);
    for(int i = 0; i < SPEC_SIZE; i++)
        alloc();
}