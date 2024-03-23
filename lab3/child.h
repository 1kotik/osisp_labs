#ifndef CHILD_H_
#define CHILD_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

void userHandler(int signal);
void alarmHandler(int signal);

#endif