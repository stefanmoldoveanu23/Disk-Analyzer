#include <signal.h>

int main() {
	kill(7653, SIGTERM);
	return 0;
}
