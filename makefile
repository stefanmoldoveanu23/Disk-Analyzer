GCC = gcc
CFLAGS = -Wall -ggdb3

manager: manager/manager.c requests_manager.o create_socket.o task.o treap.o tree.o hash.o analysis.o
	$(GCC) manager/manager.c $(CFLAGS) -o manager/manager requests_manager.o create_socket.o task.o treap.o tree.o hash.o analysis.o
	rm *.o
	rm dstructs/*.gch
	rm manager/*.gch

requests_manager.o: dstructs/tree.h dstructs/treap.h dstructs/analysis.h dstructs/hash.h
	$(GCC) manager/requests_manager.h manager/requests_manager.c $(CFLAGS) -c

client: client/client.c options_handler.o create_socket.o task.o
	$(GCC) client/client.c $(CFLAGS) -o client/client options_handler.o create_socket.o task.o
	rm *.o
	rm dstructs/*.gch
	rm client/*.gch

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

analysis.o:
	$(GCC) dstructs/analysis.h dstructs/analysis.c $(CFLAGS) -c

options_handler.o:
	$(GCC) client/options_handler.h client/options_handler.c $(CFLAGS) -c
