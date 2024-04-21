#include "utilities.h"
int initializeSharedMemory(int projID);

MessageQueue* queue;
Semaphores* semaphores;
int producers=0;
int consumers=0;
int allowance=1;

void produce();
void consume();
void createChildProcess(int queueID, int semaphoresID, int dataID, void (*action)(void));
void printInfo();
void handler(int signal);

int main(int argc, char* argv[]){
    srand(time(NULL));
    char option;
    int capacity=5;
    int queueID;
    int semaphoresID;
    int dataID;

    createSharedMemory(&queueID, &semaphoresID, &dataID, capacity);
    queue=(MessageQueue*)shmat(queueID,NULL,0);
    semaphores=(Semaphores*)shmat(semaphoresID,NULL,0);
    queue->messages=(Message*)shmat(dataID,NULL,0);
    createQueue(queue,capacity);
    createSemaphores(semaphores,capacity);
    printf("p - producer\nc - consumer\nl - list info\n- - kill children\n");
    do{
        option=getchar();
        switch(option){
        case 'p':
            createChildProcess(queueID, semaphoresID, dataID, produce);
            producers++;
            break;
        case 'c':
            createChildProcess(queueID, semaphoresID, dataID, consume);
            consumers++;
            break;
        case 'l':
            printInfo();
            break;
        case '-':
            closeAllChildren();
            producers=0;
            consumers=0;
            break;
        }
        rewind(stdin);
    }while(option!='q');

    closeAllChildren();

    detachSharedMemory(queue->messages);
    detachSharedMemory(queue);
    detachSharedMemory(semaphores);


    shmctl(queueID, IPC_RMID, 0);
    shmctl(semaphoresID, IPC_RMID, 0);
    shmctl(dataID, IPC_RMID, 0);

    exit(EXIT_SUCCESS);
}

void produce(){
    while(allowance){
        sem_wait(&semaphores->freeSpace);
        sem_wait(&semaphores->mutex);

        Message message=generateMessage();
        putMessage(queue, message);
        printf("Producer: ");
        printMessage(message);

        sem_post(&semaphores->mutex);
        sem_post(&semaphores->itemsToConsume);

        sleep(3);
    }
}

void consume(){
    while(allowance){
        sem_wait(&semaphores->itemsToConsume);
        sem_wait(&semaphores->mutex);

        Message message=getMessage(queue);
        printf("Consumer: ");
        printMessage(message);

        sem_post(&semaphores->mutex);
        sem_post(&semaphores->freeSpace);

        sleep(3);
    }
}

void createChildProcess(int queueID, int semaphoresID, int dataID, void (*action)(void)){
    pid_t pid=fork();
    if(pid==-1){
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    if(pid==0){
        signal(SIGUSR1, handler);

        queue=(MessageQueue*)shmat(queueID,NULL,0);
        semaphores=(Semaphores*)shmat(semaphoresID,NULL,0);
        queue->messages=(Message*)shmat(dataID,NULL,0);

        action();

        detachSharedMemory(queue->messages);
        detachSharedMemory(queue);
        detachSharedMemory(semaphores);
        exit(EXIT_SUCCESS);
    }
}

void printInfo(){
    sem_wait(&semaphores->mutex);
    printMessageQueue(queue);
    printf("Producers: %d  Consumers: %d\n", producers, consumers);
    sleep(1);
    sem_post(&semaphores->mutex);
}

void handler(int signal){
    if(signal==SIGUSR1){
        allowance=0;
    }
}

