#ifndef TASK_H
#define TASK_H

struct task{
	int cnt;

	int id;
	char *path;
	int priority;
};

/// appends an int to a request string
int appendInt(const int src, char *dest);

/// appends a string to a request string
int appendString(const char *src, char *dest);

/// gets a task instance and translates it to a string
int taskToString(const struct task src, char **dest);

/// reads an int from a request file descriptor
int readInt(const int fd, int *dest);

/// reads a string from a request file descriptor
int readString(const int fd, char **dest);

/// gets a request file descriptor and translates its input to a task instance
int readTask(const int fd, struct task *dest);

#endif
