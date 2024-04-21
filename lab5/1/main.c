#include "utilities.h"
#include <stdbool.h>

#define MAX_THREADS 1000

MessageQueue* queue;
Semaphores* semaphores;
pthread_t threads[MAX_THREADS];
int threadCounter=0;
int producers=0;
int consumers=0;
bool allowance=true;

void* produce();
void* consume();
void printInfo();
void closeAllThreads();

int main(int argc, char* argv[]){
    srand(time(NULL));
    char option;
    int capacity=5;
    
    queue=(MessageQueue*)calloc(1,sizeof(MessageQueue));
    semaphores=(Semaphores*)calloc(1,sizeof(Semaphores));

    createQueue(queue,capacity);
    createSemaphores(semaphores,capacity);
    printf("p - producer\nc - consumer\nl - list info\nd - close threads\n+ - increase size of queue\n- - decrease size of queue\n");
    do{
        option=getchar();
        switch(option){
        case 'p':
            pthread_create(&threads[threadCounter++],NULL,produce,NULL);
            producers++;
            break;
        case 'c':
            pthread_create(&threads[threadCounter++],NULL,consume,NULL);
            consumers++;
            break;
        case 'l':
            printInfo();
            break;
        case 'd':
            closeAllThreads();
            break;
        case '+':
            if(queue->capacity==MAX_SIZE-1){
                printf("Max size is 1000\n");
                break;
            }
            sem_post(&semaphores->freeSpace);
            queue->capacity++;
            break;
        case '-':
            if(queue->capacity==1){
                printf("Capacity is already 1\n");
                break;
            }
            sem_wait(&semaphores->itemsToConsume);
            queue->capacity--;
            break;
        }
        rewind(stdin);
    }while(option!='q');

    pthread_mutex_destroy(&semaphores->mutex);

    exit(EXIT_SUCCESS);
}

void* produce(){
    while(allowance){
        sem_wait(&semaphores->freeSpace);
        pthread_mutex_lock(&semaphores->mutex);
        Message message=generateMessage();
        putMessage(queue, message);
        printf("Producer: ");
        printMessage(message);
        pthread_mutex_unlock(&semaphores->mutex);
        sem_post(&semaphores->itemsToConsume);
        sleep(3);
    }

    pthread_exit(NULL);
}

void* consume(){
    while(allowance){
        sem_wait(&semaphores->itemsToConsume);
        pthread_mutex_lock(&semaphores->mutex);
        Message message=getMessage(queue);
        printf("Consumer: ");
        printMessage(message);
        pthread_mutex_unlock(&semaphores->mutex);
        sem_post(&semaphores->freeSpace);
        sleep(3);
    }

    pthread_exit(NULL);
}


void printInfo(){
    pthread_mutex_lock(&semaphores->mutex);
    printMessageQueue(queue);
    printf("Producers: %d  Consumers: %d\n", producers, consumers);
    sleep(1);
    pthread_mutex_unlock(&semaphores->mutex);
}

void closeAllThreads(){
    pthread_mutex_lock(&semaphores->mutex);
    allowance=false;
    for(int i=0;i<threadCounter;i++){
        pthread_cancel(threads[i]);
        pthread_join(threads[i],NULL);
    }
    threadCounter=0;
    producers=0;
    consumers=0;
    allowance=true;
    pthread_mutex_unlock(&semaphores->mutex);
}

