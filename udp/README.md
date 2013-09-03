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

First create the executables `client` and `server` with the following command:  

	  make

Next, run the server program, replacing <port> with a number between 1024 and 65535:

	 ./server <port>

After that, run `hostname -i` to get the IP address of the server. Open up a new terminal. Use the found address and the port number you used in the following command in the new terminal:

	 ./client <server_ip> <server_port>

That's all for now!

### Other Useful Commands ###

1. `make clean` to start rm the executables and what have you.

  [w]: http://www.vargascorpus.com/
  [e]: mailto:patrick.vargas@colorado.edu