#ifndef FORKS_MANAGER_H
#define FORKS_MANAGER_H

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "../dstructs/create_socket.h"
#include "../dstructs/tree.h"

struct forks_manager{
	struct socket_connection connection;

	int id;
	char *path;
	struct tree *tre;
};

int forks_startup(struct forks_manager *man);

int forks_add(struct forks_manager *man, volatile sig_atomic_t *done, void (*handler)(int));

int forks_read_progress(struct forks_manager *man);

int forks_read_path(struct forks_manager *man);

int forks_solve(struct forks_manager *man);

void forks_save(struct forks_manager *man);

void forks_write(struct forks_manager *man);

void forks_shutdown(struct forks_manager *man);

#endif
