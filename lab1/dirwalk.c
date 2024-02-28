#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#define LEN 500
#define CAP 500000
enum options{direct=0, file, slink, sort};
void browse(char* name, int* options,char** result,size_t* size);
void getOptions(int*options,int argc, char* argv[]);
void writePath(char**result,size_t* size, char* path);
int compare(const void* str1, const void* str2);
int main(int argc, char* argv[]){
    extern int optind;
    char**result=(char**)calloc(CAP,sizeof(char*));
    for(int i=0;i<CAP;i++) result[i]=(char*)calloc(LEN,1);
    size_t size=0;
    char dirname[LEN];
    int options[4]={0,0,0,0};
    getOptions(options,argc,argv);
    if(argc==1||optind==argc) strcpy(dirname, ".");
    else strcpy(dirname, argv[argc-1]);
    browse(dirname, options,result,&size);
    if(options[sort]==1){
        qsort(result,size,sizeof(char*),compare);
    }
    for(int i=0;i<size;i++){
        printf("%s\n", result[i]);
    }

    for(int i=0;i<CAP;i++){
        free(result[i]);
    }
    free(result);
}

void getOptions(int*options, int argc, char* argv[]){
    int opt = 0;
    int counter=0;
	while ( (opt = getopt(argc, argv, "dfls")) != -1){
		switch (opt) {
		case 'd': options[direct]=1; break;
		case 'f': options[file]=1; break;
		case 'l': options[slink]=1; break;
		case 's': options[sort]=1; break;
		case '?': printf("Error found !\n"); break;
		} 
	}
    for(int i=0;i<3;i++){
        if(options[i]==0) counter++;
    }
    if(counter==3){
        for(int i=0;i<3;i++) options[i]=1;
    }
}

void browse(char* name, int* options,char** result,size_t* size){
    DIR* dir=opendir(name);
    struct dirent *entry;
    struct stat stat;
    char path[LEN];
    if(!dir){
        printf("Error occured while opening %s\n", name);
        return;
    }
    entry=readdir(dir);
    while(entry){
        if(strcmp(entry->d_name, ".")==0||strcmp(entry->d_name, "..")==0) {
            entry=readdir(dir);
            continue;
        }
        strcpy(path,name);
        strcat(path,"/");
        strcat(path,entry->d_name);
        if(lstat(path,&stat)==0){
        switch(__S_IFMT&stat.st_mode){
            case __S_IFDIR:
                if(options[direct]==1) writePath(result,size,path);
                browse(path,options,result,size);
                break;
            case __S_IFREG:
                if(options[file]==1) writePath(result,size,path);
                break;
            case __S_IFLNK:
                if(options[slink]==1) writePath(result,size,path);
                break;
        }
    }
    entry=readdir(dir);
    }
    closedir(dir);
}

void writePath(char**result,size_t* size, char* path){
    strcpy(result[*size],path);
    (*size)++;
}

int compare(const void* str1, const void* str2){
    char *const *pp1 = str1;
    char *const *pp2 = str2;
    return strcmp(*pp1, *pp2);
}
