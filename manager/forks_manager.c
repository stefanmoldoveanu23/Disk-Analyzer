#include "forks_manager.h"
#include "../dstructs/paths.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT_ACCEPTOR 8081
#define PORT_RESPONSE 8082


int forks_startup(struct forks_manager *man)
{
	if (tree_init(&(man->tre))) {
		perror(NULL);
		return 1;
	}
	
	if (create_socket_acceptor(&(man->connection), PORT_ACCEPTOR)) {
		perror(NULL);
		tree_clear(&(man->tre));
		return 1;
	}
	
	man->cnt_dirs = man->cnt_files = 0;
	
	man->path = NULL;
	return 0;
}

int forks_add(struct forks_manager *man)
{
	
	pid_t pid = fork();
	if (pid == -1) {
		forks_send_result(man, 0);
		return -1;
	}
	
	if (pid) {
		close(man->connection.client_fd);
		return 1;
	}
	
	signal(SIGTERM, man->handler);
	signal(SIGINT, man->handler);
	signal(SIGTSTP, man->handler);
	signal(SIGCONT, man->handler);
	
	if (forks_read_path(man)) {
		perror("Could not read path of new fork.");
		forks_send_result(man, 0);
		close(man->connection.client_fd);
		tree_clear(&(man->tre));
		return -1;
	}
	
	if (forks_send_pid(man)) {
		perror("Could not send back pid.");
		forks_send_result(man, 0);
		close(man->connection.client_fd);
		tree_clear(&(man->tre));
		return -1;
	}
	
	
	close(man->connection.client_fd);

	
	int progress = forks_read_progress(man);
	if (progress < 0) {
		perror("Could not read the progress of new fork.");
		forks_send_result(man, 0);
		tree_clear(&(man->tre));
		free(man->path);
		return -1;
	} else if (progress > 0) {
		forks_send_result(man, 1);
		tree_clear(&(man->tre));
		return 0;
	}
	
	if (forks_solve(man)) {
		forks_send_result(man, 0);
		tree_clear(&(man->tre));
		free(man->path);
		return 0;
	} else {
		if (*(man->interrupted)) {
			tree_clear(&(man->tre));
			free(man->path);
			return 0;
		} else if (*(man->done)) {
			forks_send_result(man, 2);
		} else {
			forks_send_result(man, 1);
		}
	}
	
	if (!(*(man->done))) {
		forks_write(man);
	} else {
		forks_save(man);
	}
	
	return 0;
}

int forks_read_progress(struct forks_manager *man)
{
	char filepath[PATH_MAX];
	memset(filepath, '\0', PATH_MAX);
	
	snprintf(filepath, strlen(DIR_PATH) + 11, "%s%d", DIR_PATH, man->id);
	
	int fd = open(filepath, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		return -1;
	}
	
	char type = '\0';
	read(fd, &type, 1);
	
	if (type == '1') {
		close(fd);
		return 1;
	} else if (type == 0) {
		close(fd);
		return 0;
	}
	
	if (tree_build(man->tre, fd)) {
		return -1;
	}
	
	close(fd);
	return 0;
}

int forks_read_string(int client_fd, char *dest, int sz)
{
	int left = sz;
	
	while (left) {
		int cnt = read(client_fd, dest + sz - left, left);
		
		if (cnt < 0) {
			return 1;
		}
		
		left -= cnt;
	}
	
	dest[sz] = '\0';
	
	return 0;
}

int forks_read_path(struct forks_manager *man)
{
	char vessel[11];
	if (forks_read_string(man->connection.client_fd, vessel, 10)) {
		return 1;
	}
	man->id = atoi(vessel);
	
	if (forks_read_string(man->connection.client_fd, vessel, 10)) {
		return 1;
	}
	int sz = atoi(vessel);
	man->path = (char *)malloc(sz + 1);
	if (!(man->path)) {
		return 1;
	}
	
	if (forks_read_string(man->connection.client_fd, man->path, sz)) {
		free(man->path);
		return 1;
	}

	return 0;
}


int forks_send_pid(struct forks_manager *man)
{
	char vessel[11];
	snprintf(vessel, 11, "%010d", (int)getpid());
	
	int pos = 0, total = 10;
	while (pos != total) {
		int sent = send(man->connection.client_fd, vessel + pos, total - pos, 0);
		
		if (sent < 0) {
			perror(NULL);
			return 1;
		}
		
		pos += sent;
	}
	
	return 0;
}


int forks_send_result(struct forks_manager *man, int result)
{
	char response[32];
	
	if (result == 0) {
		if (snprintf(response, 32, "0%010d", man->id) < 0) {
			return 1;
		}
	} else {
		if (snprintf(response, 32, "%c%010d%010d%010d", result + '0', man->id, man->cnt_dirs, man->cnt_files) < 0) {
			return 1;
		}
	}
	response[31] = '\0';
	
	man->cnt_dirs = man->cnt_files = 0;
	
	struct socket_connection connection;
	if (create_socket_connector(&connection, PORT_RESPONSE)) {
		perror("Error creating response socket");
		return 1;
	}
	
	connection.server_fd = connect(connection.client_fd, (struct sockaddr *)(&connection.address), sizeof(connection.address));
	if (connection.server_fd < 0) {
		perror("Error connecting to response socket");
		shutdown(connection.client_fd, SHUT_RDWR);
		return 1;
	}
	
	if (create_socket_send_message(response, connection.client_fd)) {
		perror("Error sending response");
		close(connection.server_fd);
		shutdown(connection.client_fd, SHUT_RDWR);
		return 1;
	}
	
	close(connection.server_fd);
	shutdown(connection.client_fd, SHUT_RDWR);
	return 0;
}


