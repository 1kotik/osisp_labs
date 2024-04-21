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
#include <pthread.h>

#define MAX_SIZE 1000

typedef struct{
uint8_t type;
uint16_t hash;
uint8_t size;
uint8_t data[256];
}Message;

typedef struct{
    Message messages[MAX_SIZE];
    size_t capacity;
    int head;
    int tail;
    int producedCount;
    int consumedCount;
}MessageQueue;


Message generateMessage();
void createQueue(MessageQueue* queue, size_t capacity);
void putMessage(MessageQueue* queue, Message message);
Message getMessage(MessageQueue* queue);
void printMessage(Message message);
void printMessageQueue(MessageQueue* queue);

#endif