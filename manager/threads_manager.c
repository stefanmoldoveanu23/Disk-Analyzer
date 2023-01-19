#include "threads_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define ID_MAX 100000
#define PORT_ACCEPTOR_REQUESTS 8080
#define PORT_REQUEST 8081
#define PORT_ACCEPTOR_RESPONSES 8082

#define ANALYSES_PATH "../data/analyses"
#define DIR_PATH "../data/"


int fork_request(int id, struct analysis *anal)
{
	char request[10 + 10 + strlen(anal->path) + 5];
	memset(request, '\0', 10 + 10 + strlen(anal->path) + 5);
	
	if (snprintf(request, 10 + 10 + strlen(anal->path) + 4, "%010d%010d%s", id, (int)strlen(anal->path), anal->path) < 0) {
		return 1;
	}
	
	struct socket_connection connection;
	if (create_socket_connector(&connection, PORT_REQUEST)) {
		perror(NULL);
		
		return 1;
	}
	
	connection.server_fd = connect(connection.client_fd, (struct sockaddr *)(&connection.address), sizeof(connection.address));
	if (connection.server_fd < 0) {
		perror(NULL);
		close(connection.client_fd);
		return 1;
	}
	
	if (create_socket_send_message(request, connection.client_fd)) {
		perror("Error sending request to fork manager");
		
		close(connection.client_fd);
		return 1;
	}
	
	char vessel[11];
	memset(vessel, '\0', 11);
	
	int left = 10;
	
	while (left) {
		int cnt = read(connection.client_fd, vessel + 10 - left, left);
		
		if (cnt < 0) {
			perror(NULL);
			close(connection.client_fd);

			return 1;
		}
		
		left -= cnt;
	}
	
	anal->pid = (pid_t)atoi(vessel);			
	if (getpgid(getpid()) == getpgid(anal->pid)) {
		setpriority(PRIO_PROCESS, anal->pid, getpriority(PRIO_PROCESS, anal->pid) + 2 - anal->priority);
	}
	
	close(connection.client_fd);
	
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


int threads_startup(struct threads_manager *man)
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
	
	if (create_socket_acceptor(&(man->requests_connection), PORT_ACCEPTOR_REQUESTS)) {
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
	
	if (create_socket_acceptor(&(man->responses_connection), PORT_ACCEPTOR_RESPONSES)) {
		perror("Error creating acceptor socket");
		treap_clear(&(man->available_ids));
		treap_clear(&(man->analyses));
		tree_clear(&(man->paths));
		pthread_mutex_destroy(&(man->analyses_mutex));
		pthread_mutex_destroy(&(man->available_mutex));
		pthread_mutex_destroy(&(man->paths_mutex));
		pthread_mutex_destroy(&(man->socket_mutex));
		shutdown(man->requests_connection.server_fd, SHUT_RDWR);
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
		shutdown(man->requests_connection.server_fd, SHUT_RDWR);
		shutdown(man->responses_connection.server_fd, SHUT_RDWR);
		return -1;
	}

	return 0;
}


int threads_read_results_string(int fd, char *buffer, int sz)
{
	int left = sz;
	while (left) {
		int cnt = read(fd, buffer + sz - left, left);
		if (cnt < 0) {
			break;
		}
	
		left -= cnt;
	}
	
	return (left ? 1 : 0);
}


int threads_read_results(struct threads_manager *man)
{
	
	char buffer[11];
	buffer[10] = '\0';
	
	char result;
	int rd = 1;
	while (1) {
		rd = read(man->responses_connection.client_fd, &result, 1);
		if (rd) {
			break;
		}
	}
	
	if (rd != 1) {
		close(man->responses_connection.client_fd);
		return 1;
	}

	int id, cnt_dirs, cnt_files;
	
	if (threads_read_results_string(man->responses_connection.client_fd, buffer, 10)) {
		close(man->responses_connection.client_fd);
		return 1;
	}
	id = atoi(buffer);
	
	if (result == '0') {
		threads_remove(man, id, -1);
		return 0;
	}
	
	if (threads_read_results_string(man->responses_connection.client_fd, buffer, 10)) {
		close(man->responses_connection.client_fd);
		return 1;
	}
	cnt_dirs = atoi(buffer);
	
	if (threads_read_results_string(man->responses_connection.client_fd, buffer, 10)) {
		close(man->responses_connection.client_fd);
		return 1;
	}
	cnt_files = atoi(buffer);
	
	struct analysis *anal;
	pthread_mutex_lock(&(man->analyses_mutex));
	if (treap_find(man->analyses, id, &anal)) {
		anal->cnt_dirs += cnt_dirs;
		anal->cnt_files += cnt_files;
	} else {
		pthread_mutex_unlock(&(man->analyses_mutex));
		close(man->responses_connection.client_fd);
		return 1;
	}
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	close(man->responses_connection.client_fd);
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


int threads_add(struct threads_manager *man, struct analysis *anal, int fd)
{
	struct stat sb;
	if (stat(anal->path, &sb) || S_ISDIR(sb.st_mode) == 0) {
		perror("Path is not a directory.");
		analysis_path_no_exists(fd, anal->path);
		return 1;
	}
	
	pthread_mutex_lock(&(man->paths_mutex));
	int *other_id;
	if (tree_find_prefix(man->paths, anal->path, (void**)(&other_id))) {
		pthread_mutex_unlock(&(man->paths_mutex));
		perror("Analysis with path already exists.");
		analysis_path_already_exists(fd, anal->path, *other_id);
		return 1;
	}
	pthread_mutex_unlock(&(man->paths_mutex));
	
	pthread_mutex_lock(&(man->available_mutex));
	struct treap *new_id = get_free_id(&(man->available_ids));
	if (!new_id) {
		pthread_mutex_unlock(&(man->available_mutex));
		perror("Could not get new id.");
		analysis_custom_message(fd, "Could not create new id. Try removing some analyses.\n");
		return 1;
	}
	pthread_mutex_unlock(&(man->available_mutex));
	
	int val_id;
	new_id->anal = anal;
	val_id = new_id->id;
	
	int *id = (int *)malloc(sizeof(int));
	if (!id) {
		perror("Error allocating memory.");
		
		pthread_mutex_lock(&(man->available_mutex));
		return_id(&(man->available_ids), new_id);
		pthread_mutex_unlock(&(man->available_mutex));
		
		analysis_custom_message(fd, "There was a memory allocation error. Try again.\n");
		return 1;
	}
	*id = val_id;
	
	pthread_mutex_lock(&(man->paths_mutex));
	if (tree_insert(man->paths, anal->path, (void *)id)) {
		perror("Could not insert path in tree when creating new analysis.");
		pthread_mutex_unlock(&(man->paths_mutex));
		free(id);
		
		pthread_mutex_lock(&(man->available_mutex));
		return_id(&(man->available_ids), new_id);
		pthread_mutex_unlock(&(man->available_mutex));
		
		analysis_custom_message(fd, "There was a memory allocation error. Try again.\n");
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
		
		analysis_custom_message(fd, "There was an error starting the analysis process. Try again.\n");
		return 1;
	}
	
	pthread_mutex_lock(&(man->analyses_mutex));
	++man->analysis_cnt;
	analysis_created(fd, val_id, anal);
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	return 0;
}


void threads_suspend(struct threads_manager *man, const int id, int fd)
{
	struct analysis *anal;
	pthread_mutex_lock(&(man->analyses_mutex));
	
	if (treap_find(man->analyses, id, &anal)) {
		if (anal->suspended != ANALYSIS_SUSPENDED && anal->status != ANALYSIS_COMPLETE && getpgid(getpid()) == getpgid(anal->pid)) {
			kill(anal->pid, SIGTSTP);
			anal->total_time += (time(NULL) - anal->last_start);
			anal->suspended = ANALYSIS_SUSPENDED;
			analysis_suspended(fd, id, anal);
		} else if (anal->suspended == ANALYSIS_SUSPENDED) {
			analysis_already_suspended(fd, id, anal);
		} else {
			analysis_already_done(fd, id, anal);
		}
		
	} else {
		analysis_id_no_exists(fd, id);
	}
	
	pthread_mutex_unlock(&(man->analyses_mutex));
}


void threads_resume(struct threads_manager *man, const int id, int fd)
{
	struct analysis *anal;
	pthread_mutex_lock(&(man->analyses_mutex));
	
	if (treap_find(man->analyses, id, &anal)) {
		if (anal->suspended != ANALYSIS_RESUMED && anal->status != ANALYSIS_COMPLETE && getpgid(getpid()) == getpgid(anal->pid)) {
			kill(anal->pid, SIGCONT);
			anal->last_start = time(NULL);
			anal->suspended = ANALYSIS_RESUMED;
			analysis_resumed(fd, id, anal);
		} else if (anal->suspended == ANALYSIS_RESUMED) {
			analysis_already_resumed(fd, id, anal);
		} else {
			analysis_already_done(fd, id, anal);
		}
		
	} else {
		analysis_id_no_exists(fd, id);
	}
	
	pthread_mutex_unlock(&(man->analyses_mutex));
}


void threads_remove(struct threads_manager *man, const int id, int fd)
{
	
	pthread_mutex_lock(&(man->analyses_mutex));
	struct treap *trp = treap_extract(&(man->analyses), id);
	if (trp) {
		--man->analysis_cnt;
	}
	pthread_mutex_unlock(&(man->analyses_mutex));
	
	if (trp) {
		char name[20];
		memset(name, 0, 20);
		snprintf(name, 20, "%s%d", DIR_PATH, id);
		
		remove(name);
		
		if (getpgid(getpid()) == getpgid(trp->anal->pid)) {
			kill(trp->anal->pid, SIGINT);
		}

		pthread_mutex_lock(&(man->paths_mutex));
		tree_remove(man->paths, trp->anal->path);
		pthread_mutex_unlock(&(man->paths_mutex));
		
		
		pthread_mutex_lock(&(man->available_mutex));
		treap_insert_node(&(man->available_ids), trp);
		pthread_mutex_unlock(&(man->available_mutex));

		analysis_removed(fd, id, trp->anal);
		free(trp->anal->path);
		free(trp->anal);
		trp->anal = NULL;
	} else {
		analysis_id_no_exists(fd, id);
	}
}


void threads_shutdown(struct threads_manager *man)
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

	shutdown(man->requests_connection.server_fd, SHUT_RDWR);
	shutdown(man->responses_connection.server_fd, SHUT_RDWR);
}