int forks_fts_parc(struct forks_manager *man, struct tree *curr, FTS *ftsp)
{
	if (!(curr->info)) {
		struct state *st = (struct state *)malloc(sizeof(struct state));
		if (!st) {
			perror("Error when creating new state.");
			return 1;
		}
		
		st->size = st->done = 0;
		curr->info = st;
	}
	
	while (1) {
		if (*(man->interrupted)) {
			return 0;
		}
		
		while (!(*(man->interrupted)) && !(*(man->done)) && *(man->suspended));
		
		if (time(NULL) - man->last_send >= 1) {
			forks_send_result(man, 2);
		}
		
		FTSENT *nxt = fts_read(ftsp);
		
		switch (nxt->fts_info) {
			case FTS_DEFAULT:
			case FTS_F:
			case FTS_SL:
			case FTS_SLNONE:
			{
				++man->cnt_files;
				break;
			}
			case FTS_D:
			case FTS_DC:
			case FTS_DOT:
			{
				++man->cnt_dirs;
				break;
			}
		}
		
		switch (nxt->fts_info) {
			case FTS_DNR:
			case FTS_ERR:
			case FTS_NS:
			{
				perror("Error reading file.");
				return 1;
			}
			case FTS_DEFAULT:
			case FTS_F:
			case FTS_SL:
			case FTS_SLNONE:
			case FTS_DC:
			case FTS_DOT:
			{
				((struct state *)(curr->info))->size += nxt->fts_statp->st_size;
				break;
			}
			case FTS_D:
			{
				if (*(man->done)) {
					return 0;
				}
				struct tree *chld = hash_find(curr->hsh, nxt->fts_name);
				if (!chld) {
					if (tree_insert(curr, nxt->fts_name, NULL)) {
						return 1;
					}
					chld = hash_find(curr->hsh, nxt->fts_name);
				}
				
				if (curr->info && ((struct state *)(curr->info))->done) {
					fts_set(ftsp, nxt, FTS_SKIP);
					break;
				}
				
				if (forks_fts_parc(man, chld, ftsp)) {
					return 1;
				}
				
				((struct state *)(curr->info))->size += ((struct state *)(chld->info))->size;
				
				break;
			}
			case FTS_DP:
			{
				((struct state *)(curr->info))->done = 1;
				return 0;
			}
			default:
			{
				perror("Unknown case encountered.");
				return 1;
			}
		}
		
	}
}


int forks_solve(struct forks_manager *man)
{
	if (tree_insert(man->tre, man->path, NULL)) {
		return 1;
	}
	
	struct tree *curr = tree_get_path(man->tre, man->path);
	
	if (curr->info && ((struct state *)(curr->info))->done) {
		return 0;
	}
	
	char * const paths[2] = {man->path, NULL};
	
	FTS *ftsp = fts_open(paths, FTS_PHYSICAL | FTS_SEEDOT, NULL);
	if (!ftsp) {
		return 1;
	}
	
	fts_read(ftsp);
	
	man->last_send = time(NULL);
	if (forks_fts_parc(man, curr, ftsp)) {
		perror("Error parsing directory tree.");
		fts_close(ftsp);
		return 1;
	}

	fts_close(ftsp);
	
	return 0;
}


void forks_save(struct forks_manager *man)
{
	char filepath[PATH_MAX];
	memset(filepath, 0, PATH_MAX);
	
	if (snprintf(filepath, strlen(DIR_PATH) + 11, "%s%d", DIR_PATH, man->id) < 0) {
		tree_clear(&(man->tre));
		free(man->path);
		return;
	}

	int fd = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		tree_clear(&(man->tre));
		free(man->path);
		return;
	}
	
	write(fd, "0", 1);
	
	tree_save(&(man->tre), fd);
	free(man->path);
	
	close(fd);
}


void forks_write(struct forks_manager *man)
{
	char filepath[PATH_MAX];
	memset(filepath, 0, PATH_MAX);
	
	if (snprintf(filepath, strlen(DIR_PATH) + 11, "%s%d", DIR_PATH, man->id) < 0) {
		tree_clear(&(man->tre));
		free(man->path);
		return;
	}
	
	int fd = open(filepath, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		tree_clear(&(man->tre));
		free(man->path);
		return;
	}
	
	write(fd, "1", 1);
	write(fd, "\nPath\tUsage\tSize\tAmount\n", 24);
	write(fd, man->path, strlen(man->path));
	
	struct tree *tre = man->tre;
	while (!(tre->info)) {
		for (int i = 0; i < HASH_MOD; ++i) {
			if (tre->hsh.children[i]) {
				tre = tre->hsh.children[i]->node;
				break;
			}
		}
	}
	
	char percbuf[51];
	memset(percbuf, '#', 51);
	percbuf[50] = '\0';
	
	char buf[150];
	memset(buf, 0, 150);
	
	float total = (float)(((struct state *)(tre->info))->size);
	
	if (total < (1 << 10)) {
		snprintf(buf, 150, "\t100%%\t%.2fB\t%s\n", total, percbuf);
	} else if (total < (1 << 20)) {
		snprintf(buf, 150, "\t100%%\t%.2fKB\t%s\n", total / (1 << 10), percbuf);
	} else {
		snprintf(buf, 150, "\t100%%\t%.2fMB\t%s\n", total / (1 << 20), percbuf);
	}
	
	write(fd, buf, strlen(buf));
	
	char path[PATH_MAX];
	memset(path, 0, PATH_MAX);
	path[0] = '/';
	
	tree_write(tre, fd, path, 1, total);
	
	tree_clear(&(man->tre));
	free(man->path);
	
	close(fd);
}


void forks_shutdown(struct forks_manager *man)
{
	shutdown(man->connection.server_fd, SHUT_RDWR);
	tree_clear(&(man->tre));
	man->path = NULL;
	
	return;
}
