#ifndef UTIL_H_
#define UTIL_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct{
uint8_t type;
uint16_t hash;
uint8_t size;
uint8_t data[256];
}Message;

typedef struct{
    Message* messages;
    size_t capacity;
    int head;
    int tail;
    int producedCount;
    int consumedCount;
}MessageQueue;

typedef struct{
    sem_t freeSpace;
    sem_t itemsToConsume;
    sem_t mutex;
}Semaphores;

Message generateMessage();
void createQueue(MessageQueue* queue, size_t capacity);
void putMessage(MessageQueue* queue, Message message);
Message getMessage(MessageQueue* queue);
void printMessage(Message message);
void printMessageQueue(MessageQueue* queue);
void createSemaphores(Semaphores* semaphores, size_t queueCapacity);
void createSharedMemory(int* queueID, int* semaphoresID, int* dataID, size_t capacity);
void detachSharedMemory(void* ptr);
pid_t **getChildren();
char *getNameByPid(pid_t pid);
pid_t getPidByName(const char *process_name);
void freeChildren(pid_t **children);
void closeAllChildren();
void closeLastProcess();

#endif