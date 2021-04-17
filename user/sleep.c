#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(uint32 argc,char * argv[]){
    if( argc < 2 ){
        printf("Usage: sleep <NUM> \n");
        exit(0);
    }
    char c = argv[1][0];
    if( c < '0' || c > '9'){
        printf("Usage: sleep <NUM> \n");
        exit(0);
    }
    uint32 n = atoi(argv[1]);
    printf("sleep %d ticks\n",n);
    sleep(n);
    exit(0);
}
