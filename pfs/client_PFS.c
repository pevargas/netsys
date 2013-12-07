////////////////////////////////////////////////////////////////////////////////
// File:    client_PFS.c                Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements
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

#define MAXBUFSIZE 100

#define ERROR( boolean ) if ( boolean ) {								\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );	\
    exit ( EXIT_FAILURE );												\
  }

enum COMMAND { PUT, GET, LS, EXIT, INVALID };
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the ip address and port - From ***REFERENCE***
int createSocket( unsigned long ip, unsigned short port );

// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Recieve get data from the server
void get ( char buffer [], int sock, struct sockaddr_in remote );

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
  int nbytes;                   // Number of bytes
  int sock;                     // This will be our socket
  char buffer[ MAXBUFSIZE ];    // Recieve data from Server
  //  char cmd[ MAXBUFSIZE ];       // Command to be sent to Server
  //char temp[ MAXBUFSIZE ];      // Temporary string holder
  //char *newline = NULL;         // Get newline
  //struct sockaddr_in remote;    // "Internet socket address structure"
  //struct sockaddr_in from_addr; // Socket for server
  //unsigned int addr_length = sizeof( struct sockaddr );
  
  // Make sure ip and port are given
  if ( argc < 3 ) {
	printf("USAGE:  <server_ip> <server_port>\n");
	exit ( EXIT_FAILURE );
  }
  
  // 
  // Set up the socket 
  //
  sock = createSocket( inet_addr( argv[1] ), atoi( argv[2] ) );
  nbytes = write( sock, "Do you hear me, yo?", MAXBUFSIZE );
  ERROR( nbytes < 0 );

  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  printf("%s\n", buffer);

  /*
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
	  
	  //
	  // sendto() sends immediately.  
	  //    it will report an error if the message fails to leave the 
      //    computer.Hhowever, with UDP, there is no error if the message 
      //    is lost in the network once it leaves the computer.
	  nbytes = sendto( sock, cmd, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  
	  //
	  // Put Command
	  //
	  switch ( parseCmd ( cmd ) ) {
	  case PUT:	put ( cmd, sock, remote ); break;
	  case GET: get ( cmd, sock, remote ); break;
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
  */
  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned long ip, unsigned short port ) {
  int sock;                 // The socket to create
  struct sockaddr_in remote; // Local address

  // Create socket for incoming connections
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  ERROR( sock < 0 );

  // Construct local address structure
  bzero( &remote, sizeof( remote ) );     // Zero out structure
  remote.sin_family      = PF_INET;       // Internet address family
  remote.sin_addr.s_addr = ip;            // The address for the server
  remote.sin_port        = htons( port ); // Local port

  ERROR( connect( sock, (struct sockaddr *) &remote, sizeof( remote ) ) < 0 );
  printf( "Connected to remote\n" );

  return sock; 
} // createSocket( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] ) {
  // Convert string to lowercase
  int i;
  for (i = 0; cmd[i]; ++i ) cmd[i] = tolower( cmd[i] );

  if ( strcmp( "exit", cmd ) == 0 ) return 1;
  return 0;
} // isQuit( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Recieve get data from the server
void get ( char buffer [], int sock, struct sockaddr_in remote ) { 
  int nbytes;                        // number of bytes we receive in our message
  int eof;                           // Flag for end of file stream
  unsigned int remote_length;        // length of the sockaddr_in structure
  char filename[ MAXBUFSIZE ];       // Name of file
  FILE *fp;                          // Pointer to file

  remote_length = sizeof( remote );

  // Check for filename
  memcpy( filename, buffer + 4, strlen( buffer ) + 1 );
  if ( strcmp( filename, "" ) != 0 ) {
	strcat( filename, "_client" );
	
	// Wait for incoming message
	bzero( buffer, sizeof( buffer ) );
	nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
	ERROR( nbytes < 0 );
	
	if ( strcmp( buffer, "File does not exist" ) != 0 ) {
	  fp = fopen( filename, "w" );
	  ERROR( fp == NULL );
	  eof = 0;
	  while ( !eof ) {
		if ( strcmp( buffer, "Finished getting file" ) == 0 ) eof = 1;
		else {
		  fputs( buffer, fp ); 
		  nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
		  ERROR( nbytes < 0 );
		}
	  }
	  ERROR( fclose( fp ) );
	}
  }
} // get( )
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
	  sprintf( buffer, "Finished putting file" );
	  // Send buffer
	  nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  ERROR ( fclose( fp ) );
	}
  } // filename != ""
} // put( )
////////////////////////////////////////////////////////////////////////////////

