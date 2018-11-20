#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char **argv)
{
	int log;
	if(log = open("/proc/syslog", O_RDWR)==0){
		exit();
	}
	int i;
	//for(i=1;i<argc;i++){
	    write(log,argv[1],strlen(argv[1]));
	//}
	close(log);
	exit();
}
