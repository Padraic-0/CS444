#include "image.h"
#include "ctest.h"
#include "stdio.h"
#include "free.h"
#include "inode.h"
#include "mkfs.h"
#include "block.h"
#include "macros.h"
#include "pack.h"
#include "ls.h"
#include "string.h"
#ifdef CTEST_ENABLE

int image_fd;

void test_image_open(void){
    image_fd = image_open("test.bin",0);
    CTEST_ASSERT(image_fd != -1, "image open");

    image_fd = image_open("test.bin",1);
    CTEST_ASSERT(image_fd != -1, "image truncate");
}

void test_image_close(void){
    CTEST_ASSERT(image_close() != -1, "Image closed");
}

void test_bread_bwrite(void){
    image_open("test.bin",1);

    unsigned char block[BLOCK_SIZE] = {0};
    block[0] = 1;
    block[2] = 2;
    bwrite(0,block);

    unsigned char read[BLOCK_SIZE] = {0};
    bread(0,read);

    CTEST_ASSERT(memcmp(block, read, BLOCK_SIZE) == 0, "Written block is read");
    image_close();
}

void test_set_free(void){
    image_open("test.bin",1);

    unsigned char block[BLOCK_SIZE] = {0};

    set_free(block, 0, 1);

    CTEST_ASSERT(((block[0] >> 0) & 1) == 1, "Bit set to 1");

    set_free(block, 0, 0);
    CTEST_ASSERT(((block[0] >> 0) & 1) == 0, "Bit set to 0");

    image_close();
}

void test_find_free(void){
    image_open("test.bin",1);

    unsigned char block[BLOCK_SIZE] = {0};

    set_free(block, 0, 1);
    CTEST_ASSERT(find_free(block) == 1, "Next free bit returned");

    set_free(block, 0, 0);
    CTEST_ASSERT(find_free(block) == 0, "First free bit returned");

    image_close();
}

void test_ialloc(){
    image_open("test.bin",1);

    struct inode *temp = ialloc();
    CTEST_ASSERT(temp->inode_num == 0, "Allocates first inode");

    temp = ialloc();
    CTEST_ASSERT(temp->inode_num == 1, "Allocates first inode");


    unsigned char fill_inode_map[BLOCK_SIZE] = {0};
    memset(fill_inode_map, 255, BLOCK_SIZE);
    bwrite(INODE_MAP, fill_inode_map);
    CTEST_ASSERT(ialloc() == NULL, "Full inode_map returns NULL");
    image_close();

}

void test_incore(){
    struct inode write;

    write.size = 23;
    write.owner_id = 24;
    write.permissions = 25;
    write.flags = 25;
    write.link_count = 0;
    write.block_ptr[0] = 1;
    write.inode_num = 5;

    put_incore(&write, write.inode_num);

    struct inode read = *find_incore(write.inode_num);
    CTEST_ASSERT(read.size == write.size, "Find incore returns the correct inode");

    write.ref_count = 0;
    put_incore(&write, write.inode_num);
    CTEST_ASSERT(find_incore(write.inode_num) == NULL, "Ref count 0 returns NULL");

    struct inode zero;
    zero.inode_num = 0;
    zero.size = 11;
    zero.ref_count = 0;
    put_incore(&zero, zero.inode_num);

    CTEST_ASSERT(find_incore_free()->size == 11, "First spot is open");

    zero.ref_count = 1;
    put_incore(&zero, zero.inode_num);

    struct inode one;
    one.inode_num = 1;
    one.size = 12;
    one.ref_count = 0;

    put_incore(&one, one.inode_num);
    CTEST_ASSERT(find_incore_free()->size == 12, "Second spot is open");
}

void test_alloc(){
    image_open("test.bin",1);
    
    int allocation_index = alloc();
    unsigned char block[BLOCK_SIZE] = {0};
    bread(FREE_BLOCK_MAP,block);

    int byte_num = allocation_index / 8;  // 8 bits per byte
    int bit_num = allocation_index % 8;

    CTEST_ASSERT(((block[byte_num] >> bit_num) & 1) == 1, "Bit set to 1");

    memset(block, 255, BLOCK_SIZE);
    bwrite(FREE_BLOCK_MAP, block);
    CTEST_ASSERT(alloc() == -1, "Full block map returns -1");


    image_close();
}

