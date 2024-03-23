#ifndef PARENT_H_
#define PARENT_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <errno.h>
#include <time.h>
#include <sys/select.h>

void chooseOption(char* childPath);
void executeChild(char* childPath);
void closeLastProcess();
void printProcesses();
void closeAllChildren();
void allowPrintingEveryone(bool allowance);
void allowPrintingChild(int number, bool allowance);
void requestPrinting(int number);
void alarmHandler(int signal);
pid_t **getChildren();
char *getNameByPid(pid_t pid);
pid_t getPidByName(const char *process_name);
void freeChildren(pid_t **children);
#endif