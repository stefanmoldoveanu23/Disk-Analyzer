GCC = gcc
CFLAGS = -Wall -ggdb3

manager: manager/manager.c requests_manager.o create_socket.o task.o
	$(GCC) manager/manager.c $(CFLAGS) -o manager/manager manager/requests_manager.o dstructs/create_socket.o dstructs/task.o
	rm *.o
	rm dstructs/*.gch
	rm manager/*.gch

requests_manager.o: treap.o tree.o
	$(GCC) manager/requests_manager.h manager/requests_manager.c $(CFLAGS) -c

create_socket.o:
	$(GCC) dstructs/create_socket.h dstructs/create_socket.c $(CFLAGS) -c

task.o:
	$(GCC) dstructs/task.h dstructs/task.c $(CFLAGS) -c

treap.o:
	$(GCC) dstructs/treap.h dstructs/analysis.h dstructs/analysis.c dstructs/treap.c $(CFLAGS) -c

tree.o:
	$(GCC) dstructs/tree.h dstructs/hash.h dstructs/hash.c dstructs/tree.c $(CFLAGS) -c


