#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "image.h"
#include "macros.h"
#include "block.h"


int image_open(char *filename, int truncate){
    if(truncate){
        image_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
        return image_fd;
    }else{
        image_fd = open(filename, O_RDWR | O_CREAT, 0600);
        return image_fd;
    }
    return -1;
}

int image_close(void){
    return close(image_fd);
}