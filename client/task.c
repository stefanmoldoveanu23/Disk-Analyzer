#include "task.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

int appendInt(int val, char *dest)
{
	char *start = dest + strlen(dest);
	return snprintf(start, 5, "%.4x", val);
}

int appendString(char *string, char *dest)
{
	int sz = strlen(string);
	char *start = dest + strlen(dest);
	return snprintf(start, 5 + strlen(string), "%04x%s", sz, string);
}

char *taskToString(struct task *tsk)
{
	char *ret;
	
	switch (tsk->cnt) {
		case 6: {
			ret = (char *)malloc(4);
			memset(ret, '\0', 4);
			if (!ret) {
				return NULL;
			}
			
			if (appendInt(6, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			break;
		}
		case 1: {
			printf("1\n");
			ret = (char *)malloc(4 + 4 + 4 + 4 + strlen(tsk->path));
			memset(ret, '\0', 4 + 4 + 4 + 4 + strlen(tsk->path));
			
			if (!ret) {
				return NULL;
			}
			
			if (appendInt(1, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			if (appendInt(tsk->priority, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			if (appendString(tsk->path, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			break;
		}
		default: {
			ret = (char *)malloc(4 + 4);
			memset(ret, '\0', 4 + 4);
			
			if (!ret) {
				return NULL;
			}
			
			if (appendInt(tsk->cnt, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			if (appendInt(tsk->id, ret) < 0) {
				free(ret);
				return NULL;
			}
			
			break;
		}
	}
	
	return ret;
}

char *readStringBySize(int fd, int sz)
{
	char *string = (char *)malloc(sz + 1);
	if (!string) {
		return NULL;
	}
	
	int left = sz;
	
	while (left) {
		int cnt = read(fd, string + sz - left, left);
		
		if (cnt < 0) {
			free(string);
			return NULL;
		}
		
		left -= cnt;
	}
	
	return string;
}

int readInt(int fd)
{
	char *string = readStringBySize(fd, 4);
	if (!string) {
		return -1;
	}
	
	int ret = (string[0] << 24) | (string[1] << 16) | (string[2] << 8) | string[3];

	free(string);
	return ret;
}

char *readString(int fd)
{
	int sz = readInt(fd);
	if (sz < 0) {
		return NULL;
	}
	
	return readStringBySize(fd, sz);
}

struct task *readTask(int fd)
{
	struct task *tsk = (struct task *)malloc(sizeof(struct task));
	if (!tsk) {
		return NULL;
	}
	
	if ((tsk->cnt = readInt(fd)) < 0) {
		free(tsk);
		return NULL;
	}
	
	switch (tsk->cnt) {
		case 6: {break;}
		case 1: {
			if ((tsk->priority = readInt(fd)) < 0) {
				free(tsk);
				return NULL;
			}
			
			if ((tsk->path = readString(fd)) == NULL) {
				free(tsk);
				return NULL;
			}
		}
		default: {
			if ((tsk->id = readInt(fd)) < 0) {
				free(tsk);
				return NULL;
			}
		}
	}
	
	return tsk;
}
