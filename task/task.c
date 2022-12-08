#include "task.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

int appendInt(int src, char *dest)
{
	char *start = dest + strlen(dest);
	return snprintf(start, 5, "%.4x", src);
}

int appendString(char *src, char *dest)
{
	int sz = strlen(src);
	char *start = dest + strlen(dest);
	return snprintf(start, 5 + strlen(src), "%04x%s", sz, src);
}

int taskToString(struct task src, char **dest)
{
	switch (src.cnt) {
		case 6: {
			*dest = (char *)malloc(4);
			memset(*dest, '\0', 4);
			if (!(*dest)) {
				return -1;
			}
			
			if (appendInt(6, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			break;
		}
		case 1: {
			*dest = (char *)malloc(4 + 4 + 4 + 4 + strlen(src.path));
			memset(*dest, '\0', 4 + 4 + 4 + 4 + strlen(src.path));
			
			if (!(*dest)) {
				return -1;
			}
			
			if (appendInt(1, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			if (appendInt(src.priority, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			if (appendString(src.path, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			break;
		}
		default: {
			*dest = (char *)malloc(4 + 4);
			memset(*dest, '\0', 4 + 4);
			
			if (!(*dest)) {
				return -1;
			}
			
			if (appendInt(src.cnt, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			if (appendInt(src.id, *dest) < 0) {
				free(*dest);
				return -1;
			}
			
			break;
		}
	}
	
	return 0;
}

int readStringBySize(int fd, int sz, char **dest)
{
	*dest = (char *)malloc(sz + 1);
	if (!(*dest)) {
		return -1;
	}
	
	int left = sz;
	
	while (left) {
		int cnt = read(fd, (*dest) + sz - left, left);
		
		if (cnt < 0) {
			free(*dest);
			return -1;
		}
		
		left -= cnt;
	}
	
	return 0;
}

int readInt(int fd, int *dest)
{
	char *string;
	readStringBySize(fd, 4, &string);
	
	if (!string) {
		return -1;
	}
	
	(*dest) = (string[0] << 24) | (string[1] << 16) | (string[2] << 8) | string[3];

	free(string);
	return 0;
}

int readString(int fd, char **dest)
{
	int sz;
	if (readInt(fd, &sz)) {
		return -1;
	}
	
	return readStringBySize(fd, sz, dest);
}

int readTask(int fd, struct task *dest)
{	
	if (readInt(fd, &(dest->cnt))) {
		return -1;
	}
	
	switch (dest->cnt) {
		case 6: {break;}
		case 1: {
			if (readInt(fd, &(dest->priority))) {
				return -1;
			}
			
			if (readString(fd, &(dest->path))) {
				return -1;
			}
		}
		default: {
			if (readInt(fd, &(dest->id))) {
				return -1;
			}
		}
	}
	
	return 0;
}
