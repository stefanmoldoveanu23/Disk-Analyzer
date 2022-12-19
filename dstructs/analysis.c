#include "analysis.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int read_string_by_size(char **ret, int sz, int fd)
{
	*ret = (char *)malloc(sz + 1);
	memset(*ret, '\0', sz + 1);
	
	if (read(fd, *ret, sz) < 0) {
		free(*ret);
		return -1;
	}
	
	return 0;
}


int read_int(int *ret, int fd)
{
	char *string;
	if (read_string_by_size(&string, 10, fd)) {
		return -1;
	}
	
	*ret = atoi(string);
	free(string);
	
	return 0;
}


int read_string(char **ret, int fd)
{
	int sz;
	if (read_int(&sz, fd)) {
		return -1;
	}
	
	return read_string_by_size(ret, sz, fd);
}


int analysis_read(int *id, struct analysis *anal, int fd)
{
	if (read_int(id, fd)) {
		return -1;
	}
	
	if (read_int(&(anal->total_time), fd)) {
		return -1;
	}
	
	if (read_string(&(anal->path), fd)) {
		return -1;
	}
	
	if (read_int(&(anal->status), fd)) {
		return -1;
	}
	
	if (anal->status == ANALYSIS_INPROGRESS) {
		if (read_int(&(anal->suspended), fd)) {
			return -1;
		}
	}
	
	return 0;
}


int write_int(const int output, char *dest)
{
	char *start = dest + strlen(dest);
	return snprintf(start, 11, "%.010d", output);
}


int write_string(const char *output, char *dest)
{
	if (write_int(strlen(output), dest) < 0) {
		return -1;
	}
	
	char *start = dest + strlen(dest);
	return snprintf(start, strlen(dest) + 1, "%s", output);
}


int analysis_write(const int id, const struct analysis *anal, int fd)
{
	char *to_print;
	if (anal->status == ANALYSIS_INPROGRESS) {
		to_print = (char *)malloc(10 + 10 + 10 + strlen(anal->path) + 10 + 10 + 1);
	} else {
		to_print = (char *)malloc(10 + 10 + 10 + strlen(anal->path) + 10 + 1);
	}
	
	if (!to_print) {
		return -1;
	}
	
	if (anal->status == ANALYSIS_INPROGRESS) {
		memset(to_print, '\0', 10 + 10 + 10 + strlen(anal->path) + 10 + 10 + 1);	
	} else {
		memset(to_print, '\0', 10 + 10 + 10 + strlen(anal->path) + 10 + 1);
	}
	
	if (write_int(id, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->total_time, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_string(anal->path, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->status, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (anal->status == ANALYSIS_INPROGRESS) {
		if (write_int(anal->suspended, to_print) < 0) {
			free(to_print);
			return -1;
		}
	}
	
	if (write(fd, to_print, strlen(to_print)) < 0) {
		free(to_print);
		return -1;
	}
	
	free(to_print);
	
	return 0;
}
