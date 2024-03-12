#include "utilities.h"

int main(int argc, char** argv, char** envp){
    if(argc!=2){
        printf("Specify only variables file\n");
        exit(EXIT_FAILURE);
    }

    getEnvironmentByParent();
    selectOption(argv[1], envp);
    exit(EXIT_SUCCESS);
}


