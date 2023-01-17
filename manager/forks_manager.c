#include "forks_manager.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT_ACCEPTOR 8081
#define DIR_PATH "../data/"


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
	
	man->path = NULL;
	return 0;
}

int forks_add(struct forks_manager *man, volatile sig_atomic_t *done, void (*handler)(int))
{
	pid_t pid = fork();
	if (pid == -1) {
		return -1;
	}
	
	if (pid) {
		close(man->connection.client_fd);
		return 1;
	}
	
	signal(SIGTERM, handler);
	
	if (forks_read_path(man)) {
		perror("Could not read path of new fork.");
		close(man->connection.client_fd);
		tree_clear(&(man->tre));
		return -1;
	}
	
	if (forks_send_pid(man)) {
		perror("Could not send back pid.");
		close(man->connection.client_fd);
		tree_clear(&(man->tre));
		return -1;
	}
	
	
	close(man->connection.client_fd);


	if (forks_read_progress(man)) {
		perror("Could not read the progress of new fork.");
		close(man->connection.client_fd);
		tree_clear(&(man->tre));
		free(man->path);
		return -1;
	}
	
	
	if (forks_solve(man, done)) {
		perror("Could not do this.");
	} else {
		
	}
	
	
	
	forks_save(man);
	
	*done = 1;
	return 0;
}

int forks_read_progress(struct forks_manager *man)
{
	char filepath[PATH_MAX];
	memset(filepath, '\0', PATH_MAX);
	
	snprintf(filepath, 8 + strlen(man->path) + 1, "%s%d", DIR_PATH, man->id);
	
	int fd = open(filepath, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		return 1;
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


int forks_solve(struct forks_manager *man, volatile sig_atomic_t *done)
{
	if (tree_insert(man->tre, man->path, NULL)) {
		return 1;
	}
	
	struct tree *curr = man->tre;
	while (1) {
		for (int i = 0; i < HASH_MOD; ++i) {
			if (curr->hsh.children[i]) {
				curr = curr->hsh.children[i]->node;
				continue;
			}
		}
		break;
	}
	
	char * const paths[2] = {man->path, NULL};
	
	FTS *ftsp = fts_open(paths, FTS_PHYSICAL | FTS_SEEDOT, NULL);
	if (!ftsp) {
		return 1;
	}
	
	FTSENT *file;
	
	while (!(*done)) {
		file = fts_read(ftsp);
		if (!file) {
			break;
		}
		
		struct state *st = (struct state *)malloc(sizeof(struct state));
		if (!st) {
			perror("Error when creating new state.");
			fts_close(ftsp);
			return 1;
		}
		
		st->size = 0;
		st->done = 0;
		
		if (file->fts_info == FTS_D) {
			if (tree_insert(man->tre, file->fts_path, st)) {
				perror("Error when inserting new path to tree.");
				fts_close(ftsp);
				return 1;
			}
		}
	}

	fts_close(ftsp);
	
	return 0;
}


void forks_save(struct forks_manager *man)
{
	char filepath[PATH_MAX];
	memset(filepath, '\0', PATH_MAX);
	
	snprintf(filepath, 8 + strlen(man->path) + 1, "%s%d", DIR_PATH, man->id);
	int fd = open(filepath, O_WRONLY);
	if (fd == -1) {
		tree_clear(&(man->tre));
		return;
	}
	
	write(fd, "0", 1);
	
	tree_save(&(man->tre), fd);
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
