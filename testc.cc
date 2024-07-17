#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

typedef unsigned int (*sleep_func)(unsigned int seconds);


unsigned int sleep(unsigned int seconds)
{
    sleep_func sleep_f;
    sleep_f =(sleep_func) dlsym(RTLD_NEXT,"sleep");
    printf("/////////////////////\n");
    sleep_f(5);
    printf("/////////////////////\n");
    return 1;
}

int main(){
    sleep(2);
}
