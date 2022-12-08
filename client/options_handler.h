#ifndef OPTIONS_HANDLER_H
#define OPTIONS_HANDLER_H

#include "../task/task.h"
#include "getopt.h"

extern const char *options;
extern const struct option long_options[9];

/// generate task from arguments
int get_task(int argc, char **argv, struct task *tsk);

#endif
