#include "utilities.h"

void selectOption(char* varFile, char** env){
    char option;
    extern char** environ;
    do{
        option=getchar();
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

    if(childCounter>=100){
        printf("Too much processes! Exit\n");
        exit(EXIT_SUCCESS);
    }

    char childName[10];
    snprintf(childName, sizeof(childName), "child_%02d", childCounter);
    
    char*argv[]= {childName, varFile, &option, NULL};
    char**envp=(char**)calloc(0,sizeof(char*));

    pid_t pid=fork();


    if(pid==-1){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    else if(pid==0){
        strcat(childPath, "/child");
        createChildEnvironment(varFile, &envp);

        execve(childPath, argv, envp);
    }

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
