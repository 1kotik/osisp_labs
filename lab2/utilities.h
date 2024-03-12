#ifndef UTIL_H_
#define UTIL_H_
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
#endif