
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <signal.h>



typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);



void sigint_handler_C(int signo) {
    printf("Do you want to exit this shell (y/n)? : ");
    r = getchar();
    if (r == '\n') r = getchar();
    while(r != 'n' && r != 'N' && r != 'y' && r != 'Y') {
        printf("invalid input, enter the choice(y/Y/n/N) again : ");
        r = getchar();
        if (r == '\n') r = getchar();
    }

    if(r == 'y' || r == 'y') printf("user chose yes");
}

void sigtstp_handler_Z(int signo) {
    printf("must use Ctl C \n");
}


int main (int argc, char *argv[]) {

    signal(SIGINT, sigint_handler_C);
    signal(SIGTSTP, sigint_handler_Z);
    while (1); 
}

