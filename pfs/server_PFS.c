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
#define MAXFILES   512
#define MAXCLIENTS 5

#define ERROR( boolean ) if ( boolean ) {\
    fprintf( stderr, "[%s:%i] %s\n", __FILE__, __LINE__-1, strerror( errno ) );\
    exit ( EXIT_FAILURE );\
  }

enum COMMAND { SUCCESS, FAILURE, PUT, GET, LS, EXIT, INVALID };

typedef struct {
  int total;
  struct {
	char name[MAXLINE];  // File owner
	unsigned long  addr; // Address of owner
	unsigned short port; // Port of owner 
	int entires;         // Number of Files
	struct {
	  char name[ MAXLINE ];  // The name of the file
	  int size;              // The size of the file (in B)
	} catalog[ MAXFILES ];
  } clients[ MAXCLIENTS ];
} Repository;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket - From ***REFERENCE***
int acceptConnection( int sock );

// Creates a socket for the port - From ***REFERENCE***
int createSocket( unsigned short port );

// Function to see if it's time to terminate program.
int isQuit ( char cmd[ ] );

// Function to get the list from a certain connection
void getList ( Repository *repo, int sock );

// Take the input and see what needs to be done.
int parseCmd ( char cmd[ ] );

// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote );

// Register the client
int registerClient( Repository *repo, int sock );
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
  char msg[ MAXBUFSIZE ];            // Message to return
  Repository repo; // The registration list

  //
  // Make sure port is given on command line
  // 
  if ( argc != 2 ) {
	printf( "USAGE:  <port>\n" );
	exit( EXIT_FAILURE );
  }

  repo.total = 0;
  
  //
  // Set up the socket
  //
  int sock = createSocket( atoi( argv[1] ) );
  int nuSock = acceptConnection( sock );
  if ( registerClient( &repo, nuSock ) == SUCCESS ) {
	sprintf( msg, "I got your message, %s!", repo.clients[repo.total-1].name );
	nbytes = send( nuSock, msg, MAXBUFSIZE, 0 );
	ERROR( nbytes < 0 );
  }

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
// Function to get the list from a certain connection
void getList ( Repository *repo, int sock ) {
  char buffer[ MAXBUFSIZE ]; // Buffer
  char msg[ MAXBUFSIZE ];    // Message to send
  int nbytes;                // Number of Bytes
  int n;                     // Number of files to add
  
  nbytes = send( sock, "GET", MAXBUFSIZE, 0 );
  ERROR( nbytes < 0 );
  
  bzero( buffer, sizeof( buffer ) );
  nbytes = recv( sock, buffer, MAXBUFSIZE, MSG_WAITALL );
  ERROR( nbytes < 0 );
  n = atoi( buffer );
  printf( "Will add %i files\n",  n );

} // getList( )
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

  if      ( strcmp( "success",  command ) == 0 ) return SUCCESS;
  else if ( strcmp( "failure",  command ) == 0 ) return FAILURE;
  else if ( strcmp( "put",      command ) == 0 ) return PUT;
  else if ( strcmp( "get",      command ) == 0 ) return GET;
  else if ( strcmp( "ls",       command ) == 0 ) return LS;
  else if ( strcmp( "exit",     command ) == 0 ) return EXIT;
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

////////////////////////////////////////////////////////////////////////////////
// Register the client
int registerClient( Repository *repo, int sock ) {
  int i;                      // Iterator
  int nbytes;                 // Number of Bytes
  char buffer[ MAXBUFSIZE ];  // Buffer
  struct sockaddr_in remote;  // Local address
  unsigned int remote_length; // Length of address
  remote_length = sizeof( remote );

  bzero( buffer, sizeof( buffer ) );
  nbytes = recv( sock, buffer, MAXBUFSIZE, MSG_WAITALL );
  ERROR( nbytes < 0 );
  printf( "Who is connecting? %s\n", buffer );

  for ( i = 0; i < repo->total; ++i ) {
	if ( strcmp( buffer, repo->clients[i].name ) == 0 ) {
	  printf( "'%s' already exists\n", buffer );
	  nbytes = send( sock, "FAILURE", MAXBUFSIZE, 0 );
	  ERROR( nbytes < 0 );	  
	  return FAILURE;
	}
  } 
 
  ERROR( getsockname( sock, (struct sockaddr *) &remote, &remote_length ) < 0 );
  strcpy( repo->clients[repo->total].name, buffer );
  repo->clients[repo->total].addr   = remote.sin_addr.s_addr;
  repo->clients[repo->total++].port = remote.sin_port;

  getList( repo, sock );

  return SUCCESS;
} // registerClient( )
////////////////////////////////////////////////////////////////////////////////
