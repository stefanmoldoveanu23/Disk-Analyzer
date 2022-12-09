#include "task.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"

int appendInt(const int src, char *dest)
{
	char *start = dest + strlen(dest);
	char val[4];
	memcpy(val, &src, 4);
	return snprintf(start, 4, "%s", val);
}

int appendString(const char *src, char *dest)
{
	int sz = strlen(src);
	char *start = dest + strlen(dest);
	
	if appendInt(sz, dest);
	
}

int taskToString(const struct task src, char **dest)
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

int readStringBySize(const int fd, const int sz, char **dest)
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

int readInt(const int fd, int *dest)
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

int readString(const int fd, char **dest)
{
	int sz;
	if (readInt(fd, &sz)) {
		return -1;
	}
	
	return readStringBySize(fd, sz, dest);
}

int readTask(const int fd, struct task *dest)
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
			break;
		}
		default: {
			if (readInt(fd, &(dest->id))) {
				return -1;
			}
		}
	}
	
	return 0;
}
