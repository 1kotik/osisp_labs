#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

char* getEnvValue(char** env,char* variable);
void executeChild(char* childPath, char* varFile, char option);
void selectOption(char* varFile, char** env);
void getEnvironmentByParent();
void getEnvironmnetByChild(char* varFile, char**env);
int compare(const void* str1, const void* str2);
void createChildEnvironment(char* varFile, char*** env);

int main(int argc, char** argv, char** envp){
    if(argc!=2){
        printf("Specify only variables file\n");
        exit(EXIT_FAILURE);
    }

    getEnvironmentByParent();
    selectOption(argv[1], envp);
    exit(EXIT_SUCCESS);
}

void selectOption(char* varFile, char** env){
    char option;
    extern char** environ;
    do{
        scanf("%c",&option);
        switch(option){
        case '+':
            executeChild(getenv("CHILD_PATH"), varFile, option);
            break;
        case '*':
            executeChild(getEnvValue(env,"CHILD_PATH"), varFile, option);
            break;
        case '&':
            executeChild(getEnvValue(environ, "CHILD_PATH"), varFile, option);
            break;
        }
        rewind(stdin);
    } while(option!='q');
}

void executeChild(char* childPath, char* varFile, char option){
    static int childCounter=0;
    childCounter++;

    if(childCounter>=100) return;

    char childName[10];
    snprintf(childName, sizeof(childName), "child_%02d", childCounter);
    
    char*argv[]= {childName, varFile, &option, NULL};
    char**envp=(char**)calloc(0,sizeof(char*));

    pid_t pid=fork();


    if(pid==-1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    strcat(childPath, "/child");
    createChildEnvironment(varFile, &envp);
    printf("%s\n",childPath);

    execve(childPath, argv, envp);

    perror("execve");
    exit(EXIT_FAILURE);

    waitpid(pid, NULL, 0);
}

char* getEnvValue(char** env,char* variable){

    char value[255];

    for(int i=0;env[i]!=NULL;i++){
        if(strncmp(variable,env[i],strlen(variable))==0&&env[i][strlen(variable)]=='=') return &env[i][strlen(variable)+1];
    } 

}

void getEnvironmentByParent(){
    int size=0;
    extern char** environ;

    for(;environ[size];size++);

    char** env=(char**)calloc(size, sizeof(char*));

    for(int i=0;i<size;i++){
        env[i]=(char*)calloc(strlen(environ[i])+1,1);
        strcpy(env[i], environ[i]);
    }

    qsort(env, size, sizeof(char*), compare);

    for(int i=0;i<size;i++){
        printf("%s\n",env[i]);
        free(env[i]);
    }

    free(env);
}

void getEnvironmnetByChild(char* varFile, char**env){
    FILE* file=fopen(varFile, "r");

    char variable[150];
    char* value;

    while(fscanf(file, "%s", variable)!=EOF){
        if(env) value=getEnvValue(env,variable); // * &
        else value=getenv(variable); // +

        if(!value){
            continue;
        }

        printf("%s=%s\n", variable, value);
    }

    fclose(file);
}

int compare(const void* str1, const void* str2){
    char *const *pp1 = str1;
    char *const *pp2 = str2;
    return strcmp(*pp1, *pp2);
}

void createChildEnvironment(char* varFile, char*** env){
    FILE* file=fopen(varFile, "r");
    if(!file){
        printf("File error\n");
        exit(EXIT_FAILURE);
    }
    int size=0;
    char variable[150];
    char* value;

    while(fscanf(file, "%s", variable)!=EOF){
        if(value=getenv(variable)){
            strcat(variable, "=");
            strcat(variable, value);
            size++;
            *env=(char**)realloc(*env, (size+1)*sizeof(char*));
            (*env)[size-1]=(char*)calloc(strlen(variable)+1,1);
            strcpy((*env)[size-1],variable);
        }
    }

    fclose(file);
    (*env)[size]=NULL;
}


