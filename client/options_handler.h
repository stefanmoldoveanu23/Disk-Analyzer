#ifndef OPTIONS_HANDLER_H
#define OPTIONS_HANDLER_H

#include "../dstructs/task.h"
#include "getopt.h"

extern const char *options;
extern const struct option long_options[10];

/// generate task from arguments
int get_task(const int argc, char **argv, struct task *tsk);

#endif
