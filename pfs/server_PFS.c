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

#include "athena.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket
int acceptConnection( int sock );

// Creates a socket for the port
int createSocket( unsigned short port );

// Function to get the list from a certain connection
void getList ( Repository *r, int sock );

// Recieve put data from the client
void put ( char msg [], char buffer [], int sock, struct sockaddr_in remote );

// Register the client
int registerClient( Repository *r, int sock );

// Remove client from list
int removeClient( Repository *r, int sock ); 

// Send the master list
void sendMaster( Repository *r, int sock );

// Who is it?
int whoisthis( Repository *r, int sock );
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main ( int argc, char * argv[ ] ) {
  int sock, nuSock;          // This will be our sockets
  int nbytes;                // Number of bytes we receive in our message
  int code;                  // Error code for Select
  char buffer[ MAXBUFSIZE ]; // A buffer to store our received message
  char msg[ MAXBUFSIZE ];    // A Message to return
  Repository repo;           // The registration list
  fd_set fds;
  struct timeval timeout;
  int count = 0;

  //
  // Make sure port is given on command line
  // 
  if ( argc != 2 ) {
	printf( "USAGE:  <port>\n" );
	exit( EXIT_FAILURE );
  }

  //
  // Initialize all the things!
  //
  repo.total = 0;
  maxPrint( &repo );
  sock = createSocket( atoi( argv[1] ) );

  timeout.tv_sec = 2;
  timeout.tv_usec = 0;

  FD_ZERO( &fds );
  FD_SET( sock, &fds );
  nuSock = acceptConnection( sock );
  ERROR( nuSock < 0 );
  FD_SET( nuSock, &fds );
  
  //
  // Enter command loop
  //
  for ( ; ; ) {
	/*	if ( ( code = select( sizeof( fds )*8, &fds, NULL, NULL, &timeout ) ) < 0 ) {
	  ERROR( code < 0 );
	}
	else if ( code == 0 ) {
	  printf( "$ Timeout [%i]\r", count = ++count % 100 );
	  ERROR( count );
	}
	else {
	  count = 0;
	  if ( FD_ISSET( sock, &fds ) ) {
		// Loop through all the conenctions Available
		while ( ( nuSock = acceptConnection( sock ) ) > 0 );
		FD_SET( nuSock, &fds );
	  }
	*/	  
	ERROR( nuSock < 0 );
	bzero( buffer, MAXBUFSIZE );
	nbytes = read( nuSock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
	printf( "[%s] %s\n", repo.cxn[whoisthis( &repo, nuSock )].name, buffer );
	
	switch ( parseCmd ( buffer ) ) 
	  {
	  case CONNECT:
		{
		  nbytes = write( nuSock, buffer, MAXBUFSIZE );
		  ERROR( nbytes < 0 );
		  switch( registerClient( &repo, nuSock ) ) 
			{
			case SUCCESS:
			  getList ( &repo, nuSock );
			  printf( "> Success in registering '%s'\n", repo.cxn[repo.total-1].name );
			  break;
			case FAILURE:
			  printf( "> Error in registration\n" );
			  break;
			}
		  break;
		}
	  case GET:
		sendMaster( &repo, nuSock );
		break;
	  case EXIT:
		{
		  switch( removeClient( &repo, nuSock ) ) {
		  case SUCCESS: 
			printf( "> Client has been removed.\n" );
			break;
		  case FAILURE:
			printf( "> Error removing client.\n" );
			break;
		  }
		  break;
		}
	  default: 
		sprintf( msg, "Invalid command: '%s'", buffer); 
		nbytes = write( nuSock, msg, MAXBUFSIZE );
		ERROR( nbytes < 0 );
		break;
	  }
  }
  
  close( nuSock );
  close( sock );
  
  return EXIT_SUCCESS;
} // main( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Accepts a TCP Conection along the socket
int acceptConnection( int sock ) {
  int nuSock;                 // The new socket to be used
  struct sockaddr_in remote;  // The address of the remote connection
  unsigned int remote_length; // The length of the address

  // Set the size
  remote_length = sizeof( remote );
  // Wait for the client to connect
  nuSock = accept( sock, (struct sockaddr *) &remote, &remote_length );
  if ( nuSock < 0 ) {
	ERROR( (errno != ECHILD) && (errno != ERESTART) && (errno != EINTR) );
  }
  // We are connected to a NEW client!
  printf( "> New Challenger Approaching! [%s:%i]\n", 
		  inet_ntoa( remote.sin_addr ), ntohs( remote.sin_port ) );
  
  return nuSock;
} // acceptConnection( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Creates a socket for the port
int createSocket( unsigned short port ) {
  int sock;                 // The socket to create
  int sockoptval = 1;       // Socket Option Value
  struct sockaddr_in local; // Local address

  // Create socket for incoming connections
  sock = socket( AF_INET, SOCK_STREAM, 0 );
  ERROR( sock < 0 );

  // Allow immediate Reuse of port
  //  setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof( int ) );

  // Construct local address structure
  bzero( &local, sizeof( local ) );            // Zero out structure
  local.sin_family      = AF_INET;             // Internet address family
  local.sin_addr.s_addr = htonl( INADDR_ANY ); // Any incoming interface
  local.sin_port        = htons( port );       // Local port

  // Bind to the local address
  ERROR( bind( sock, (struct sockaddr *) &local, sizeof( local ) ) < 0 );

  // Mark the socket so it will listen for incoming connections
  ERROR( listen( sock, MAXPENDING ) < 0 );

  printf( "$ Waiting for clients\n" );

  return sock; 
} // createSocket( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function to get the list from a certain connection
void getList ( Repository *r, int sock ) {
  int i;                     // Iterator
  int nbytes;                // Number of Bytes
  int n;                     // Number of files to add
  char buffer[ MAXBUFSIZE ]; // Buffer
  
  do {
	bzero( buffer, MAXBUFSIZE );
	nbytes = read( sock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
  }	while( !strcmp( buffer, "" ) );

  n = atoi( buffer );
  r->cxn[r->total - 1].entries = n;

  for ( i = 0; i < n; ++i ){
	nbytes = read( sock, buffer, MAXBUFSIZE );
	ERROR( nbytes < 0 );
	memcpy( &r->cxn[r->total -1].list[i], &buffer, sizeof( FM ) );
  }

  printRepo( r );
} // getList( )
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
int registerClient( Repository *r, int sock ) {
  int i;                      // Iterator
  int nbytes;                 // Number of Bytes
  char buffer[ MAXBUFSIZE ];  // Buffer
  struct sockaddr_in remote;  // Local address
  unsigned int remote_length; // Length of address
  remote_length = sizeof( remote );

  bzero( buffer, MAXBUFSIZE );
  nbytes = read( sock, buffer, MAXBUFSIZE );
  ERROR( nbytes < 0 );
  printf( "$ Checking if '%s' is already registered\n", buffer );

  for ( i = 0; i < r->total; ++i ) {
	if ( strcmp( buffer, r->cxn[i].name ) == 0 ) {
	  printf( "'%s' already exists\n", buffer );
	  nbytes = write( sock, "FAILURE", MAXBUFSIZE );
	  ERROR( nbytes < 0 );	  
	  return FAILURE;
	}
  } 
 
  ERROR( getsockname( sock, (struct sockaddr *) &remote, &remote_length ) < 0 );
  strcpy( r->cxn[r->total].name, buffer );
  r->cxn[r->total].entries         = 0;
  r->cxn[r->total].sin_addr.s_addr = remote.sin_addr.s_addr;
  r->cxn[r->total++].sin_port      = remote.sin_port;

  nbytes = write( sock, "SUCCESS", MAXBUFSIZE );
  ERROR( nbytes < 0 );	    

  return SUCCESS;
} // registerClient( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Remove client from list
int removeClient( Repository *r, int sock ) {
  int i;                      // Iterator
  i = whoisthis( r, sock );
  
  if ( i != ( r->total - 1 ) ) {
	for ( ; i < r->total - 1; ++i )
	  memcpy( &r->cxn[i], &r->cxn[i+1], sizeof( r->cxn[i] ) );
	
	r->total--;
  }
  else {
	return FAILURE;
  }
  
  return SUCCESS;
} // removeClient( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Send the master list
void sendMaster( Repository *r, int sock ) {
  int nbytes;                // Number of Bytes
  char buffer[ MAXBUFSIZE ]; // Buffer

  printf( "> Request Master List\n" );

  bzero( buffer, MAXBUFSIZE );
  memcpy( &buffer, r, sizeof( Repository ) );
  nbytes = write( sock, r, MAXBUFSIZE );
  ERROR( nbytes < 0 );
} // sendMaster( )
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Who is it?
int whoisthis( Repository *r, int sock ) {
  int i;                      // Iterator
  struct sockaddr_in remote;  // Local address
  unsigned int remote_length; // Length of address
  remote_length = sizeof( remote );
  
  ERROR( getsockname( sock, (struct sockaddr *) &remote, &remote_length ) < 0 );

  if ( r->total > 0 ) {
	for ( i = 0; i < r->total; ++i )
	  if ( ( r->cxn[i].sin_addr.s_addr == remote.sin_addr.s_addr ) &&
		   ( r->cxn[i].sin_port        == remote.sin_port ) )
		break;
  }
  else {
	return -1;
  }

  return i;
} // whoisthis( )
////////////////////////////////////////////////////////////////////////////////
