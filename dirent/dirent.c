#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include "string.h"
#include "errno.h"
#include "limits.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Wrong amount of arguments.\n");
		return 1;
	}

	DIR *d;
	struct dirent *dir;
	d = opendir(argv[1]);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			char buf[PATH_MAX + 1];
			buf[0] = '\0';
			strcat(buf, argv[1]);
			strcat(buf, "/");
			strcat(buf, dir->d_name);

			struct stat stats;
			if (stat(buf, &stats)) {
				perror(NULL);
			} else {
				printf("%s %ld\n", dir->d_name, stats.st_size);
			}
		}
		closedir(d);
	}

	return 0;
}
