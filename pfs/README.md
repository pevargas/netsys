User Datagram Protocol Assignment
=================================

The requirements for this program is to

* implement a file location server over TCP (server_PFS)
* implement a peer to peer file distribution service over TCP (client_PFS)

The program is a peer-to-peer file distribution service. Each client will register its name and its files with the locatioin server. The location server districutes the complete list of filenames and file locations to each client. Clients then got directly to each peer to obtain the file they desire.

**Patrick E. Vargas**

*  BS Computer Science, Software Engineering  
   *  Atmospheric and Oceanic Science Minor  
   *  Technology, Arts and Media Minor  
*  (720) 515 - 6402  
*  [www.vargascorpus.com][w]  
*  [patrick.vargas@colorado.edu][e]  

Included Files
--------------

1. client_PFS.c
2. server_PFS.c
3. athena.h
4. makefile
5. README.txt (this file)

Commands
--------
### To Run the Program ###

1. Create the executables `client` and `server` with the following command:  

	  	make

2. Run the server program, replacing <port> with a number between 1024 and 65535:

	 	./server_PFS <Port Number>

3. Run `hostname -i` to get the IP address of the server. Open up a new terminal. Use the found address and the port number you used in the following command in the new terminal:

	 	./client_PFS <Client Name> <Server IP> <Server Port>

### While running the client ####

* Register name with file location server
* Occurs on startup
* Register files with file location server
* Request master file list from file location server `ls`
* Print master file list
* Get file from another client `get`
* Provide file to another client
* Exit current client `exit`

### Other Useful Commands ###

1. `make clean` to create clean workspace.

Acknowledgments
---------------

This assignment is based on the my first assignment, "Programming Assignment 1 - UDP Socket Programming", University of Colorado Boulder.

Notes 
-----

  [w]: http://www.vargascorpus.com/
  [e]: mailto:patrick.vargas@colorado.edu
