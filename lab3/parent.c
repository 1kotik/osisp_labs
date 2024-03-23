#include "parent.h"

int main(int argc,char*argv[]){
    int pid=(int)getpid();
    int ppid=(int)getppid();
    printf("Name: %s PID: %d PPID: %d\n", argv[0],pid,ppid);

    signal(SIGALRM, alarmHandler);

    chooseOption(argv[1]);

    exit(EXIT_SUCCESS);
}


void chooseOption(char* childPath){
    char option[10];
    do{
        fgets(option,10,stdin);
        switch(option[0]){
        case '+':
            executeChild(childPath);
            break;
        case '-':
            closeLastProcess();
            break;
        case 'l':
            printProcesses();
            break;
        case 'k':
            closeAllChildren();
            break;
        case 's':
            if(option[1]=='\n') allowPrintingEveryone(false);
            else allowPrintingChild(atoi(option+1), false);
            break;
        case 'g':
            if(option[1]=='\n') allowPrintingEveryone(true);
            else allowPrintingChild(atoi(option+1), true);
            break;
        case 'p':
            if(option[1]!='\n') requestPrinting(atoi(option+1));
            break;
        }
        rewind(stdin);
    }while(option[0]!='q');
    closeAllChildren();
    exit(EXIT_SUCCESS);
}

void executeChild(char* childPath){
    static int childCounter=0;
    childCounter++;

    if(childCounter>=100){
        printf("Too much processes! Exit\n");
        exit(EXIT_SUCCESS);
    }

    char childName[10];
    snprintf(childName, sizeof(childName), "C_%d", childCounter);
    char*argv[]={childName, NULL};
    pid_t pid=fork();

    if(pid==-1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid==0){
        strcat(childPath, "/child");
        execve(childPath, argv, NULL);
        
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else{
        usleep(1500);
        kill(pid, SIGUSR1);
    }

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

void printProcesses(){
    int pid=(int)getpid();
    int ppid=(int)getppid();
    printf("Parent name: %s PID: %d PPID: %d\n", getNameByPid(pid),pid,ppid);
    printf("Children:\n");
    pid_t** children=getChildren();
    for(int i=0;children[i];i++) printf("Name: %s PID: %d\n", getNameByPid(*(children[i])), (int)*(children[i]));
    freeChildren(children);
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

void allowPrintingEveryone(bool allowance){
    pid_t** children=getChildren();
    for(int i=0;children[i];i++){
        if(allowance) kill(*(children[i]), SIGUSR1);
        else kill(*(children[i]), SIGUSR2);
    }
    freeChildren(children);
}

void allowPrintingChild(int number, bool allowance){
    char name[10];
    snprintf(name, 10, "C_%i", number);
    pid_t pid=getPidByName(name);
    if(pid==-1){
        printf("No such process\n");
        allowPrintingEveryone(true);
        return;
    }
    if(allowance) kill(pid, SIGUSR1);
    else kill(getPidByName(name), SIGUSR2);
}

void requestPrinting(int number){
    allowPrintingEveryone(false);
    allowPrintingChild(number,true);
    alarm(5);
}

void alarmHandler(int signal){
    if(signal==SIGALRM) allowPrintingEveryone(true);
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