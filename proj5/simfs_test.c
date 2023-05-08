#include "image.h"
#include "ctest.h"
#include "stdio.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"
#include "block.h"
#include "macros.h"
#ifdef CTEST_ENABLE

int image_fd;

int main(void){
    CTEST_VERBOSE(1);
    unsigned char wipe_file[BLOCK_SIZE] = {0};

    CTEST_ASSERT(image_open("test.bin",0) != -1, "testing file is opened");
    CTEST_ASSERT(image_close() != -1, "testing file is closed");
    image_close();


    image_open("test.bin",0);
    unsigned char block[BLOCK_SIZE] = {0};
    set_free(block, 0, 1);
    CTEST_ASSERT(((block[0] >> 0) & 1) == 1, "testing bit can be set");


    set_free(block, 0, 0);
    CTEST_ASSERT(((block[0] >> 0) & 1) == 0, "testing bit can be freed");

    set_free(block, 0, 1);
    CTEST_ASSERT(find_free(block) == 1, "testing the block# with free bit is returned");
    CTEST_ASSERT(find_low_clear_bit(block[0]) == 1, "testing that the second bit is the free one");
    set_free(block, 0, 0);  //undo fill

    for (int i = 0; i < 8; i++){
        set_free(block, i, 1);  //fill the first byte
    }
    CTEST_ASSERT(find_free(block) == 8, "testing that after first byte is filled the second byte first bit is returned");
    for (int i = 0; i < 8; i++){
        set_free(block, i, 0);  //undo fill the first byte
    }


    set_free(block,1,1);
    set_free(block,0,1);
    bwrite(0,block);    //1100 0000
    unsigned char block2[BLOCK_SIZE] = {0};
    bread(0,block2);    //1100 0000
    CTEST_ASSERT(((block2[0] >> 0) & 1) && ((block2[0] >> 1) & 1)  == 1, "testing write and read works");
    image_close();


    image_open("test.bin",0);
    unsigned char block3[BLOCK_SIZE] = {0};
    bread(0,block3);
    CTEST_ASSERT(((block3[0] >> 0) & 1) && ((block3[0] >> 1) & 1)  == 1, "testing write and read saves to file after close");
    bread(0,wipe_file);
    image_close();


    image_open("test.bin",0);
    mkfs();
    unsigned char block4[BLOCK_SIZE] = {0};
    bread(FREE_BLOCK_MAP,block4);
    for (int j = 0; j < SPEC_SIZE; j++)
        CTEST_ASSERT((((block4[0]) >> j) & 1) == 1, "bit map is filled");


    unsigned char inode_block[BLOCK_SIZE] = {0};
    ialloc();
    bread(INODE_MAP,inode_block);
    CTEST_ASSERT((((inode_block[0]) >> 0) & 1) == 1, "inode map is filled");
    bread(INODE_MAP,wipe_file);


    CTEST_RESULTS();
    CTEST_EXIT();

    return 0;
}
#else
int main(void){
    printf("running good");
}
#endif