# Disk-Analyzer
OS Project - Disk Analyzer

The project will analyze a disk and offer information.
Options:
1. Begin analysis of directory.
	- You can give a specific priority; default is 2(normal).
2. Remove analysis/results.
3. Suspend/Resume analysis.
4. Print status of analysis(pending, in progress, complete).
5. Print analysis result.
6. List all analysis tasks, with ID and path.

Since some actions can affect others(like suspending an analysis), there will need to be a way for the daemons to communicate with eachother. The easiest way to do that is to create a manager that is running at all times which holds all information, and every time the user calls "da", a request will be sent to the manager. For that purpose, we will use [sockets](https://www.geeksforgeeks.org/socket-programming-cc/).
A manager will receive requests for clients.
Requests:
1. Ask to start analysis with given path.
2. Ask to remove analysis with given ID.
3. Ask to suspend analysis with given ID.
4. Ask to resume analysis with given ID.
5. Request status of analysis with given ID.
6. Request analysis result.
7. Request list of all analysis tasks, with ID and path.

I will also implement multiple data structures:
- data structure for analysis job; will hold flags for status, suspended, etc. It will also have a mutex, because multiple requests might use one analysis at the same time. And, obviously, it will hold the analysis results;
- data structure for requests management; will hold a list of job analyses, a double-linked list of all active thread ids, and a list of all completed thread addresses. Periodically, I will parse the list of completed threads, and using the address I will join those threads and eliminate them from the double-linked list. Since multiple threads will use the completed threads list, I will also need a mutex;
- when closing the computer, there may be job analyses still in progress; in order to not lose that progress, the data will be memorised in a tree data structure, and will be stored in a binary file for easy retrieval;
- further modifications may be made.

TASKS:
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
- [ ] Implement getopt for option handling;
- [ ] Create different branches and have each branch handle specific option tasks;
- TBD
