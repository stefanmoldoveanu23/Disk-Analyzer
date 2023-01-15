#include "forks_manager.h"

#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#define PORT_ACCEPTOR 8081

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

int forks_add(struct forks_manager *man, volatile sig_atomic_t *done)
{
	pid_t pid = fork();
	if (pid == -1) {
		return 1;
	}
	
	if (pid) {
		close(man->connection.client_fd);
		return 1;
	}
	
	while (!(*done));
	
	close(man->connection.client_fd);
	tree_clear(&(man->tre));
	
	*done = 1;
	return 0;
}

void forks_shutdown(struct forks_manager *man)
{
	shutdown(man->connection.server_fd, SHUT_RDWR);
	tree_clear(&(man->tre));
	man->path = NULL;
	
	return;
}
