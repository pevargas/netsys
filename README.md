Network Systems
===============

My work for CSCI 4273: Network Systems.

Directories
-----------

### udp/

**User Datagram Protocol Assignment:** The purpose of this program is to create a simple udp socket between a client and a server program. Please view the readme within the directory for more informaiton.

**Notes:** With three minutes to spare, I was able to turn it in. I was able to get the basic program working, such as the `ls` and `exit` command. The `get` and `put` commands do work. With a text file, there is no problem. With the three minutes to spare, I discovered I was using the wrong functions to implment binary file transfer. If I had more time, I believe I could have completed the assignemnt. Perhaps I will in the future, if time permits.

### gbn/

**Go-Back-N, Sliding Windows:** The purpose of this program is to transfer a file from the client to the server. The send is implmented through a window-based scheme. Packets with header information and data are sent to the server and an acknowledgment is sent back. This assignment was done in conjunction with [Brittany Harsha][e2].

**Notes:** We were able to get the window going. For some reason, our output file is off by one byte. In addition, our sliding window logic does not handle the packet losses. It does however discard ourt-of-range packets and buffers out-of-order packets. It also has the timeout set and works. Our logic for recieving duplicate acks is where we are failing.

### tcp/

**Link State Routing:** Purpose:

* learn how to establish and maintain TCP connections
* learn how distributed dynamic routing protocols accomplish packet routing
* implement a link state routing protocol

This assignment was done in conjunction with [Brittany Harsha][e2].

### p2p/

* Implement a file location server over TCP
* Implement a peer to peer file distribution service over TCP

The purpose of this assignment is to create a peer to peer file distribution service. Each client will register its name and its list files with the location server.  The location server distributes the complete list of filenames and file locations to each client.  Clients then go directly to peers to obtain the file they desire.

Contact
-------

**Patrick E. Vargas**

*  BS Computer Science, Software Engineering  
   *  Atmospheric and Oceanic Science Minor  
   *  Technology, Arts and Media Minor  
*  (720) 515 - 6402  
*  [www.vargascorpus.com][w]  
*  [patrick.vargas@colorado.edu][e]  

  [w]: http://www.vargascorpus.com/
  [e]: mailto:patrick.vargas@colorado.edu
  [e2]: mailto:b.grace.harsha@gmail.com
