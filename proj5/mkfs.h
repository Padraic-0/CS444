#ifndef MKFS_H
#define MKFS_H


void mkfs(void);
void write_dir(int index, unsigned char *block, int inode_num, char* file_name);
#endif