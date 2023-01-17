#include "requests_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define ID_MAX 100000
#define PORT_ACCEPTOR 8080
#define PORT_REQUEST 8081

#define ANALYSES_PATH "../data/analyses"


int fork_request(int id, struct analysis *anal)
{
	char *request = (char*)malloc(10 + 10 + strlen(anal->path) + 5);
	memset(request, '\0', 10 + 10 + strlen(anal->path) + 5);
	
	if (snprintf(request, 10 + 10 + strlen(anal->path) + 4, "%010d%010d%s", id, (int)strlen(anal->path), anal->path) < 0) {
		return 1;
	}
	
	struct socket_connection connection;
	if (create_socket_connector(&connection, PORT_REQUEST)) {
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
			
			return 1;
		}
		
		pos += sent;
	}
	
	free(request);
	
	char vessel[11];
	memset(vessel, '\0', 11);
	
	int left = 10;
	
	while (left) {
		int cnt = read(connection.client_fd, vessel + 10 - left, left);
		
		if (cnt < 0) {
			perror(NULL);
			close(connection.server_fd);
			shutdown(connection.client_fd, SHUT_RDWR);

			return 1;
		}
		
		left -= cnt;
	}
	
	anal->pid = (pid_t)atoi(vessel);
	
	close(connection.server_fd);
	shutdown(connection.client_fd, SHUT_RDWR);
	
	return 0;
}


int initial_forks(struct treap *trp)
{
	if (!trp) {
		return 0;
	}
	
	if (fork_request(trp->id, trp->anal)) {
		return 1;
	}
	
	return initial_forks(trp->chld_left) || initial_forks(trp->chld_right);
}


int requests_startup(struct requests_manager *man)
{
	srand(time(NULL));
	
	int fd = open(ANALYSES_PATH, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		perror("Error opening data file");
		return -1;
	}
	
	if (pthread_mutex_init(&(man->analyses_mutex), NULL)) {
		perror("Error initializing analyses mutex");
		close(fd);
		return -1;
	}
	
	if (pthread_mutex_init(&(man->available_mutex), NULL)) {
		perror("Error initializing available ids mutex");
		close(fd);
		pthread_mutex_destroy(&(man->analyses_mutex));
		return -1;
	}
	
	if (pthread_mutex_init(&(man->paths_mutex), NULL)) {
		perror("Error initializing paths mutex.");
		close(fd);
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		return -1;
	}
	
	if (pthread_mutex_init(&(man->socket_mutex), NULL)) {
		perror("Error initializing socket mutex.");
		close(fd);
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
	}
	
	char *string = (char *)malloc(11);
	if (!string) {
		perror("Error allocating memory to string in order to read amount of saved analyses");
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		close(fd);
		return -1;
	}
	memset(string, '\0', 11);
	
	int total_read = read(fd, string, 10);
	if (total_read < 0) {
		perror("Error reading amount of saved analyses");
		free(string);
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		close(fd);
		return -1;
	} else if (!total_read) {
		man->analysis_cnt = 0;
	} else {
		man->analysis_cnt = atoi(string);
	}
	free(string);
	
	if (tree_init(&(man->paths))) {
		perror("Error initializing tree for analyses paths");
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		close(fd);
		return -1;
	}
	
	man->analyses = NULL;
	
	int id, mx_id = 0;
	
	for (int i = 0; i < man->analysis_cnt; ++i) {
		struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
		if (!anal) {
			perror("Error allocating memory to anal variable");
			treap_clear(&(man->analyses));
			tree_clear(&(man->paths));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			pthread_mutex_destroy(&(man->paths_mutex));
			pthread_mutex_destroy(&(man->socket_mutex));
			close(fd);
			return -1;
		}
		
		if (analysis_read(&id, anal, fd)) {
			perror("Error reading analysis from data file");
			free(anal);
			treap_clear(&(man->analyses));
			tree_clear(&(man->paths));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			pthread_mutex_destroy(&(man->paths_mutex));
			pthread_mutex_destroy(&(man->socket_mutex));
			close(fd);
			return -1;
		}
		if (treap_insert_new(&(man->analyses), id, anal)) {
			perror("Error inserting analysis in treap");
			free(anal);
			treap_clear(&(man->analyses));
			tree_clear(&(man->paths));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			pthread_mutex_destroy(&(man->paths_mutex));
			pthread_mutex_destroy(&(man->socket_mutex));
			close(fd);
			return -1;
		}
		
		int *idp = (int *)malloc(sizeof(int));
		if (!idp) {
			perror("Error allocating memory to id pointer");
			treap_clear(&(man->analyses));
			tree_clear(&(man->paths));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			pthread_mutex_destroy(&(man->paths_mutex));
			pthread_mutex_destroy(&(man->socket_mutex));
			close(fd);
			return -1;
		}
		
		*idp = id;
		if (tree_insert(man->paths, anal->path, idp)) {
			perror("Error inserting analysis in treap");
			free(idp);
			treap_clear(&(man->analyses));
			tree_clear(&(man->paths));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			pthread_mutex_destroy(&(man->paths_mutex));
			pthread_mutex_destroy(&(man->socket_mutex));
			close(fd);
			return -1;
		}
		
		if (id > mx_id) {
			mx_id = id;
		}
	}
	close(fd);
	
	man->available_ids = NULL;
	
	for (int i = 1; i <= mx_id + 1; ++i) {
		if (i > ID_MAX) {
			break;
		}

		if (!treap_find(man->analyses, i, NULL)) {
			if (treap_insert_new(&(man->available_ids), i, NULL)) {
				perror("Error inserting id in treap");
				treap_clear(&(man->available_ids));
				treap_clear(&(man->analyses));
				tree_clear(&(man->paths));
				pthread_mutex_destroy(&(man->analyses_mutex));
				pthread_mutex_destroy(&(man->available_mutex));
				pthread_mutex_destroy(&(man->paths_mutex));
				pthread_mutex_destroy(&(man->socket_mutex));
				return -1;
			}
		}
	}
	
	if (create_socket_acceptor(&(man->connection), PORT_ACCEPTOR)) {
		perror("Error creating acceptor socket");
		treap_clear(&(man->available_ids));
		treap_clear(&(man->analyses));
		tree_clear(&(man->paths));
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		return -1;
	}

	if (initial_forks(man->analyses)) {
		perror("Error creating startup forks.");
		treap_clear(&(man->available_ids));
		treap_clear(&(man->analyses));
		tree_clear(&(man->paths));
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		shutdown(man->connection.server_fd, SHUT_RDWR);
	}

	return 0;
}


