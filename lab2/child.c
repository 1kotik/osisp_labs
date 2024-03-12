#include "utilities.h"

int main(int argc, char** argv, char** envp){
    if(argc != 3){
        printf("Need argv and envp");
        exit(EXIT_FAILURE);
    }

    pid_t pid=getpid();
    pid_t ppid=getppid();
    extern char** environ;

    printf("NAME: %s\nPID: %d\nPPID: %d\n", argv[0], (int)pid, (int)ppid);
    printf("%c\n",argv[2][0]);
    switch(argv[2][0]){
    case '+':
        getEnvironmnetByChild(argv[1], NULL);
        break;
    case '*':
        getEnvironmnetByChild(argv[1], envp);
        break;
    case '&':
        getEnvironmnetByChild(argv[1], environ);
        break;
    }

    exit(EXIT_SUCCESS);
}

