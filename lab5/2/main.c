#include "utilities.h"
#include <stdbool.h>

#define MAX_THREADS 1000

MessageQueue* queue;
pthread_mutex_t mutex;
pthread_cond_t condConsume;
pthread_cond_t condProduce;
pthread_t threads[MAX_THREADS];
int threadCounter=0;
int producers=0;
int consumers=0;
int storage=0;
int minStorage=0;
int maxStorage=5;
bool allowance=true;

void* produce();
void* consume();
void printInfo();
void closeAllThreads();

int main(int argc, char* argv[]){
    srand(time(NULL));
    char option;
    int capacity=5;

    pthread_cond_init(&condConsume,NULL);
    pthread_cond_init(&condProduce,NULL);
    pthread_mutex_init(&mutex,NULL);
    
    queue=(MessageQueue*)calloc(1,sizeof(MessageQueue));

    createQueue(queue,capacity);
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
            maxStorage++;
            queue->capacity++;
            break;
        case '-':
            if(queue->capacity==1){
                printf("Capacity is already 1\n");
                break;
            }
            maxStorage--;
            queue->capacity--;
            break;
        }
        rewind(stdin);
    }while(option!='q');

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condConsume);
    pthread_cond_destroy(&condProduce);

    exit(EXIT_SUCCESS);
}

void* produce(){
    while(allowance){
        pthread_mutex_lock(&mutex);
        if(storage==maxStorage){
            pthread_cond_wait(&condProduce,&mutex);
            pthread_cond_broadcast(&condConsume);
        }
        Message message=generateMessage();
        putMessage(queue, message);
        printf("Producer: ");
        printMessage(message);
        storage++;
        pthread_mutex_unlock(&mutex);
        sleep(3);
    }

    pthread_exit(NULL);
}

void* consume(){
    while(allowance){
        pthread_mutex_lock(&mutex);
        if(storage==0){
            pthread_cond_wait(&condConsume,&mutex);
            pthread_cond_broadcast(&condProduce);
        }
        Message message=getMessage(queue);
        printf("Consumer: ");
        printMessage(message);
        storage--;
        pthread_mutex_unlock(&mutex);
        sleep(3);
    }

    pthread_exit(NULL);
}


void printInfo(){
    pthread_mutex_lock(&mutex);
    printMessageQueue(queue);
    printf("Producers: %d  Consumers: %d\n", producers, consumers);
    sleep(1);
    pthread_mutex_unlock(&mutex);
}

void closeAllThreads(){
    pthread_mutex_lock(&mutex);
    allowance=false;
    for(int i=0;i<threadCounter;i++){
        pthread_cancel(threads[i]);
        pthread_join(threads[i],NULL);
    }
    threadCounter=0;
    producers=0;
    consumers=0;
    allowance=true;
    pthread_mutex_unlock(&mutex);
}

