#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "image.h"
#include "free.h"
#include "inode.h"
#include "block.h"
#include "macros.h"


int ialloc(void){
    unsigned char inode_map[BLOCK_SIZE] = {0};
    bread(INODE_MAP,inode_map);
    int free_inode = find_free(inode_map);
    set_free(inode_map, free_inode, 1);
    bwrite(INODE_MAP, inode_map);
    return -1;

}

