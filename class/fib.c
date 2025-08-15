#include <stdio.h>
#include <stdlib.h>

int fib(int x, int y){
    printf("%d\n", x);c// print 0
    int r = x; //r = 0
    x = x + y; //x = 1
    y = r; // y = 0
    fib(x,y); // 1,0
    if(y > 50){
        return 0;
    }

}

int main(void){

    fib(0,1);
    return 0;


}