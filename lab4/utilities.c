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
    printf("BUFFER:  Produced: %d  Consumed: %d  Capacity: %d  Current size: %d\n", 
    queue->producedCount, queue->consumedCount, queue->capacity, queue->producedCount-queue->consumedCount);
}

void createSemaphores(Semaphores* semaphores, size_t queueCapacity){

    if(sem_init(&semaphores->freeSpace, 1, queueCapacity)==-1){
        perror("free space semaphore error");
        exit(EXIT_FAILURE);
    }

    if(sem_init(&semaphores->itemsToConsume, 1, 0)==-1){
        perror("items to consume semaphore error");
        exit(EXIT_FAILURE);
    }

    if(sem_init(&semaphores->mutex, 1, 1)==-1){
        perror("mutex error");
        exit(EXIT_FAILURE);
    }

}

void createSharedMemory(int* queueID, int* semaphoresID, int* dataID, size_t capacity){
    key_t queueKey=ftok(".",'a');
    key_t semaphoresKey=ftok(".",'b');
    key_t dataKey=ftok(".",'c');        //for queue->messages field

    if(queueKey==-1||semaphoresKey==-1||dataKey==-1){
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    *queueID=shmget(queueKey, (sizeof(MessageQueue)+capacity*sizeof(Message)), IPC_CREAT | 0666);
    *semaphoresID=shmget(semaphoresKey, sizeof(Semaphores), IPC_CREAT | 0666);
    *dataID=shmget(dataKey, (sizeof(Message)*capacity), IPC_CREAT | 0666);
    if(*queueID==-1||*semaphoresID==-1){
        perror("shmget error");
        exit(EXIT_FAILURE);              
    }
}

void detachSharedMemory(void* ptr){
    shmdt(ptr); //why error??
}

pid_t **getChildren(){
    pid_t ppid = getppid();
    pid_t **ret = (pid_t **)malloc(sizeof(pid_t *));
    int count = 0;
    ret[count] = (pid_t *)malloc(sizeof(pid_t));
    DIR *dir;
    struct dirent *entry;

    // Открываем директорию /proc
    dir = opendir("/proc");
    if (!dir)
    {
        perror("Ошибка при открытии /proc");
        exit(EXIT_FAILURE);
    }
    // Читаем содержимое директории /proc
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (!atoi(entry->d_name) || (atoi(entry->d_name) && atoi(entry->d_name) < ppid))
            continue;

        char full_path[264];
        snprintf(full_path, 264, "/proc/%s", entry->d_name);
        struct stat fileStat;
        if (lstat(full_path, &fileStat) == -1)
            continue;

        if (S_ISDIR(fileStat.st_mode))
        {
            // Проверяем, является ли имя директории числом (это может быть pid процесса)
            char *endptr;
            pid_t pid = strtol(entry->d_name, &endptr, 10);

            if (*endptr == '\0')
            {
                // Это числовое имя, проверим, является ли это дочерним процессом
                char proc_path[256];
                snprintf(proc_path, sizeof(proc_path), "/proc/%d/stat", pid);

                FILE *file = fopen(proc_path, "r");
                if (file)
                {
                    // Читаем информацию из файла /proc/<pid>/stat
                    long ppid = 0;
                    fscanf(file, "%*d %*s %*c %ld", &ppid);
                    fclose(file);

                    // Если ppid соответствует pid родительского процесса, то это дочерний процесс
                    if (ppid == getpid())
                    {
                        count++;
                        ret = (pid_t **)realloc(ret, (count + 1) * sizeof(pid_t *));
                        ret[count] = (pid_t *)malloc(sizeof(pid_t));
                        *(ret[count - 1]) = pid;
                    }
                }
            }
        }
    }
    closedir(dir);
    ret[count] = NULL;
    return ret;
}

char *getNameByPid(pid_t pid){
    char proc_path[256];
    int path_length = snprintf(proc_path, sizeof(proc_path), "/proc/%d/cmdline", pid);

    // Проверка на ошибки при формировании пути к файлу
    if (path_length < 0 || path_length >= (int)sizeof(proc_path))
    {
        perror("Error forming file path");
        exit(EXIT_FAILURE);
    }

    // Открытие файла с именем процесса
    FILE *file = fopen(proc_path, "r");
    if (!file)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Выделение памяти для имени процесса
    char *name = (char *)malloc(256);
    if (!name)
    {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    // Чтение имени процесса из файла
    fgets(name, 256, file);

    // Убедимся, что строка завершается символом '\0'
    name[strcspn(name, "\n")] = '\0';

    // Переаллокация памяти под точное количество символов
    char *temp = (char *)realloc(name, strlen(name) + 1);
    if (!temp)
    {
        perror("Error reallocating memory");
        free(name);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    name = temp;

    // Закрытие файла
    if (fclose(file))
    {
        perror("Error closing file");
        free(name);
        exit(EXIT_FAILURE);
    }

    // Возвращение указателя на строку с именем процесса
    return name;
}

pid_t getPidByName(const char *process_name){
    // Открытие директории /proc
    DIR *dir = opendir("/proc");
    if (!dir)
    {
        perror("Error opening /proc directory");
        return -1;
    }

    // Чтение содержимого директории /proc
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        pid_t current_pid = atoi(entry->d_name);
        if (current_pid > 0)
        {
            char *name = getNameByPid(current_pid);
            if (name && !strcmp(name, process_name))
            {
                free(name);
                closedir(dir);
                return current_pid;
            }
            free(name);
        }
    }

    // Закрытие директории /proc
    closedir(dir);
    return -1;
}

void freeChildren(pid_t **children){
    for (int i = 0; children[i]; i++)
        free(children[i]);
    free(children);
}

void closeAllChildren(){
    pid_t **children = getChildren();
    for (int i = 0; children[i]; i++){
        char *pname = getNameByPid(*(children[i]));
        kill(*(children[i]),SIGKILL);
        printf("Process %s was deleted\n", pname);
        waitpid(*(children[i]), NULL, 0);
    }
    freeChildren(children);
}

void closeLastProcess(){
    pid_t** children=getChildren();
    int last=0;

    for(;children[last];last++);

    if(last==0){
        printf("No children\n");
        freeChildren(children);
        return;
    }
    printf("Process %s was deleted. %d remained.\n", getNameByPid(*(children[last-1])),last-1);
    kill(*(children)[last-1], SIGKILL);
    waitpid(*(children[last- 1]), NULL, 0);
    freeChildren(children);
}