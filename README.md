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

A manager will receive requests for clients.
Requests:
1. Ask if given path is child of an already existing analysis.
2. Ask to remove analysis with given ID.
3. Ask to suspend analysis with given ID.
4. Ask to resume analysis with given ID.
5. Request status of analysis with given ID.
6. Request analysis result.
7. Request list of all analysis tasks, with ID and path.

TASKS:
- [x] Learn daemons;
- [x] Test using multiple .c files for single .h file;
- [x] Test using getcwd for caller working directory;
- [x] Implement socket communication(with daemons):
	- [x] Implement multiple client management;
	- [x] Introduce threads to multiple client management;
- [x] Test using dirent to parse through directory contents;
- [ ] Implement getopt for option handling;
- [ ] Implement data structure for information management:
	- [x] Implement header file;
	- [ ] Implement source file;
- [ ] Create different branches and have each branch handle specific option tasks;
- TBD
