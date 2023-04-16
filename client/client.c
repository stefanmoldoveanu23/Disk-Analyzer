#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

#include "options_handler.h"
#include "../dstructs/create_socket.h"

#define PORT 8080

int client_read_string(int fd, char *buffer, int sz)
{
	int left = sz;
	while (left) {
		int cnt = read(fd, buffer + sz - left, left);
		if (cnt < 0) {
			return 1;
		}
		
		left -= cnt;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	struct task tsk;
	
	if (get_task(argc, argv, &tsk)) {
		if (errno) {
			perror(NULL);
		}
		
		return 1;
	}
	
	char *request;
	
	if (taskToString(tsk, &request)) {
		return 1;
	}
	
	if (tsk.cnt == 1) {
		free(tsk.path);
	}
	
	struct socket_connection connection;
	if (create_socket_connector(&connection, PORT)) {
		perror(NULL);
		free(request);
		
		return 1;
	}
	
	connection.server_fd = connect(connection.client_fd, (struct sockaddr*)(&connection.address), sizeof(connection.address));
	if (connection.server_fd < 0) {
		perror(NULL);
		free(request);
		shutdown(connection.client_fd, SHUT_RDWR);
		return 1;
	}
	
	int pos = 0, total = strlen(request);
	while (pos != total) {
		int sent = send(connection.client_fd, request + pos, total - pos, 0);
		
		if (sent < 0) {
			perror(NULL);
			free(request);
			close(connection.server_fd);
			shutdown(connection.client_fd, SHUT_RDWR);
			
			return errno;
		}
		
		pos += sent;
	}
	
	free(request);
	
	if (tsk.cnt < 6) {
		char buffer[11];
		memset(buffer, 0, 11);
		
		if (!client_read_string(connection.client_fd, buffer, 10)) {
			int sz = atoi(buffer);
			char response[sz + 1];
			memset(response, 0, sz + 1);
			
			if (!client_read_string(connection.client_fd, response, sz)) {
				printf("%s", response);
			}
		}
	} else if (tsk.cnt == 6) {
		printf("\n   ID\tPRI\t     Path\t      Time Elapsed\t            Status\t      Details\n");

		char cntbuf[11];
		memset(cntbuf, 0, 11);
		
		if (!client_read_string(connection.client_fd, cntbuf, 10)) {
			int cnt = atoi(cntbuf);
			
			while (cnt--) {
				char szbuf[11];
				memset(szbuf, 0, 11);

				if (!client_read_string(connection.client_fd, szbuf, 10)) {
					int sz = atoi(szbuf);
					char buffer[sz + 1];
					memset(buffer, 0, sz + 1);

					if (!client_read_string(connection.client_fd, buffer, sz)) {
						printf("%s", buffer);
					}
				}
			}
			
		}
	} else if (tsk.cnt == 7) {
		char sol;
		int rd = 0;
		while (!rd) {
			rd = read(connection.client_fd, &sol, 1);
		}
		
		printf("%d\n", rd);
		
		if (rd < 0) {
			perror(NULL);
			close(connection.server_fd);
			shutdown(connection.client_fd, SHUT_RDWR);
			
			return 0;
		}
		
		char szbuf[11];
		memset(szbuf, 0, 11);
		
		if (!client_read_string(connection.client_fd, szbuf, 10)) {
			int sz = atoi(szbuf);
			char bufin[sz + 1];
			memset(bufin, 0, sz + 1);
			
			if (!client_read_string(connection.client_fd, bufin, sz)) {
				if (sol == '1') {
					int in_fd = open(bufin, O_RDONLY);
					if (in_fd < 0) {
						perror(NULL);
					}
					if (in_fd >= 0) {
						struct stat in_stat;
						if (!fstat(in_fd, &in_stat)) {
							char dump;
							int rd = 0;
							while (!rd) {
								rd = read(in_fd, &dump, 1);
							}
							
							if (rd == 1 && dump == '1') {
								char buffer[101];
								
								while (1) {
									memset(buffer, 0, 101);
									rd = read(in_fd, buffer, 100);
									if (rd < 0) {
										perror(NULL);
										break;
									}
									
									printf("%s", buffer);
									if (rd < 100) {
										break;
									}
								}
								printf("\n");
							}
						}
						
					}
				} else {
					printf("%s", bufin);
				}

			}
		}
	} else {
		printf("Usage: da [OPTION]... [DIR]...\nAnalyze the space occupied by the directory at [DIR]\n\
	-a, --add analyze a new directory path for disk usage\n\
	-p, --priority set priority for the new analysis (works only with -a argument)\n\
	-S, --suspend <id> suspend task with <id>\n\
	-R, --resume <id> resume task with <id>\n\
	-r, --remove <id> remove the analysis with the given <id>\n\
	-i, --info <id> print status about the analysis with <id> (pending, progress, d\n\
	-l, --list list all analysis tasks, with their ID and the corresponding root p\n\
	-p, --print <id> print analysis report for those tasks that are \"done\"\n");
	}
	
	close(connection.server_fd);
	shutdown(connection.client_fd, SHUT_RDWR);
	
	return 0;
}