void test_read_write_inode(){
    image_open("test.bin",1);
    
    struct inode write;

    write.size = 23;
    write.owner_id = 24;
    write.permissions = 25;
    write.flags = 25;
    write.link_count = 0;
    write.block_ptr[0] = 1;
    write.inode_num = 5;
    
    write_inode(&write);
    
    struct inode read;
    read_inode(&read, write.inode_num);
    
    CTEST_ASSERT(read.size == write.size, "Read inode is same size as written");
    CTEST_ASSERT(read.owner_id == write.owner_id, "Read inode onwer is same and written");
    CTEST_ASSERT(read.block_ptr[0] == write.block_ptr[0], "Last field is correct");

    struct inode write2;
    write2.inode_num = 45;
    for (int i = 0; i < INODE_PTR_COUNT; i++){
        write2.block_ptr[i] = i;
    }
    write_inode(&write2);
    
    struct inode read2;
    read_inode(&read2, write2.inode_num);

    for (int i = 0; i < INODE_PTR_COUNT; i++){
        CTEST_ASSERT(read2.block_ptr[i] == write2.block_ptr[i], "Block ptrs are rw correctly");
    }

    image_close();
}

void test_iget_iput(){
    image_open("test.bin",1);

    struct inode write;

    write.size = 23;
    write.owner_id = 24;
    write.permissions = 25;
    write.flags = 25;
    write.link_count = 0;
    write.block_ptr[0] = 1;
    write.inode_num = 5;
    write.ref_count = 1;

    iput(&write);

    struct inode get = *iget(write.inode_num);
    CTEST_ASSERT(get.size == write.size, "iput and iget work together");

    write.inode_num = 34;
    write_inode(&write);

    get = *iget(34);
    CTEST_ASSERT(get.ref_count == 1, "find_incore returns NULL so new incore inode is populated from storage");
    CTEST_ASSERT(get.size == 23, "In storage inode is read out and put incore");


    image_close();
}

void test_mkfs(){
    image_open("test.bin",1);
    mkfs();

    unsigned char block[BLOCK_SIZE] = {0};
    bread(FREE_BLOCK_MAP, block);
    CTEST_ASSERT(block[0] == 0b11111111, "SPEC size is allocated plus root directory");
    CTEST_ASSERT(block[1] == 0b00000000, "Next byte has no allocation");

    image_close();
}

void test_ls(){
    image_open("test.bin",1);
    mkfs();

    ls();
    printf("Does above match:\n------------\n0 .\n0 ..\n------------\n");

    image_close();
}

void test_directory(){
    image_open("test.bin",1);
    mkfs();

    struct directory *test = directory_open(ROOT_INODE_NUM);
    struct inode *for_later = test->inode;

    CTEST_ASSERT(test->inode->inode_num == ROOT_INODE_NUM, "Directory opens correct inode");

    struct directory_entry ent;
    directory_get(test,&ent);

    CTEST_ASSERT(strcmp(ent.name,".") == 0, "Parent Directory is got");

    directory_get(test,&ent);
    CTEST_ASSERT(strcmp(ent.name,"..") == 0, "Current Directory is got");

    CTEST_ASSERT(directory_get(test,&ent) == -1, "No more directories");

    directory_close(test);
    
    CTEST_ASSERT(for_later->ref_count == 1, "Not sure if correct"); //mkfs made the 0 inode with ialloc setting ref_count to 1, then open directory called iget so now ref count is 2, dir_close then decreases refcount

    image_close();
}

void test_directory_make(){
    image_open("test.bin",1);
    mkfs();

    directory_make("/foo");
    ls();
    printf("Does above match:\n------------\n0 .\n0 ..\n1 foo\n------------\n");

    struct directory *test = directory_open(ROOT_INODE_NUM);
    struct directory_entry ent;
    directory_get(test,&ent);
    directory_get(test,&ent);
    directory_get(test,&ent);

    CTEST_ASSERT(strcmp(ent.name, "foo") == 0, "Directory sucessfully created");

    directory_close(test);

    image_close();
}

