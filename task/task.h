#ifndef TASK_H
#define TASK_H

struct task{
	int cnt;

	int id;
	char *path;
	int priority;
};

/// appends an int to a request string
int appendInt(int src, char *dest);

/// appends a string to a request string
int appendString(char *src, char *dest);

/// gets a task instance and translates it to a string
int taskToString(struct task src, char **dest);

/// reads an int from a request file descriptor
int readInt(int fd, int *dest);

/// reads a string from a request file descriptor
int readString(int fd, char **dest);

/// gets a request file descriptor and translates its input to a task instance
int readTask(int fd, struct task *dest);

#endif
