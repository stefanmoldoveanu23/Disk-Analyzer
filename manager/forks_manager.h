#ifndef FORKS_MANAGER_H
#define FORKS_MANAGER_H

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include "../dstructs/create_socket.h"
#include "../dstructs/tree.h"

struct forks_manager{
	volatile sig_atomic_t *done;
	volatile sig_atomic_t *interrupted;
	volatile sig_atomic_t *suspended;
	void (*handler)(int);
	
	struct socket_connection connection;

	int id;
	
	int last_send;
	int cnt_dirs;
	int cnt_files;
	
	char *path;
	struct tree *tre;
};

int forks_startup(struct forks_manager *man);

int forks_add(struct forks_manager *man);

int forks_read_progress(struct forks_manager *man);

int forks_read_path(struct forks_manager *man);

int forks_send_pid(struct forks_manager *man);

int forks_send_result(struct forks_manager *man, int result);

int forks_solve(struct forks_manager *man);

void forks_save(struct forks_manager *man);

void forks_write(struct forks_manager *man);

void forks_shutdown(struct forks_manager *man);

#endif
