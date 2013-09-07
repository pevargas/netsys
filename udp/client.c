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

enum COMMAND { PUT, GET, LS, EXIT, INVALID };
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] );

// Function to handle put command
void put ( char cmd [ ], int sock, struct sockaddr_in remote );
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
  char temp[ MAXBUFSIZE ];      // Temporary string holder
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
	*cmd = '\0';
	//bzero( cmd, sizeof( cmd ) );
	printf( "> " );
	
	//
	// Grab Command Line
	//
	if ( fgets( cmd, MAXBUFSIZE, stdin ) != NULL ) {
	  newline = strchr( cmd, '\n'); // Find the newline
	  if ( newline != NULL ) *newline = '\0'; // Overwrite
	  
	  /*******************
	    sendto() sends immediately.  
	      it will report an error if the message fails to leave the 
          computer.Hhowever, with UDP, there is no error if the message 
          is lost in the network once it leaves the computer.
	  ********************/  
	  nbytes = sendto( sock, cmd, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  
	  //
	  // Put Command
	  //
	  switch ( parseCmd ( cmd ) ) {
	  case PUT:	put ( cmd, sock, remote ); break;
	  }
		
	  // Blocks till bytes are received
	  bzero( buffer, sizeof( buffer ) );
	  nbytes = recvfrom(sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &from_addr, &addr_length);  
	  ERROR ( nbytes < 0 );
	  
	  printf( "%s\n", buffer );
	}
	else if ( strcmp( "get", temp ) == 0 ) {

	}
  } while ( !isQuit( cmd ) );  
  
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
// Function to handle put command
void put ( char cmd [ ], int sock, struct sockaddr_in remote ) { 
  int nbytes;                   // Number of bytes send by sendto()
  char buffer[ MAXBUFSIZE ];    // Recieve data from Server
  char filename[ MAXBUFSIZE ];  // Name of file
  FILE *fp;                     // Pointer to file
  
  // Check for filename
  memcpy( filename, cmd + 4, strlen( cmd ) + 1 );
  // Make sure filename isn't null
  if ( strcmp( filename, "" ) != 0 ) {
	fp = fopen( filename, "r" );
	// See if file exists
	if ( fp == NULL ) {
	  // If file is MIA, print message to buffer
	  if ( errno == ENOENT ) {
		sprintf( buffer, "File does not exist" );
		// Send buffer
		nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
		ERROR ( nbytes < 0 );
	  }
	  else ERROR ( fp == NULL );
	} // fp == NULL
	else {
	  *buffer = '\0';
	  // Else read contents of file into buffer
	  while ( fgets( buffer, MAXBUFSIZE, fp ) != NULL ) {
		printf( "%s", buffer );
		// Send one line from file
		nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
		ERROR ( nbytes < 0 );
	  }
	  // Tell server we're done
	  sprintf( buffer, "Finished sending file" );
	  // Send buffer
	  nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  ERROR ( fclose( fp ) );
	}
  } // filename != ""
} // void put ()
////////////////////////////////////////////////////////////////////////////////

