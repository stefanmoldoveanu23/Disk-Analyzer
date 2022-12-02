# Disk-Analyzer
OS Project - Disk Analyzer

The project will analyze a disk using the [fts library](https://man7.org/linux/man-pages/man3/fts.3.html) and offer information.
## Options:
1. Begin analysis of directory.
	- You can give a specific [priority](https://linux.die.net/man/2/setpriority); default is 2(normal).
2. Remove analysis/results.
3. Suspend/Resume analysis.
4. Print status of analysis(pending, in progress, complete).
5. Print analysis result.
6. List all analysis tasks, with ID and path.

Options will be handled using the [getopt](https://man7.org/linux/man-pages/man3/getopt.3.html) functionalities.

Since some actions can affect others(like suspending an analysis), there will need to be a way for the daemons to communicate with eachother. The easiest way to do that is to create a manager that is running at all times which holds all information, and every time the user calls "da", a request will be sent to the manager. For that purpose, we will use [sockets](https://www.geeksforgeeks.org/socket-programming-cc/).
A manager will receive requests for clients.
## Requests:
1. Ask to start analysis with given path.
2. Ask to remove analysis with given ID.
3. Ask to suspend analysis with given ID.
4. Ask to resume analysis with given ID.
5. Request status of analysis with given ID.
6. Request analysis result.
7. Request list of all analysis tasks, with ID and path.

## I will also implement multiple data structures:
- data structure for analysis job; will hold flags for status, suspended, etc. It will also have a mutex, because multiple requests might use one analysis at the same time. It will also hold the file descriptor for the file that holds the result;
- data structure for requests management; will hold a list of job analyses, a double-linked list of all active thread ids, and a list of all completed thread addresses. Periodically, I will parse the list of completed threads, and using the address I will join those threads and eliminate them from the double-linked list. Since multiple threads will use the completed threads list, I will also need a mutex;
- when closing the computer, there may be job analyses still in progress; in order to not lose that progress, the data will be memorised in a tree data structure, and will be stored in a file for easy retrieval;
- further modifications may be made.

## How does it work?
![alt text](https://user-images.githubusercontent.com/100515480/205260201-cb1b4e32-ef8e-43c3-bbde-78d56a4900d9.png)
Because analyses can have different priorities, they have to be made in different processes. However, if a request turns into a separate process, the only ways for two processes to communicate are through shared memory, which is limited and clunky, or through sockets, which is more manageable.

However, the analysis results processes connecting to the same socket that clients do will cause the requests to get processed slower; we will handle that by creating two sockets: one handles client, the other one handles results. These two sockets will accept connections in two different threads.

The finished analyses thread just needs to read an id and change the analysis' status to "done", so it can just do that serially.

The requests thread however might need to send a result back to the client, so it is better in this case to create new threads that handle a request.

If a requests asks for a new analysis, the thread will get an unused ID, it will send te corresponding message to the client, and only then it will send the request to start the analysis. The thread doesn't need to wait for the analysis to finish, so it will just close. The fork manager, after forking, will send the PID of the new analysis to the result thread, so that the analysis can be suspended, resumed or deleted directly by the request solver thread.

For all other requests, the thread will solve it by itself.

After finishing an analysis, the process will write the results in a file, after which it will send a request to the complete analyses socket to check the analysis as "done".

### How does it handle shutdown
At shutdown, the threads manager process will receive a SIGTERM signal. When a process receives a signal, it will be caught by any of the threads that don't block it. I want specifically the main thread to catch it, so the results thread, the request thread, and the request solver threads will all block SIGTERM.

The main thread will manually send the SIGINT signal to the results and request threads with [pthread_kill](https://man7.org/linux/man-pages/man3/pthread_kill.3.html).

The results thread will just solve the current result and close. It's easy to handle because it works serially.

The requests thread will send call [pthread_cancel](https://man7.org/linux/man-pages/man3/pthread_cancel.3.html). The reason for that is because some threads will have to change flags, which needs to be done before the threads is closed. Threads can suspend cancellation by using the [pthread_setcancelstate and pthread_setcanceltype](https://man7.org/linux/man-pages/man3/pthread_setcancelstate.3.html).

At the initial fork, the threads manager will be the parent, so it will have access to the pid of the fork manager. Because of that, the thread manager process will send the SIGTERM signal to the fork manager process, which will further send it's signal to all of its children(either with the pid of 0, which sends the signal to all processes in the same control group, or by just memorizing all child processes and sending the signal to them one by one).

Each analysis will handle SIGTERM by immediatly exiting out of the analysis function, and writing the progress into a file. When starting the computer again, the request manager will look for all of the unfinished analyses and send requests to the fork manager. When an analysis normally ends, it writes the result into that same folder, so if at shutdown an analysis ended but didn't have time to send the message to the result manager, at startup it will be requested to do an analysis aready finished, so it will need a way to differentiate between a string that represents an analysis result or an analysis progress.

When reading an analysis result, input needs to know first how long it is, in order to know how muchi it should read. Because of that, the first for bytes at the start of the file will represent an int equal to the size of the result. If the file contains the progress instead, the first 4 bytes will represent an int equal to -1. This way the analysis process can differentiate between the two. 


A signal handler function can only do asynchronous actions, because it halts the program even if it is inside an operation. If we want to interact with the rest of the program, we will need to use the [sig_atomic_t](https://www.alphacodingskills.com/c/notes/c-signal-sig-atomic-t.php) type. A variable of this type can safely be modified inside of a signal handler.

## TASKS:
- [x] Learn daemons;
	- Intentionally [orphaned processes](https://stackoverflow.com/a/17955149). Only the parent process runs in the foreground, but the child processes will run in the background.
- [x] Test using getcwd for caller working directory;
	- The "da" command will run an executable from a different directory. If user doesn't provide full path, I will use getcwd in order to complete it. However, I needed to check if getcwd returned the current directory of the caller, or of the executable.
- [x] Implement socket communication(with daemons):
	- [x] Implement multiple client management;
	- [x] Introduce threads to multiple client management;
- [x] Test using dirent to parse through directory contents;
- [x] Implement systemd service to execute program at startup and shutdown;
	- Since the manager needs to run at all times, there needs to be a way to make it run automatically at system startup, and a way to ask it to close itself(and handle pending requests) at shutdown. At shutdown it will send the manager(and its children) the SIGTERM signal.
- [ ] Implement data structures for information management:
	- [x] Implement header file;
	- [ ] Implement source file;
- [ ] Implement client functionality;
	- [ ] Implement getopt for option handling;
- [ ] Implement manager functionality:
	- [ ] Implement thread manager:
		- [ ] Implement request thread;
		- [ ] Implement results thread;
		- [ ] Handle startup;
		- [ ] Handle shutdown;
	- [ ] Implement fork manager:
		- [ ] Implement analysis that handles interruption;
		- [ ] Handle shutdown;
- TBD
