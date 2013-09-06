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
#include <ctype.h>
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

/* You will have to modify the program below */

#define MAXBUFSIZE 100
#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  //
  // Variables
  // 
  int nbytes;                   // Number of bytes send by sendto()
  int sock;                     // This will be our socket
  char buffer[ MAXBUFSIZE ];    // Recieve data from Server
  char cmd[ MAXBUFSIZE ];       // Command to be sent to Server
  char *newline = NULL;         // Get newline
  struct sockaddr_in remote;    // "Internet socket address structure"
  struct sockaddr_in from_addr; // Socket for server
  unsigned int addr_length = sizeof( struct sockaddr );
  
  // Make sure ip and port are given
  if ( argc < 3 ) {
	printf("USAGE:  <server_ip> <server_port>\n");
	exit ( EXIT_FAILURE );
  }
  
  // 
  // Set up the socket 
  //
  /*******************
    Here we populate a sockaddr_in struct with information 
      regarding where we'd like to send our packet i.e the Server.
  ********************/
  // zero the struct
  bzero( &remote, sizeof( remote ) );             
  // address family
  remote.sin_family = AF_INET;                     
  // sets port to network byte order
  remote.sin_port = htons( atoi( argv[ 2 ] ) );  
  // sets remote IP address  
  remote.sin_addr.s_addr = inet_addr( argv[ 1 ] );
  
  ERROR( remote.sin_addr.s_addr < 0 );
  
  // Causes the system to create a generic socket of 
  //   type UDP (datagram)
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sock < 0 );  

  //
  // Enter command loop
  //
  do {
	bzero( cmd, sizeof( cmd ) );
	printf( "> " );
	if ( fgets( cmd, MAXBUFSIZE, stdin ) != NULL ) {
	  newline = strchr( cmd, '\n'); // Find the newline
	  if ( newline != NULL ) *newline = '\0'; // Overwrite
	  //scanf( "%s", cmd );
	  
	  /*******************
	    sendto() sends immediately.  
	      it will report an error if the message fails to leave the 
          computer.Hhowever, with UDP, there is no error if the message 
          is lost in the network once it leaves the computer.
	  ********************/  
	  nbytes = sendto( sock, cmd, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  //printf( "Client sent %i bytes to Server(%s:%d)\n", 
	  //		nbytes, inet_ntoa(remote.sin_addr), ntohs(remote.sin_port) );  
	  
	  // Blocks till bytes are received
	  bzero( buffer, sizeof( buffer ) );
	  nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &from_addr, &addr_length);  
	  ERROR ( nbytes < 0 );
	  
	  printf( "- %s\n", buffer );
	}
  } while ( !isQuit( cmd ) );  
  
  close( sock );
  
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] ) {
  if ( tolower( cmd[0] ) == 'e' &&
	   tolower( cmd[1] ) == 'x' &&
	   tolower( cmd[2] ) == 'i' &&
	   tolower( cmd[3] ) == 't'
	   ) {
	return 1;
  }

  return 0;
}
////////////////////////////////////////////////////////////////////////////////
