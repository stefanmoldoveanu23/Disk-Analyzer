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


int fork_request(struct analysis *anal, int is_startup)
{
	/* COMPLETE AFTER IMPLEMENTING FORK MANAGER */
	return 0;
}


int initial_forks(struct treap *trp)
{
	if (!trp) {
		return 0;
	}
	
	if (fork_request(trp->anal, 1)) {
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
	
	char *string = (char *)malloc(11);
	if (!string) {
		perror("Error allocating memory to string in order to read amount of saved analyses");
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
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
	char *fullpath = realpath(anal->path, NULL);
	if (fullpath == NULL) {
		perror(NULL);
		return 1;
	}
	
	free(anal->path);
	anal->path = fullpath;
	
	struct stat sb;
	if (stat(fullpath, &sb) || S_ISDIR(sb.st_mode) == 0) {
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
		return_id(&(man->available_ids), new_id);
		return 1;
	}
	
	pthread_mutex_lock(&(man->paths_mutex));
	if (tree_insert(man->paths, anal->path, (void *)id)) {
		perror("Could not insert path in tree when creating new analysis.");
		free(id);
		return_id(&(man->available_ids), new_id);
		return 1;
	}
	pthread_mutex_unlock(&(man->paths_mutex));
	
	pthread_mutex_lock(&(man->analyses_mutex));
	treap_insert_node(&(man->analyses), new_id);
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	if (fork_request(anal, 0)) {
		perror("Could not communicate with forking manager.");
		treap_remove(&(man->analyses), new_id->id);
		tree_remove(man->paths, anal->path);
		return_id(&(man->available_ids), new_id);
		return 1;
	}
	
	++man->analysis_cnt;
	anal->last_start = time(NULL);
	
	return 0;
}


void requests_shutdown(struct requests_manager *man)
{
	/* IT IS GOING TO BE MORE COMPLEX, BUT I NEED TO IMPLEMENT THE FORK MANAGER FIRST */
	int fd = open(ANALYSES_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
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

	shutdown(man->connection.server_fd, SHUT_RDWR);
}
