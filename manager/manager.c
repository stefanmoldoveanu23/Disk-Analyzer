#include "../task/task.h"
#include "../socket/create_socket.h"

#include <sys/types.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
	daemon(1, 1);
	
	int server_fd = create_socket_acceptor(PORT);
	int client_fd = accept(
}
