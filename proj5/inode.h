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

struct directory {
    struct inode *inode;
    unsigned int offset;
};

struct directory_entry {
    unsigned int inode_num;
    char name[16];
};

void directory_close(struct directory *dir);
struct directory *directory_open(int inode_num);
int directory_get(struct directory *dir, struct directory_entry *ent);

struct inode *ialloc(void);

struct inode *find_incore_free(void);
struct inode *find_incore(unsigned int inode_num);

void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);

struct inode *iget(int inode_num);
void iput(struct inode *in);

//test helpers
void print_inode_map(void);
void put_incore(struct inode* inode, int index);
void print_inode(struct inode *in);
int get_block_offset_bytes(int inode_num);

#endif