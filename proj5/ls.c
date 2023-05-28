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
#include "ls.h"

void ls(void){
    struct directory *dir;
    struct directory_entry ent;

    dir = directory_open(0);

    while (directory_get(dir, &ent) != -1){
        printf("%d %s\n", ent.inode_num, ent.name);
    }
    directory_close(dir);
}