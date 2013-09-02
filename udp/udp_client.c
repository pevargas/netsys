////////////////////////////////////////////////////////////////////////////////
// File:    udp_client.c                Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements
//   1. The client must take two command line arguments: an IP address of the 
//      machine on which the server application is running, and the port the 
//      server application is using. [The IP address can be obtained using 
//      hostname -i . Type man hostname at the shell prompt for more information
//      on how to use the hostname command.]
//   2. It should prompt the user to type any of the following commands 
//     A. get [file_name] 
//     B. put [file_name] 
//     C. ls 
//     D. exit 
//     E. It must then send the command to the server.
//   3. Then it must wait for the server's response. Once the server responds, 
//      it should print appropriate messages, if any, on the standard output.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAXBUFSIZE 100
////////////////////////////////////////////////////////////////////////////////

/* You will have to modify the program below */

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  int nbytes;                             // number of bytes send by sendto()
  int sock;                               // this will be our socket
  char buffer[ MAXBUFSIZE ];
  
  struct sockaddr_in remote;              // "Internet socket address structure"
  
  if ( argc < 3 ) {
	printf("USAGE:  <server_ip> <server_port>\n");
	exit ( EXIT_FAILURE );
  }
  
  /******************
	Here we populate a sockaddr_in struct with
	information regarding where we'd like to send our packet 
	i.e the Server.
  ******************/
  bzero( &remote, sizeof( remote ) );              // zero the struct
  remote.sin_family = AF_INET;                     // address family
  remote.sin_port = htons( atoi( argv[ 2 ] ) );    // sets port to network byte order
  remote.sin_addr.s_addr = inet_addr( argv[ 1 ] ); // sets remote IP address
  
  // Causes the system to create a generic socket of type UDP (datagram)
  if ( ( sock = socket( PF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
	fprintf( stderr, "[%s:%i] Unable to create socket: %s\n", 
			 __FILE__, __LINE__ - 1, strerror( errno ) );
	exit ( EXIT_FAILURE );
  }
  
  /******************
	sendto() sends immediately.  
	it will report an error if the message fails to leave the computer
	however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
  ******************/
  char command[] = "apple";
  nbytes = sendto( sock, command, nbytes, 0, (struct sockaddr * )&remote, sizeof(remote));
  
  // Blocks till bytes are received
  struct sockaddr_in from_addr;
  int addr_length = sizeof( struct sockaddr );
  bzero( buffer, sizeof( buffer ) );
  nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr_in *) &from_addr, addr_length);  
  
  printf( "Server says %s\n", buffer );
  
  close( sock );
  
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

