#include "image.h"
#include "ctest.h"
#include "stdio.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"
#include "block.h"
#include "macros.h"
#include "pack.h"
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
    bwrite(0,wipe_file);
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
    bwrite(INODE_MAP,wipe_file);
    

    struct inode test;
    test.ref_count = 0;
    test.inode_num = 1;
    put_incore(&test, 0);
    CTEST_ASSERT(find_incore(1) == NULL, "ref_count 0 returns null");

    struct inode test2;
    test2.ref_count = 1;
    test2.inode_num = 2;
    put_incore(&test2, 1);
    CTEST_ASSERT(find_incore(2)->inode_num == 2, "incore array holds values");

    //incore[1,2] current incore arrray
    CTEST_ASSERT(find_incore_free()->inode_num == 1 , "First inode is free because ref_Count == 0");
    image_close();

    image_open("test.bin",1);
    mkfs();
    struct inode write;
    write.size = 10;
    write.owner_id = 1;
    write.permissions = 2;
    write.flags = 3;
    write.link_count = 4;
    write.inode_num = 0;
    
    write_inode(&write);
    struct inode read;
    read_inode(&read,0);
    //print_inode(&read);
    CTEST_ASSERT(read.size == write.size, "Verify the written node is read out");
    CTEST_ASSERT(read.owner_id == write.owner_id, "Verify the written node is read out");
    CTEST_ASSERT(read.permissions == write.permissions, "Verify the written node is read out");
    CTEST_ASSERT(read.flags == write.flags, "Verify the written node is read out");
    CTEST_ASSERT(read.link_count == write.link_count, "Verify the written node is read out");

    CTEST_ASSERT(find_incore(0) == NULL, "Inode is in file but not incore");
    CTEST_ASSERT(read.ref_count == 0, "The in file inode 'read' has refcount 0");
    CTEST_ASSERT(iget(0)->ref_count == 1, "iget goes down find_incore == NULL path and sets ref_count = 1");
    CTEST_ASSERT(iget(0)->ref_count == 2, "iget goes down find_incore != NULL path and sets ref_count ++");

    struct inode inode_0 = *iget(0); //inode 0 will now how ref count of 3
    inode_0.flags = 25; //modifiy the incore inode value
    read_inode(&read,0); //Get the in file inode 0
    CTEST_ASSERT(read.flags != inode_0.flags && read.size == inode_0.size, "The same inode but the updated flags are not seen on the infile inode");
    find_incore(0)->ref_count = 0; //set ref_count to 0 for iput to update
    inode_0.ref_count = 0;
    iput(&inode_0);
    read_inode(&read,0); //Get the in file inode 0
    CTEST_ASSERT(read.flags == inode_0.flags && read.size == inode_0.size, "The same inode but the updated flags are not seen on the infile inode");
    image_close();

    image_open("test.bin",1);
    mkfs();
    struct inode *inode_zero = ialloc();
    CTEST_ASSERT(inode_zero->inode_num == 0, "ialloc sets allocated inode number to the first free inode");
    unsigned char inode_map[BLOCK_SIZE] = {0};
    bread(INODE_MAP,inode_map);
    CTEST_ASSERT(find_free(inode_map) == 1, "ialloc has set first inode bit to inuse");
    
    image_close();

    CTEST_RESULTS();
    CTEST_EXIT();

    return 0;
}
#else
int main(void){
    printf("running good");
}
#endif