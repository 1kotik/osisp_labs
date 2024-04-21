#include "utilities.h"

Message generateMessage(){
    Message message;
    message.hash=0;
    message.size=rand()%257;

    for(int i=0;i<message.size;i++){
        message.data[i]=rand()%257;
        message.hash^=message.data[i];
    }

    if(message.size==256) message.size=0;
    
    return message;
}

void createQueue(MessageQueue* queue, size_t capacity){
    queue->capacity=capacity;
    queue->head=capacity-1;
    queue->tail=capacity-1;
    queue->consumedCount=0;
    queue->producedCount=0;
}

void putMessage(MessageQueue* queue, Message message){
    if(queue->producedCount-queue->consumedCount==queue->capacity){
        printf("Queue is full.\n");
        return;
    }

    queue->messages[queue->head--]=message;
    queue->producedCount++;

    if(queue->head<0) queue->head=queue->capacity-1;
}

Message getMessage(MessageQueue* queue){
    if(queue->producedCount==queue->consumedCount){
        printf("Queue is empty.\n");
        return (Message){0};
    }

    Message message=queue->messages[queue->tail--];
    queue->consumedCount++;

    if(queue->tail<0) queue->tail=queue->capacity-1;

    return message;
}

void printMessage(Message message){
    printf("MESSAGE:  Type: %d  Hash: %d  Size: %d\nData: ", message.type, message.hash, message.size);
    if(message.hash!=0){
        for(int i=0;i<message.size;i++) printf("%d ", message.data[i]);
    }
    printf("\n");
}

void printMessageQueue(MessageQueue* queue){
    printf("BUFFER:  Produced: %d  Consumed: %d  Capacity: %d\n", 
    queue->producedCount, queue->consumedCount, queue->capacity);
}

