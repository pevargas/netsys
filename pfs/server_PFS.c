////////////////////////////////////////////////////////////////////////////////
// File: server_PFS.c                   Fall 2013
// Student: Patrick Vargas              patrick.vargas@colorado.edu
// University of Colorado Boulder       CSCI 4273: Network Systems
// Requirements:
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

#define MAXBUFSIZE 100
#define MAXLINE    256
#define MAXPENDING 5

#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }

enum COMMAND { PUT, GET, LS, EXIT, INVALID };

typedef struct {
  char name[MAXLINE];      // The name of the file
  int size;                // The size of the file (in KB)
  char owner[MAXLINE];     // File owner
  struct in_addr sin_addr; // Address of owner
  unsigned short sin_port; // Port of owner
} FileMeta;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket - From ***REFERENCE***
int acceptConnection( int sock );

// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned short port );

// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Function to handle get command
void get ( char msg [], char cmd [ ], int sock, struct sockaddr_in remote );

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
  //  int sock, ibind;                   // This will be our socket and bind
  int nbytes;                        // number of bytes we receive in our message
  //struct sockaddr_in client, remote; // "Internet socket address structure"
  //unsigned int remote_length;        // length of the sockaddr_in structure
  char buffer[ MAXBUFSIZE ];         // a buffer to store our received message
  //char msg[ MAXBUFSIZE ];            // Message to return
  //char *newline = NULL;              // Get newline
  //FILE *fp;                          // Pointer to file

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
  int sock = createSocket( atoi( argv[1] ) );
  int nuSock = acceptConnection( sock );

  bzero( buffer, sizeof( buffer ) );
  nbytes = read( nuSock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );
  printf( "Client says: %s\n", buffer );
  
  nbytes = write( nuSock, "I got your message, yo!", MAXBUFSIZE );
  ERROR( nbytes < 0 );

  /*  
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
	case PUT: put ( msg, buffer, sock, remote ); break;
	case GET: get ( msg, buffer, sock, remote ); break;
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
	case EXIT: sprintf( msg, "Server will exit."); break;
	default: sprintf( msg, "Invalid command: %s", buffer); break;
	}
	printf( "%s\n", msg );
	
	// Send response
	nbytes = sendto( sock, msg, nbytes, 0, (struct sockaddr *)&remote, sizeof(remote));
	ERROR( nbytes < 0 );
	// printf( "Server sent %i bytes\n", nbytes);	
  } while ( !isQuit( buffer ) );  
  */  
  close( nuSock );
  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket - From ***REFERENCE***
int acceptConnection( int sock ) {
  int nuSock;                 // The new socket to be used
  struct sockaddr_in remote;  // The address of the remote connection
  unsigned int remote_length; // The length of the address

  // Set the size
  remote_length = sizeof( remote );
  // Wait for the client to connect
  nuSock = accept( sock, (struct sockaddr *) &remote, &remote_length );
  ERROR( nuSock < 0 );

  // We are connected to the client!
  printf( "Handling client %s\n", inet_ntoa( remote.sin_addr ) );
  
  return nuSock;
} // acceptConnection( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned short port ) {
  int sock;                 // The socket to create
  struct sockaddr_in local; // Local address

  // Create socket for incoming connections
  sock = socket( PF_INET, SOCK_STREAM, 0 );
  ERROR( sock < 0 );

  // Construct local address structure
  bzero( &local, sizeof( local ) );            // Zero out structure
  local.sin_family      = AF_INET;             // Internet address family
  local.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface
  local.sin_port        = htons( port );       // Local port

  // Bind to the local address
  ERROR( bind( sock, (struct sockaddr *) &local, sizeof( local ) ) < 0 );

  // Mark the socket so it will listen for incoming connections
  ERROR( listen( sock, MAXPENDING ) < 0 );

  printf( "Waiting for client\n" );

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
// Function to handle get command
void get ( char msg [ ], char cmd [ ], int sock, struct sockaddr_in remote ) { 
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
		strcpy( msg, buffer );
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
	  sprintf( buffer, "Finished getting file" );
	  strcpy( msg, buffer );
	  // Send buffer
	  nbytes = sendto( sock, buffer, MAXBUFSIZE, 0, (struct sockaddr *) &remote, sizeof(remote));
	  ERROR ( nbytes < 0 );
	  ERROR ( fclose( fp ) );
	}
  }  else sprintf( msg, "USAGE: get <file_name>");
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

  if      ( strcmp( "put",  command ) == 0 ) return PUT;
  else if ( strcmp( "get",  command ) == 0 ) return GET;
  else if ( strcmp( "ls",   command ) == 0 ) return LS;
  else if ( strcmp( "exit", command ) == 0 ) return EXIT;
  else return INVALID;
} // parseCmd( )
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
		if ( strcmp( buffer, "Finished putting file" ) == 0 ) eof = 1;
		else {
		  fputs( buffer, fp );
		  nbytes = recvfrom( sock, buffer, MAXBUFSIZE, 0, ( struct sockaddr * ) &remote, &remote_length );
		  ERROR( nbytes < 0 );
		}
	  }
	  ERROR( fclose( fp ) );
	}
  }
  else sprintf( msg, "USAGE: put <file_name>");
} // put()
////////////////////////////////////////////////////////////////////////////////
