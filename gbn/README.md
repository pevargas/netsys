Go-Back N, Sliding Windows and Window-Based Flow Control Assignment
===================================================================

The purpose of this program is to transfer a file from the client to the server. The send is implmented through a window-based scheme. Packets with header information and data are sent to the server and an acknowledgment is sent back. In general, the purpose of this assignment is to gain:  


1.  An understanding of reliable transmission transport layer protocols
2.  An understanding of cumulative acknowledgements
3.  An understanding of sliding windows
4.  An understanding of sequencing packets 
5.  An understanding of flow control

Group
-----
**Brittney Harsha**

[b.grace.harsha@gmail.com][e1]

**Patrick E. Vargas**

*  BS Computer Science, Software Engineering  
   *  Atmospheric and Oceanic Science Minor  
   *  Technology, Arts and Media Minor  
*  (720) 515 - 6402  
*  [www.vargascorpus.com][w]  
*  [patrick.vargas@colorado.edu][e2]  

Included Files
--------------

1. GBNclient.c
2. GBNserver.c
3. makefile
4. README.txt (this file)

README
------

* Author Information
* Description of application
* How to compile and execute application
* Which of the following functions work
	- reliably transfer a data file between a client and server despite possible packet losses (packet error rate will vary during our testing to evaluate the correctness of your protocol implementation)
	- sliding windows at both the client and server, with cumulative acknowledgements
	- timeouts with retransmission
	- discarding of out­of­range packets
	- buffering out­of­order packets
	- **[non­4273]** window­based flow control such that the receiver's advertised window size dynamically adjusts to the receiver's ability to process
* Technical details of the application

Commands
--------
### To Run the Program ###

1. Create the executables `GBNclient` and `GBNserver` with the following command:  

	  	make

2. Run the server program, replacing <server_port> with a number between 1024 and 65535, <error_rate> is between 0 and 1, <random_seed> is NULL for allways random or a constant to repeat the sequence, <output_fil> is the name of the recieved file and <recieve_log> is the name of the log file for the server:

	 	./GBNserver <server_port> <error_rate> <random_seed> <output_file> <receive_log>

3. Run `hostname -i` to get the IP address of the server. Open up a new terminal. Use the found address and the port number you used in the following command in the new terminal, <error_rate> is between 0 and 1, <random_seed> is NULL for allways random or a constant to repeat teh sequence, <send_file> is the name of the file to be sent and <send_log> is the name of the log file for the client.:

	 	./GBNclient <server_ip_address> <server_port> <error_rate> <random_seed> <send_file> <send_log>

### Other Useful Commands ###

1. `make clean` to create clean workspace.

Acknowledgments
---------------

This assignment is based on the files given by Professor Han, CSCI 4273: Network Systems, "Programming Assignment 2 - Go-Back N, Sliding Windows and Window-Based Flow Control", University of Colorado Boulder. Our implementation builds upon the tar file given to us as well as our preiveious assignment, "User Datagram Protocol Assignment".

Which of the following functions work
-------------------------------------

- [] Reliably transfer a data file between a client and server despite possible packet losses (packet error rate will vary during our testing to evaluate the correctness of your protocol implementation)
- [] Sliding windows at both the client and server, with cumulative acknowledgements
- [] Timeouts with retransmission
- [] Discarding of out­of­range packets
- [] Buffering out­of­order packets


  [w]: http://www.vargascorpus.com/
  [e2]: mailto:patrick.vargas@colorado.edu
  [e1]: mailto:b.grace.harsha@gmail.com
