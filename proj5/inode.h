#ifndef INODE_H
#define INODE_H

#define INODE_PTR_COUNT 16
#define MAX_SYS_OPEN_FILES 64

struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

struct inode *ialloc(void);
void print_inode_map(void);
struct inode *find_incore_free(void);
struct inode *find_incore(unsigned int inode_num);
void put_incore(struct inode* inode, int index);

void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
int get_block_offset_bytes(int inode_num);
void print_inode(struct inode *in);

struct inode *iget(int inode_num);
void iput(struct inode *in);
#endif