#include "types.h"
#include "user.h"



int main(int argc, char *argv[]){
    if (argc != 3) {
        printf(1,"divide: usage divide x y \n");}
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int re1, re2 ,re3;  //error happended when define float variable
    re1 = x/y;
    re2 = (10*x/y)%10;
    re3 = (100*x/y)%10;
    

    printf(1, "%s / %s is %d", argv[1],argv[2],re1);
    if ((re2==0)&&(re3==0))
        printf(1,"\n");
    else if (re3==0)
        printf(1,".%d\n",re2);
    else
        printf(1,".%d%d\n",re2,re3);
    exit();
}
