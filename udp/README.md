User Datagram Protocol Assignment
=================================

The purpose of this program is to create a simple udp socket between a client and a server program.

**Patrick E. Vargas**

*  BS Computer Science, Software Engineering  
   *  Atmospheric and Oceanic Science Minor  
   *  Technology, Arts and Media Minor  
*  (720) 515 - 6402  
*  [www.vargascorpus.com][w]  
*  [patrick.vargas@colorado.edu][e]  

Included Files
--------------

1. client.c
2. server.c
3. makefile
4. README.txt (this file)

Commands
--------
### To Run the Program ###

1. Create the executables `client` and `server` with the following command:  

	  	make

2. Run the server program, replacing <port> with a number between 1024 and 65535:

	 	./server <port>

3. Run `hostname -i` to get the IP address of the server. Open up a new terminal. Use the found address and the port number you used in the following command in the new terminal:

	 	./client <server_ip> <server_port>

### While running the client ####

1. `put <file_name>` to put file on server.

2. `get <file_name>` to get file from server.

3. `ls` to list all the files on the server.

4. `exit` to destroy connection safely.

### Other Useful Commands ###

1. `make clean` to create clean workspace.

  [w]: http://www.vargascorpus.com/
  [e]: mailto:patrick.vargas@colorado.edu
