Peer-To-Peer File Server Assignment
===================================

The requirements for this program is to

* implement a file location server over TCP (server_PFS)
* implement a peer to peer file distribution service over TCP (client_PFS)

The program is a peer-to-peer file distribution service. Each client will register its name and its files with the locatioin server. The location server districutes the complete list of filenames and file locations to each client. Clients then got directly to each peer to obtain the file they desire.

**Please see the notes section for details**

Author
------
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
* Register files with file location server
* List client's files `ls`
* <del>Request master file list from file location server</del>
* <del>Print master file list</del>
* <del>Get file from another client</del>
* <del>Provide file to another client</del>
* Exit current client `exit`

The strikes mean I uas unable to get this portion working after 30 hours of work and my limited capacity as a funciton human being.

### Other Useful Commands ###

1. `make clean` to create clean workspace.

Acknowledgments
---------------

This assignment is based on the my first assignment, "Programming Assignment 1 - UDP Socket Programming", University of Colorado Boulder.

Notes 
-----

After struggling with sockets and TCP for the majority of the assignment, I was able to get some functionality working. When you boot up the client, assuming the server is booted as well, the client automatically requests to be registered. It then successfully send it's list of files to the server. While still in the client, you can also type ls to get the current list of it's files and quit to exit. I also at one point was able to register multiple clients and reject already registered clients. But that made it impossible for the old clients to continue working.

Upon any other predefined commands, such as get, I receive infinite loop errors. I wasn't able to push out the master file list. Since this functionality didn't work, I was unable to move onto the sending of files between peers. After 30 hours of working on this one assignment, I decided it's time to call it quits. I could have given it more time if it weren't for the fact that I'm also a student in other courses, and I have a job so I can, you know, eat. I really wish we could have focused on the peer-to-peer file part instead of networking i/o. A baseline program would have been efficient since I'm a mere mortal and cannot possibly comprehend sockets, let alone tcp.

I feel my solution would be to use the select() function in order to monitor all the sockets. I believe since the accept() function is blocking and only moves to the next connection when a new client connects that all my woes would be forgotten using the select function. Again, since I'm a mere mortal, I was unable to get select to work. After a single timeout, it would enter into an infinite loop. A functioning example provided by this university would have been nice. 

  [w]: http://www.vargascorpus.com/
  [e]: mailto:patrick.vargas@colorado.edu
