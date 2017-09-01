#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int main(int argc, char **argv){
    if (argc < 2){
        printf("No args\n");
    }else{
        printf("args = %s\n",argv[1]);
    }
    return E_PD_CONFIG_SUCCESS;
}
