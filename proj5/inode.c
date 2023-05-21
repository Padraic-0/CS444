#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "image.h"
#include "free.h"
#include "inode.h"
#include "block.h"
#include "macros.h"
#include "pack.h"

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};
int next_free = 0;

struct inode *ialloc(void){
    unsigned char inode_map[BLOCK_SIZE] = {0};
    bread(INODE_MAP,inode_map);
    int free_inode = find_free(inode_map);
    if (free_inode != -1){
        set_free(inode_map,free_inode, 1);
        bwrite(INODE_MAP,inode_map);
        struct inode *temp = iget(free_inode);
        if (temp == NULL){
            return NULL;
        }
        temp->size = 0;
        temp->owner_id = 0;
        temp->flags = 0;
        temp->inode_num = free_inode;
        write_inode(temp);
        return temp;
    }
    return NULL;

}

void put_incore(struct inode* inode, int index){
    incore[index] = *inode;
}

struct inode *find_incore_free(void){
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++){
        if (incore[i].ref_count == 0)
            return &incore[i];
    }
    return NULL;
}

struct inode *find_incore(unsigned int inode_num){
    for (int i = 0; i < MAX_SYS_OPEN_FILES; i++){
        if (incore[i].inode_num == inode_num && incore[i].ref_count != 0){
            return &incore[i];
        }
    }
    return NULL;

}

int get_block_offset_bytes(int inode_num){
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    return block_num;
}

void read_inode(struct inode *in, int inode_num){
    int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char temp[BLOCK_SIZE] = {0};
    bread(block_num,temp);
    in->size=read_u32(temp + block_offset_bytes);
    in->owner_id=read_u16(temp + block_offset_bytes+4);
    in->permissions =read_u8(temp + block_offset_bytes+6);
    in->flags =read_u8(temp + block_offset_bytes+7);
    in->link_count=read_u8(temp + block_offset_bytes+8);
    int nine = 9;
    for (int i = 0; i < INODE_PTR_COUNT; i++){
       in->block_ptr[i] =read_u16(temp + block_offset_bytes+nine+(2*i));
    }

}

void write_inode(struct inode *in){
    int block_num = in->inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
    int block_offset = in->inode_num % INODES_PER_BLOCK;
    int block_offset_bytes = block_offset * INODE_SIZE;
    unsigned char temp[BLOCK_SIZE] = {0};
    bread(block_num,temp);
    write_u32(temp + block_offset_bytes, in->size);
    write_u16(temp + block_offset_bytes+4,in->owner_id);
    write_u8(temp + block_offset_bytes+6, in->permissions);
    write_u8(temp + block_offset_bytes+7,in->flags);
    write_u8(temp + block_offset_bytes+8,in->link_count);
    int nine = 9;
    for (int i = 0; i < INODE_PTR_COUNT; i++){
        write_u16(temp + block_offset_bytes+nine+(2*i),in->block_ptr[i]);
    }
    bwrite(block_num,temp);
    
}

void print_inode_map(void){
    unsigned char inode_map[BLOCK_SIZE] = {0};
    bread(INODE_MAP, inode_map);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 7; j++) {
            printf("%u", (inode_map[i] >> j) & 1);
        }
        printf("\n");
    }
    
}

void print_inode(struct inode *in) {
    printf("Size: %d\n", in->size);
    printf("Owner ID: %d\n", in->owner_id);
    printf("Permissions: %d\n", in->permissions);
    printf("Flags: %d\n", in->flags);
    printf("Link Count: %d\n", in->link_count);
    printf("Ref Count: %d\n", in->ref_count);
}

struct inode *iget(int inode_num){

    struct inode *temp = find_incore(inode_num);
    if (temp != NULL){
        temp->ref_count ++;
        return temp;
    }else{
       temp = find_incore_free();
       if(temp == NULL){
        return NULL;
       }
       read_inode(temp,inode_num);
       temp->ref_count = 1;
       temp->inode_num = inode_num;
       return temp;
    }
}

void iput(struct inode *in){
    if (in->ref_count == 0){
        write_inode(in);
    }else{
        in->ref_count --;
    }
    
}

/*
int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
int block_offset = inode_num % INODES_PER_BLOCK;
int block_offset_bytes = block_offset * INODE_SIZE;
*/