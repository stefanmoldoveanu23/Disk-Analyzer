#include "analysis.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <unistd.h>

#define DIR_PATH "../data/"


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
	
	if (read_int(&(anal->cnt_dirs), fd)) {
		return -1;
	}
	
	if (read_int(&(anal->cnt_files), fd)) {
		return -1;
	}
	
	if (read_string(&(anal->path), fd)) {
		return -1;
	}
	
	if (read_int(&(anal->priority), fd)) {
		return -1;
	}
	
	if (read_int(&(anal->status), fd)) {
		return -1;
	}
	
	if (read_int(&(anal->suspended), fd)) {
		return -1;
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
	return snprintf(start, strlen(output) + 1, "%s", output);
}


int analysis_write(const int id, const struct analysis *anal, int fd)
{
	char *to_print;
	to_print = (char *)malloc(10 + 10 + 10 + 10 + 10 + strlen(anal->path) + 10 + 10 + 10 + 1);
	
	if (!to_print) {
		return -1;
	}
	
	memset(to_print, '\0', 10 + 10 + 10 + strlen(anal->path) + 10 + 10 + 10 + 1);
	
	if (write_int(id, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->total_time, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->cnt_dirs, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->cnt_files, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_string(anal->path, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->priority, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->status, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write_int(anal->suspended, to_print) < 0) {
		free(to_print);
		return -1;
	}
	
	if (write(fd, to_print, strlen(to_print)) < 0) {
		free(to_print);
		return -1;
	}
	
	free(to_print);
	
	return 0;
}



void analysis_custom_message(int fd, char *msg)
{
	if (fd < 0) {
		return;
	}
	
	int sz = 11 + strlen(msg);
	char buffer[sz];
	memset(buffer, 0, sz);
	if (snprintf(buffer, sz, "%010d%s", (int)strlen(msg), msg) < 0) {
		return;
	}
	
	sz = strlen(buffer);
	int left = sz;
	while (left) {
		int sent = send(fd, buffer + sz - left, left, 0);
		
		if (sent < 0) {
			break;
		}
		left -= sent;
	}
	
	if (left) {
		perror("Error sending message");
		return;
	}
	
	return;
}


void analysis_created(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Created analysis task with ID \'%d\' for \'%s\' and priority \'%s\'.\n", id, anal->path, (anal->priority == 1 ? "low" : (anal->priority == 2 ? "normal" : "high"))) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_path_no_exists(int fd, char *path)
{
	int sz = strlen(path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "There is no directory at \'%s\'.\n", path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_path_already_exists(int fd, char *path, int id)
{
	int sz = strlen(path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Directory \'%s\' is already included in analysis with ID \'%d\'.\n", path, id) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_id_no_exists(int fd, int id)
{
	int sz = 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "No existing analysis for task ID \'%d\'.\n", id) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_suspended(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Suspended analysis task with ID \'%d\' for \'%s\'.\n", id, anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_already_done(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Analysis task with ID \'%d\' for \'%s\' is already done.\n", id, anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_already_suspended(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Analysis task with ID \'%d\' for \'%s\' has already been suspended.\n", id, anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_already_resumed(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Analysis task with ID \'%d\' for \'%s\' has already been resumed.\n", id, anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_resumed(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Resumed analysis task with ID \'%d\' for \'%s\'.\n", id, anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_removed(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Removed analysis task with ID \'%d\', status \'%s\' for \'%s\'.\n", id, (anal->status == ANALYSIS_PENDING ? "pending" : (anal->status == ANALYSIS_INPROGRESS ? "in progress" : "done")), anal->path) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_status(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	if (snprintf(buffer, sz, "Analysis task with ID \'%d\' for \'%s\' has status \'%s\'.\n", id, anal->path, (anal->status == ANALYSIS_PENDING ? "pending" : (anal->status == ANALYSIS_INPROGRESS ? "in progress" : "done"))) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}


void analysis_list(int fd, int id, struct analysis *anal)
{
	int sz = strlen(anal->path) + 150;
	char buffer[sz];
	memset(buffer, 0, sz);
	
	char prio[4];
	memset(prio, 0, 4);
	strcat(prio, (anal->priority == 1 ? "*" : (anal->priority == 2 ? "**" : "***")));
	
	char status[15];
	memset(status, 0, 15);
	strcat(status, (anal->status == ANALYSIS_PENDING ? "pending" : (anal->status == ANALYSIS_INPROGRESS ? "in progress" : "done")));
	
	int total_time = anal->total_time + (anal->status != ANALYSIS_COMPLETE && anal->suspended != ANALYSIS_SUSPENDED) * (time(NULL) - anal->last_start);
	int min = total_time / 60;
	int sec = total_time % 60;
	
	if (snprintf(buffer, sz, "%5d\t%3s\t%s\t%5dm %2ds\t%15s\t%d files, %d dirs\n", id, prio, anal->path, min, sec, status, anal->cnt_files, anal->cnt_dirs) < 0) {
		return;
	}
	
	analysis_custom_message(fd, buffer);
}

void analysis_report(int out_fd, int id)
{
	int sz = 20;
	char buffer[sz];
	memset(buffer, 0, sz);
	if (snprintf(buffer, sz, "%s%d", DIR_PATH, id) < 0) {
		perror("Error creating file path");
		return;
	}
	
	int in_fd = open(buffer, O_RDONLY);
	if (in_fd < 0) {
		perror("Error opening input file");
		return;
	}
	
	struct stat in_stat;
	if (fstat(in_fd, &in_stat)) {
		perror("Error reading stats of file");
		return;
	}
	
	off_t offset = 0;
	if (sendfile(out_fd, in_fd, &offset, in_stat.st_size) != in_stat.st_size) {
		perror("Error sending contents to client");
		return;
	}
	
}