struct treap *get_free_id(struct treap **trp)
{
	if (!(*trp)) {
		perror("There are no more ids available. Remove some analyses!");
		return NULL;
	}
	
	if (!((*trp)->chld_right)) {
		if (!((*trp)->chld_left)) {
			int id = (*trp)->id;
			
			if (id < ID_MAX) {
				if (treap_insert_new(&(*trp), id + 1, NULL)) {
					perror("Error inserting new id into available_ids.");
					return NULL;
				}
			}
			
			return treap_extract(trp, id);
		} else {
			return treap_extract(trp, (*trp)->chld_left->id);
		}
	} else {
		return treap_extract(trp, (*trp)->id);
	}
}


void return_id(struct treap **trp, struct treap *node) {
	if ((*trp) && !((*trp)->chld_left) && !((*trp)->chld_right)) {
		treap_remove(trp, (*trp)->id);
	}
	
	node->anal = NULL;
	treap_insert_node(trp, node);
}


int requests_add(struct requests_manager *man, struct analysis *anal)
{
	struct stat sb;
	if (stat(anal->path, &sb) || S_ISDIR(sb.st_mode) == 0) {
		perror("Path is not a directory.");
		return 1;
	}
	
	pthread_mutex_lock(&(man->paths_mutex));
	if (tree_find_prefix(man->paths, anal->path, NULL)) {
		perror("Analysis with path already exists.");
		return 1;
	}
	pthread_mutex_unlock(&(man->paths_mutex));
	
	pthread_mutex_lock(&(man->available_mutex));
	struct treap *new_id = get_free_id(&(man->available_ids));
	if (!new_id) {
		perror("Could not get new id.");
		return 1;
	}
	pthread_mutex_unlock(&(man->available_mutex));
	
	new_id->anal = anal;
	
	int *id = (int *)malloc(sizeof(int));
	if (!id) {
		perror("Error allocating memory.");
		
		pthread_mutex_lock(&(man->available_mutex));
		return_id(&(man->available_ids), new_id);
		pthread_mutex_unlock(&(man->available_mutex));
		return 1;
	}
	*id = new_id->id;
	
	pthread_mutex_lock(&(man->paths_mutex));
	if (tree_insert(man->paths, anal->path, (void *)id)) {
		perror("Could not insert path in tree when creating new analysis.");
		free(id);
		
		pthread_mutex_lock(&(man->available_mutex));
		return_id(&(man->available_ids), new_id);
		pthread_mutex_unlock(&(man->available_mutex));
		return 1;
	}
	pthread_mutex_unlock(&(man->paths_mutex));
	
	pthread_mutex_lock(&(man->analyses_mutex));
	anal->last_start = time(NULL);
	treap_insert_node(&(man->analyses), new_id);
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	if (fork_request(*id, anal)) {
		perror("Could not communicate with forking manager.");
		
		pthread_mutex_lock(&(man->analyses_mutex));
		struct treap *trp = treap_extract(&(man->analyses), new_id->id);
		pthread_mutex_unlock(&(man->analyses_mutex));
		
		pthread_mutex_lock(&(man->paths_mutex));
		tree_remove(man->paths, anal->path);
		pthread_mutex_unlock(&(man->paths_mutex));
		
		pthread_mutex_lock(&(man->available_mutex));
		return_id(&(man->available_ids), trp);
		pthread_mutex_unlock(&(man->available_mutex));
		return 1;
	}
	
	pthread_mutex_lock(&(man->analyses_mutex));
	++man->analysis_cnt;
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	return 0;
}


void requests_shutdown(struct requests_manager *man)
{
	int fd = open(ANALYSES_PATH, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	char cnt[15] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	snprintf(cnt, 11, "%.010d", man->analysis_cnt);
	write(fd, cnt, 10);
	
	treap_save(&(man->analyses), fd);
	close(fd);
	
	tree_clear(&(man->paths));
	treap_clear(&(man->available_ids));
	
	pthread_mutex_destroy(&(man->analyses_mutex));
	pthread_mutex_destroy(&(man->available_mutex));
	pthread_mutex_destroy(&(man->paths_mutex));
	pthread_mutex_destroy(&(man->socket_mutex));

	shutdown(man->connection.server_fd, SHUT_RDWR);
}
