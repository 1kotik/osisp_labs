#include "utilities.h"
int initializeSharedMemory(int projID);

MessageQueue* queue;
Semaphores* semaphores;
int producers=0;
int consumers=0;

void produce();
void consume();
void createChildProcess(int queueID, int semaphoresID, int dataID, void (*action)(void));
void printInfo();

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

    shmctl(queueID, IPC_RMID, 0);
    shmctl(semaphoresID, IPC_RMID, 0);
    shmctl(dataID, IPC_RMID, 0);

    detachSharedMemory(queue->messages);
    detachSharedMemory(queue);
    detachSharedMemory(semaphores);

    exit(EXIT_SUCCESS);
}

void produce(){
    while(1){
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
    while(1){
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

int initializeSharedMemory(int projID){
    key_t queue_key = ftok(".", projID);
    if (queue_key == -1) {
        perror("ftok");
        exit(1);
    }
    int shmid = shmget(queue_key, sizeof(Semaphores), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget error");
        exit(1);
    }
    return shmid;
}
