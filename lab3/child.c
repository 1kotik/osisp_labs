#include "child.h"

typedef struct stat{
    int count00;
    int count01;
    int count10;
    int count11;
}counter;

typedef struct pair{
    int a;
    int b;
}memory;

bool print;
bool alarmEnded;
bool signalCheck;
memory pair={0,0};
counter stat={0,0,0,0};

int main(int argc,char*argv[]){

    int pid=(int)getpid();
    int ppid=(int)getppid();
    printf("Name: %s PID: %d PPID: %d\n", argv[0],pid,ppid);
    signal(SIGALRM, alarmHandler);
    signal(SIGUSR1, userHandler);
    signal(SIGUSR2, userHandler);

    int counter=100;
    memory pairs[2]={{0,0},{1,1}};
    bool flag=false;
    while(true){
        while(counter--){
            ualarm(10000,0);
            alarmEnded=false;
            while(!alarmEnded){
                flag=!flag;
                pair.a=flag;
                pair.b=flag;
            }
        }
        if(print){
            printf("Name: %s PID: %d PPID: %d\n", argv[0],pid,ppid);
            printf("00: %d  01: %d  10: %d  11: %d\n", stat.count00, stat.count01, stat.count10, stat.count11);
        }
        counter=100;
        if (getppid() != ppid) exit(EXIT_SUCCESS);
    }
    exit(EXIT_SUCCESS);
}

void userHandler(int signal){
    if(signal==SIGUSR1){
        print=true;
        signalCheck=true;
    }
    else if(signal==SIGUSR2){
        print=false;
        signalCheck=true;
    }
}

void alarmHandler(int signal){
    alarmEnded=true;
    if(signal==SIGALRM){
        if(pair.a==pair.b){
            if(pair.a==0) stat.count00++;
            else stat.count11++;
        }
        else{
            if(pair.a==0) stat.count01++;
            else stat.count10++;
        }
    }
}



