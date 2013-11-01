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
