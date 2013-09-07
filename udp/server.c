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
#include <ctype.h>
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

#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }

enum COMMAND { PUT, GET, LS, EXIT, INVALID };
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] );

// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  //
  // Variables
  //
  int sock, ibind;                   // This will be our socket and bind
  int nbytes;                        // number of bytes we receive in our message
  struct sockaddr_in client, remote; // "Internet socket address structure"
  unsigned int remote_length;        // length of the sockaddr_in structure
  char buffer[ MAXBUFSIZE ];         // a buffer to store our received message
  char msg[ MAXBUFSIZE ];            // Message to return
  char *newline = NULL;              // Get newline
  FILE *fp;                          // Pointer to file

  //
  // Make sure port is given on command line
  // 
  if ( argc != 2 ) {
	printf( "USAGE:  <port>\n" );
	exit( EXIT_FAILURE );
  }
  
  //
  // Set up the socket
  //
  /********************
    This code populates the sockaddr_in struct with
      the information about our socket
  *********************/
  // zero the struct
  bzero( &client, sizeof( client ) );            
  // address family
  client.sin_family = AF_INET;                   
  // htons() sets the port # to network byte order
  client.sin_port = htons( atoi( argv[ 1 ] ) );  
  // supplies the IP address of the local machine
  client.sin_addr.s_addr = INADDR_ANY;           
  
  // Causes the system to create a generic socket of 
  //   type UDP (datagram)
  sock = socket( AF_INET, SOCK_DGRAM, 0 );
  ERROR( sock < 0 );

  /********************
    Once we've created a socket, we must bind that socket to the 
	local address and port we've supplied in the sockaddr_in struct
  *********************/
  ibind = bind( sock, ( struct sockaddr * ) &client, sizeof( client ) );
  ERROR( ibind < 0 );
  remote_length = sizeof( remote );
  
  printf( "Waiting for client\n" );
  
  //
  // Enter command loop
  //
  do {
	// Wait for incoming message
	bzero( buffer, sizeof( buffer ) );
	nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
	ERROR( nbytes < 0 );
	printf( "Client(%s:%d): %s\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), buffer );
	
	switch ( parseCmd ( buffer ) ) {
	case PUT:
	  put( msg, buffer, sock, remote );
	  break;
	case GET:
	  sprintf( msg, "Server will get file.");	  
	  break;
	case LS:
	  *buffer = '\0';
      *msg = '\0';
	  fp = popen( "ls", "r" );
	  ERROR( fp == NULL );
	  while ( fgets( buffer, MAXBUFSIZE, fp ) != NULL ) {
		newline = strchr( buffer, '\n'); // Find the newline
		if ( newline != NULL ) *newline = ' '; // Overwrite
	    strcat( msg, buffer );
	  }
	  ERROR( pclose( fp ) < 0 );
	  break;
	case EXIT:
	  sprintf( msg, "Server will exit.");
	  break;
	default:
	  sprintf( msg, "Invalid command: %s", buffer);
	  break;
	}
	printf( "%s\n", msg );
	
	// Send response
	nbytes = sendto( sock, msg, nbytes, 0, (struct sockaddr *)&remote, sizeof(remote));
	ERROR( nbytes < 0 );
	// printf( "Server sent %i bytes\n", nbytes);	
  } while ( !isQuit( buffer ) );  
  
  close( sock );
  
  return EXIT_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] ) {
  // Convert string to lowercase
  int i;
  for (i = 0; cmd[i]; ++i ) cmd[i] = tolower( cmd[i] );

  if ( strcmp( "exit", cmd ) == 0 ) return 1;
  return 0;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] ) {
  int i;
  char command[ MAXBUFSIZE ];

  // Convert string to lowercase
  for (i = 0; cmd[i]; ++i ) cmd[i] = tolower( cmd[i] );

  // Get the command
  sscanf( cmd, "%s", command );

  if      ( strcmp( "put", command ) == 0 )  return PUT;
  else if ( strcmp( "get", command ) == 0 )  return GET;
  else if ( strcmp( "ls", command ) == 0 )   return LS;
  else if ( strcmp( "exit", command ) == 0 ) return EXIT;
  else return INVALID;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote ) { 
  int nbytes;                        // number of bytes we receive in our message
  int eof;                           // Flag for end of file stream
  unsigned int remote_length;        // length of the sockaddr_in structure
  char filename[ MAXBUFSIZE ];       // Name of file
  FILE *fp;                          // Pointer to file

  remote_length = sizeof( remote );

  // Check for filename
  memcpy( filename, buffer + 4, strlen( buffer ) + 1 );
  if ( strcmp( filename, "" ) != 0 ) {
	strcat( filename, "_server" );
	sprintf( msg, "Filename: %s", filename );
	
	// Wait for incoming message
	bzero( buffer, sizeof( buffer ) );
	nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
	ERROR( nbytes < 0 );
	
	if ( strcmp( buffer, "File does not exist" ) == 0 ) sprintf( msg, "%s", buffer );
	else {
	  fp = fopen( filename, "w" );
	  ERROR( fp == NULL );
	  eof = 0;
	  while ( !eof ) {
		nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
		ERROR( nbytes < 0 );
		
		if ( strcmp( buffer, "Finished putting file" ) == 0 ) eof = 1;
		else fputs( buffer, fp );
	  }
	  ERROR( fclose( fp ) );
	}
  }
  else sprintf( msg, "USAGE: put <file_name>");
} // put()
////////////////////////////////////////////////////////////////////////////////
