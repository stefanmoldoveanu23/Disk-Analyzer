#ifndef FORKS_MANAGER_H
#define FORKS_MANAGER_H

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "../dstructs/create_socket.h"
#include "../dstructs/tree.h"

struct forks_manager{
	struct socket_connection connection;

	char *path;
	struct tree *tre;
};

int forks_startup(struct forks_manager *man);

int forks_add(struct forks_manager *man, volatile sig_atomic_t *done);

int forks_read_path(struct forks_manager *man);

int forks_solve(struct forks_manager *man);

int forks_write(struct forks_manager *man);

int forks_done(struct forks_manager *man);

void forks_shutdown(struct forks_manager *man);

#endif
