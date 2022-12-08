#ifndef TASK_H
#define TASK_H

struct task{
	int cnt;

	int id;
	char *path;
	int priority;
};

/// appends an int to a request string
int appendInt(int val, char *dest);

/// appends a string to a request string
int appendString(char *string, char *dest);

/// gets a task instance and translates it to a string
char *taskToString(struct task *tsk);

/// reads an int from a request file descriptor
int readInt(int fd);

/// reads a string from a request file descriptor
char *readString(int fd);

/// gets a request file descriptor and translates its input to a task instance
struct task *readTask(int fd);

#endif
