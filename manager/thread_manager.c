#include "thread_manager.h"

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

#define ANALYSES_PATH "../data/analyses"


int fork_request(int id, struct analysis *anal)
{
	/* COMPLETE AFTER IMPLEMENTING FORK MANAGER */
	return 0;
}


void forks_startup(struct treap *trp)
{
	if (!trp) {
		return;
	}
	
	fork_request(trp->id, trp->anal);
	
	forks_startup(trp->chld_left);
	forks_startup(trp->chld_right);
}


int handle_startup(struct thread_manager *man)
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
	
	int cnt;
	char *string = (char *)malloc(11);
	if (!string) {
		perror("Error allocating memory to string in order to read amount of saved analyses");
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
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
		close(fd);
		return -1;
	} else if (!total_read) {
		cnt = 0;
	} else {
		cnt = atoi(string);
	}
	free(string);
	
	man->analyses = NULL;
	
	int id, mx_id = -1;
	
	while (cnt--) {
		struct analysis *anal = (struct analysis *)malloc(sizeof(struct analysis));
		if (!anal) {
			perror("Error allocating memory to anal variable");
			clear_treap(&(man->analyses));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			close(fd);
			return -1;
		}
		
		if (analysis_read(&id, anal, fd)) {
			perror("Error reading analysis from data file");
			free(anal);
			clear_treap(&(man->analyses));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
			close(fd);
			return -1;
		}
		if (insert_treap_new(&(man->analyses), id, anal)) {
			perror("Error inserting analysis in treap");
			free(anal);
			clear_treap(&(man->analyses));
			pthread_mutex_destroy(&(man->analyses_mutex));
			pthread_mutex_destroy(&(man->available_mutex));
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

		if (find_treap(man->analyses, i, NULL)) {
			if (insert_treap_new(&(man->available_ids), i, NULL)) {
				perror("Error inserting id in treap");
				clear_treap(&(man->available_ids));
				clear_treap(&(man->analyses));
				pthread_mutex_destroy(&(man->analyses_mutex));
				pthread_mutex_destroy(&(man->available_mutex));
				return -1;
			}
		}
	}
	
	if (create_socket_acceptor(&(man->connection), PORT_ACCEPTOR)) {
		perror("Error creating acceptor socket");
		clear_treap(&(man->available_ids));
		clear_treap(&(man->analyses));
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		return -1;
	}

	forks_startup(man->analyses);

	return 0;
}


void handle_shutdown(struct thread_manager *man)
{
	/* IT IS GOING TO BE MORE COMPLEX, BUT I NEED TO IMPLEMENT THE FORK MANAGER FIRST */
	int fd = open(ANALYSES_PATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	save_treap(&(man->analyses), fd);
	close(fd);
	
	clear_treap(&(man->available_ids));
	
	pthread_mutex_destroy(&(man->analyses_mutex));
	pthread_mutex_destroy(&(man->available_mutex));

	shutdown(man->connection.server_fd, SHUT_RDWR);
}
