#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "block.h"
#include "mkfs.h"
#include "macros.h"
#include "macros.h"
#include "inode.h"
#include "pack.h"
#include "string.h"

void write_dir(int index, unsigned char *block, int inode_num, char* file_name){
    int index_offset = index * DIRECTORY_SIZE;
    write_u16(block + index_offset, inode_num );
    strcpy((char *)(block + index_offset + sizeof(unsigned short)), file_name);
}

void mkfs(void){
    unsigned char buffer[BLOCK_SIZE*NUM_BLOCK] = {0};
    write(image_fd, buffer, BLOCK_SIZE*NUM_BLOCK);
    for(int i = 0; i < SPEC_SIZE; i++){
        if (alloc() == -1){
            perror("Failed to alloc spec");
        }
    }
    
    //root directory
    struct inode *root = ialloc();
    int root_block_num = alloc();
    unsigned char root_block[BLOCK_SIZE] = {0};

    write_dir(0,root_block, root->inode_num, ".");
    write_dir(1,root_block, root->inode_num, "..");

    bwrite(root_block_num, root_block);

    root->size = 64;
    root->link_count = 1;
    root->flags = 2;
    root->block_ptr[0] = root_block_num;
    //printf("\nwritting root inode: ref_Count: %d\n", root->ref_count);
    //fflush(stdout);
    iput(root);
}