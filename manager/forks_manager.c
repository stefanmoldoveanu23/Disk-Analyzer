#include "forks_manager.h"

#define PORT_ACCEPTOR 8081

int forks_startup(struct forks_manager *man)
{
	if (tree_init(&(man->tre))) {
		perror(NULL);
		return 1;
	}
	
	if (create_socket_acceptor(man->connection, PORT_ACCEPTOR)) {
		perror(NULL);
		tree_clear(&(man->tre));
		return 1;
	}
	
	path = NULL;
	return 0;
}