void test_namei(){
    image_open("test.bin",1);
    mkfs();

    CTEST_ASSERT(namei("/")->inode_num == ROOT_INODE_NUM, "Namei returns the root inode as of now");

    CTEST_ASSERT(namei("nothing") == NULL, "Namei returns NULL if not the root path");

    image_close();
}


int main(void){
    CTEST_VERBOSE(1);

    test_image_open();
    test_image_close();
    test_set_free();
    test_bread_bwrite();
    test_find_free();
    test_ialloc();
    test_incore();
    test_alloc();
    test_read_write_inode();
    test_iget_iput();
    test_mkfs();
    test_ls();
    test_directory();
    test_directory_make();
    test_namei();
    /*


















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
    image_close();

    image_open("test.bin",1);
    mkfs();
    struct inode write;
    write.size = 10;
    write.owner_id = 1;
    write.permissions = 2;
    write.flags = 3;
    write.link_count = 4;
    write.inode_num = 5;
    
    write_inode(&write);
    struct inode read;
    read_inode(&read,5);
    //print_inode(&read);
    CTEST_ASSERT(read.size == write.size, "Verify the written node is read out");
    CTEST_ASSERT(read.owner_id == write.owner_id, "Verify the written node is read out");
    CTEST_ASSERT(read.permissions == write.permissions, "Verify the written node is read out");
    CTEST_ASSERT(read.flags == write.flags, "Verify the written node is read out");
    CTEST_ASSERT(read.link_count == write.link_count, "Verify the written node is read out");

    CTEST_ASSERT(find_incore(5) == NULL, "Inode is in file but not incore");
    CTEST_ASSERT(read.ref_count == 0, "The in file inode 'read' has refcount 0");
    CTEST_ASSERT(iget(5)->ref_count == 1, "iget goes down find_incore == NULL path and sets ref_count = 1");
    CTEST_ASSERT(iget(5)->ref_count == 2, "iget goes down find_incore != NULL path and sets ref_count ++");

    struct inode inode_0 = *iget(0); //inode 0 will now how ref count of 3
    inode_0.flags = 25; //modifiy the incore inode value
    read_inode(&read,0); //Get the in file inode 0
    CTEST_ASSERT(read.flags != inode_0.flags && read.size == inode_0.size, "updated flags not on the infile inode");
    find_incore(0)->ref_count = 0; //set ref_count to 0 for iput to update
    inode_0.ref_count = 0;
    iput(&inode_0);
    read_inode(&read,0); //Get the in file inode 0
    CTEST_ASSERT(read.flags == inode_0.flags && read.size == inode_0.size, "updated flags on the infile inode");
    image_close();

    image_open("test.bin",1);
    mkfs();
    struct inode *inode_zero = ialloc();
    CTEST_ASSERT(inode_zero->inode_num == 1, "ialloc accounts of root dir");
    unsigned char inode_map[BLOCK_SIZE] = {0};
    bread(INODE_MAP,inode_map);
    CTEST_ASSERT(find_free(inode_map) == 2, "find free sees first 2 blocks");


    struct inode* root = iget(0);
    unsigned char root_block[BLOCK_SIZE] = {0};
    bread(root->block_ptr[0], root_block);
    printf("%d", root->block_ptr[0]);
    CTEST_ASSERT(root->block_ptr[0] == 6, "root directory has the 7th block");

    struct inode* incore_print;
    for (int i = 0; i < 10; i++){
        incore_print = find_incore(i);
        if (incore_print != NULL){
            print_inode(incore_print);
            printf("\n");
        }
    }
    
    image_close();

    image_open("test.bin",1);
    mkfs();
    ls();
    */
    CTEST_RESULTS();
    CTEST_EXIT();

    return 0;
}
#else
int main(void){
    printf("running good");
}
#endif