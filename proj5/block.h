#ifndef BLOCK_H
#define BLOCK_H


unsigned char *bread(int block_num, unsigned char *block);
void bwrite(int block_num, unsigned char *block);
int alloc(void);

extern int image_fd;

#endif