#include "types.h"
#include "user.h"

int str2int(char str_x[]){
	int x;
	if (str_x[0] == '-') 
		x = - atoi(&(str_x[1]));
	else
		x = atoi(str_x);
	return x;
}



int main(int argc, char *argv[]){
    if (argc != 3) {
        printf(1,"divide: usage divide x y \n");
		exit();
	}
    int x = str2int(argv[1]);
    int y = str2int(argv[2]);
	if (y==0){
		printf(1,"divide: cannot divide by ZERO!!\n");
		exit();
	}

	int nega;
	nega = 0;
    int re1, re2 ,re3;  //error happended when define float variable
	if (x*y<0)
		nega = 1;
	x = x<0? -x:x;
	y = y<0? -y:y;
	re1 = x/y;
    re2 = (10*x/y)%10;
    re3 = (100*x/y)%10;
    

    printf(1, "%s / %s is ", argv[1],argv[2]);
	if (nega)
		printf(1,"-");
	printf(1,"%d",re1);
    if ((re2==0)&&(re3==0))
        printf(1,"\n");
    else if (re3==0)
        printf(1,".%d\n",re2);
    else
        printf(1,".%d%d\n",re2,re3);
    exit();
}
