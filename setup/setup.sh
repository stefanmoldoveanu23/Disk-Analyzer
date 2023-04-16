#!/bin/bash -i

case "$1" in
	paths)
	echo -e "#ifndef PATHS_H\n#define PATHS_H\n\n#define ANALYSES_PATH \"$(realpath data/analyses)\"\n#define DIR_PATH \"$(realpath data)/\"\n\n#endif" > dstructs/paths.h
	;;
	create)
	#sudo cp manager/manager /usr/local/bin/manager_da
	sudo cp client/client /usr/local/bin/da
	;;
	delete)
	#sudo rm /usr/local/bin/manager_da
	sudo rm /usr/local/bin/da
	;;
	*)
	echo "Wrong argument. Need create or delete."
	;;
esac
