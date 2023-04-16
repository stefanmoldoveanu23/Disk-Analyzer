GCC = gcc
CFLAGS = -Wall -ggdb3
ADDFLAGS = -lpthread

setup: setup_paths setup_execs
	setup/service_setup.sh create_service
	setup/setup.sh create

setup_paths:
	setup/setup.sh paths

setup_execs: manager client clean

open:
	sudo systemctl start manager_da

close:
	sudo systemctl stop manager_da

manager: manager/manager.c threads_manager.o forks_manager.o create_socket.o task.o treap.o tree.o hash.o analysis.o
	$(GCC) manager/manager.c $(CFLAGS) $(ADDFLAGS) -o manager/manager threads_manager.o forks_manager.o create_socket.o task.o treap.o tree.o hash.o analysis.o

threads_manager.o: dstructs/tree.h dstructs/treap.h dstructs/analysis.h dstructs/hash.h dstructs/paths.h
	$(GCC) manager/threads_manager.h manager/threads_manager.c $(CFLAGS) -c

forks_manager.o: dstructs/tree.h dstructs/create_socket.h dstructs/paths.h
	$(GCC) manager/forks_manager.h manager/forks_manager.c $(CFLAGS) -c

client: client/client.c options_handler.o create_socket.o task.o
	$(GCC) client/client.c $(CFLAGS) -o client/client options_handler.o create_socket.o task.o

create_socket.o:
	$(GCC) dstructs/create_socket.h dstructs/create_socket.c $(CFLAGS) -c

task.o:
	$(GCC) dstructs/task.h dstructs/task.c $(CFLAGS) -c

treap.o: dstructs/analysis.h
	$(GCC) dstructs/treap.h dstructs/treap.c $(CFLAGS) -c

tree.o: dstructs/hash.h
	$(GCC) dstructs/tree.h dstructs/tree.c $(CFLAGS) -c

hash.o:
	$(GCC) dstructs/hash.h dstructs/hash.c $(CFLAGS) -c

analysis.o: dstructs/paths.h
	$(GCC) dstructs/analysis.h dstructs/analysis.c $(CFLAGS) -c

options_handler.o:
	$(GCC) client/options_handler.h client/options_handler.c $(CFLAGS) -c

paths.o:
	$(GCC) dstructs/paths.h $(CFLAGS) -c

clean:
	rm *.o
	rm dstructs/*.gch
	rm manager/*.gch
	rm client/*.gch
