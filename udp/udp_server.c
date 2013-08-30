////////////////////////////////////////////////////////////////////////////////
// File: udp_server.c                   Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
//   1. The server must take one command line argument: a port number for the 
//      server to use. You should select port #'s > 5000.
//   2. It should wait for a UDP connection.
//   3. Depending on the commands received, the server responds to the client's 
//      request in the following manner: 
//     A. get [file_name]: 
//          The server transmits the requested file to the client (use files of
//          small size in order of 2 to 5 KB for transfer like any jpeg file).
//     B. put [file_name]: 
//          The server receives the transmitted file by the client and stores it
//          locally (use files of small size in order of 2 to 5 KB for transfer
//          like any jpeg file).
//     C. ls: 
//          The server should search all the files it has in its local directory
//          and send a list of all these files to the client
//     D. exit: 
//          The server should exit gracefully. 
//     E. For any other commands, the server should simply repeat the command
//        back to the client with no modification, stating that the given
//        command was not understood.
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
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* You will have to modify the program below */

#define MAXBUFSIZE 100
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  int sock, ibind;                    // This will be our socket and bind
  struct sockaddr_in sin, remote;     // "Internet socket address structure"
  unsigned int remote_length;         // length of the sockaddr_in structure
  int nbytes;                         // number of bytes we receive in our message
  char buffer[ MAXBUFSIZE ];          // a buffer to store our received message
  if ( argc != 2 ) {
	printf( "USAGE:  <port>\n" );
	exit( EXIT_FAILURE );
  }
  
  /******************
	This code populates the sockaddr_in struct with
	the information about our socket
  ******************/
  bzero( &sin, sizeof( sin ) );               // zero the struct
  sin.sin_family = AF_INET;                   // address family
  sin.sin_port = htons( atoi( argv[ 1 ] ) );  // htons() sets the port # to network byte order
  sin.sin_addr.s_addr = INADDR_ANY;           // supplies the IP address of the local machine
  
  // Causes the system to create a generic socket of type UDP (datagram)
  if ( ( sock = socket( AF_LOCAL, SOCK_DGRAM, 0 ) ) < 0 ) {
	fprintf( stderr, "[%i] Unable to create socket: %s\n", __LINE__ - 1, strerror( errno ) );
	exit ( EXIT_FAILURE );
  }
  
  
  /******************
    Once we've created a socket, we must bind that socket to the 
	local address and port we've supplied in the sockaddr_in struct
  ******************/
  if ( ( ibind = bind( sock, ( struct sockaddr * ) &sin, sizeof( sin ) ) ) < 0 ) {
	fprintf( stderr, "[%i] Unable to bind socket: %s\n", __LINE__ - 1, strerror( errno ) );
	exit ( EXIT_FAILURE );
  }
  
  remote_length = sizeof( remote );
  
  // waits for an incoming message
  bzero( buffer, sizeof( buffer ) );
  nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, NULL, 0);
  
  printf( "The client says %s\n", buffer );
  
  char msg[] = "orange";
  nbytes = sendto( sock, msg, nbytes, 0, NULL, 0);
  
  close( sock );
  
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

